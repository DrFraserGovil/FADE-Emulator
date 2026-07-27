#pragma once

#include <Eigen/Dense>
#include <FADE/Distributions/Distribution.h>
#include <FADE/Distributions/Kernels.h>
#include <FADE/ModelSettings.h>
#include <FADE/Parameters/ParameterVector.h>
#include <FADE/Train/TrainingPoint.h>
#include <JSL.h>
namespace FADE
{

	template <class T = double>
	class Submodel
	{
	  public:
		Submodel(ModelSettings &parentSettings, sint depCount, sint expertCount) : Parameters(parentSettings.Hyper, depCount, expertCount), Settings(parentSettings)
		{
			Ne = expertCount;
			Nd = depCount;
			LOG(DEBUG) << "  - Constructing submodel " << Nd << "-" << Ne;
			SetSizes();
		}
		void SyncParameters()
		{
			Parameters.UpdateDerived();
			for (sint i = 0; i < Ne; ++i)
			{
				ComputeTkFromVec([&](sint j) -> T & { return Parameters.ExpertPosition(i, j); }, TkExpert[i]);
			}
		}
		void CopyPosition(Submodel<T> &model)
		{
			Parameters.Params = model.Parameters.Params;
			BestScore = model.BestScore;
			SyncParameters();
		}

		T LogGaussian(double y)
		{
			T v = ExpertWeights[0] + LogGaussianDistribution(y, 0, Parameters);
			for (sint e = 1; e < Ne; ++e)
			{
				v = ale(v, ExpertWeights[e] + LogGaussianDistribution(y, e, Parameters));
			}
			return v;
		}
		void AccumulateLogGaussian(double y, T &accumulator, double logweight)
		{
			for (sint e = 0; e < Ne; ++e)
			{
				accumulator = ale(accumulator, logweight + ExpertWeights[e] + LogGaussianDistribution(y, e, Parameters));
			}
		}

		void ResetCache()
		{
			for (sint k = 0; k < Nd; ++k)
			{
				TkPos[k] = 0;
				for (sint e = 0; e < Ne; ++e)
				{
					TkExpert[e][k] = 0;
				}
			}
		}

		void SetPosition(const std::vector<double> &x)
		{

			ResetCache();
			ComputeTkFromVec([&](sint i) -> double { return x[i]; }, TkPos);
			for (sint k = 0; k < Nd; ++k)
			{
				T wsum = 0;
				for (sint i = 0; i < Ne; ++i)
				{
					T dk = ComputeDistance([&x](sint idx) -> double { return x[idx]; }, [&](sint idx) -> T & { return Parameters.ExpertPosition(i, idx); }, k);
					// LOG(INFO) << "Distance " << x << " " << Parameters.ExpertPosition(i, 0) << " = " << dk;
					TkExpert[i][k] += LogKernel(dk, 1);
					if (i == 0) { wsum = TkExpert[i][k]; }
					else
					{
						wsum = ale(wsum, TkExpert[i][k]);
					}
				}
				for (sint i = 0; i < Ne; ++i)
				{
					if (k == 0)
					{

						ExpertWeights[i] = TkExpert[i][k] - wsum + TkPos[k];
					}
					else
					{
						ExpertWeights[i] = ale(ExpertWeights[i], (TkExpert[i][k] - wsum + TkPos[k]));
					}
					TkExpert[i][k] -= wsum;
				}
			}
		}

		void Save(JSL::IO::VaultWriter &vault)
		{
			std::string root = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne);
			vault[root + "/position.dat"] << Parameters.ToString();
			vault[root + "/score.dat"] << BestScore;

			Eigen::Index rows = CholeskyMat.rows();
			Eigen::Index cols = CholeskyMat.cols();
			std::ostringstream os;
			os << rows << " " << cols << "\n";
			os << std::setprecision(17);
			for (Eigen::Index r = 0; r < CholeskyMat.rows(); ++r)
			{
				for (Eigen::Index c = 0; c < CholeskyMat.cols(); ++c)
				{
					os << CholeskyMat(r, c) << (c + 1 == CholeskyMat.cols() ? "" : " ");
				}
				os << "\n";
			}
			vault[root + "/cholesky.dat"] << os.str();
		}

		void Load(JSL::IO::VaultReader &vault)
		{
			Parameters.UpdateDerived();
			std::string root = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne);
			std::string param = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne) + "/position.dat";
			if (!vault.Files().contains(param))
			{
				LOG(ERROR) << "Provided input is missing a submodel entry for " << param << ", despite the metadata indicating it exists.\nThe model is most likely corrupted";
				exit(1);
			}
			LOG(DEBUG) << "  - Loading submodel " << root << " from file";
			auto lines = vault[param].AsLines();
			if (lines.size() != Parameters.Size())
			{
				LOG(ERROR) << "Dimensional mismatch between model on disk and submodel (" << Nd << ", " << Ne << ")\n"
						   << "Either the model is corrupted, or it is not compatible with this version of FADE.";
				exit(1);
			}
			else
			{
				Parameters.Load(lines);
			}
			SyncParameters();
			std::ostringstream os;
			os << "Computing the Hessian for the model with the following properties:\n";
			for (sint k = 0; k < Nd; ++k)
			{
				os << "\tDep. " << k + 1 << " position: (";
				for (sint q = 0; q < Settings.Hyper.InputDimension; ++q)
				{
					os << Parameters.DepPosition(k, q);
				}
				os << ")\n";
			}
			for (sint k = 0; k < Nd; ++k)
			{
				os << JSL::Display::Yellow();
				os << "\tDep. " << k + 1 << " metric: (";
				for (sint q = 0; q < Settings.Hyper.MatrixSize; ++q)
				{
					os << Parameters.Phi(k, q);
				}
				os << ")\n";
			}
			for (sint e = 0; e < Ne; ++e)
			{
				os << JSL::Display::Green();
				os << "\tExp. " << e + 1 << " position: (";
				for (sint q = 0; q < Settings.Hyper.InputDimension; ++q)
				{
					os << Parameters.ExpertPosition(e, q);
				}
				os << ")\n";
			}
			for (sint e = 0; e < Ne; ++e)
			{
				os << JSL::Display::Purple();
				os << "\tExp. " << e + 1 << " parameters: (";
				for (sint q = 0; q < Settings.Hyper.ProbabilityDimension; ++q)
				{
					if (q > 0) os << ", ";
					os << Parameters.ExpertParameter(e, q);
				}
				os << ")\n";
			}
			LOG(INFO) << os.str();
		}

		ParameterVector<T> Parameters;

		std::pair<T, T> EstimateMoments()
		{
			// if (Settings.Hyper.Family == "gaussian")
			// {
			T muSum = 0;
			T vSum = 0;
			for (sint e = 0; e < Nd; ++e)
			{
				T &mu = Parameters.ExpertParameter(e, 0);
				T &sigma = Parameters.ExpertParameter(e, 1);

				muSum += ExpertWeights[e] * mu;
				vSum += ExpertWeights[e] * (sigma * sigma + mu * mu);
			}
			return {muSum, sqrt(vSum - muSum * muSum)};
			// }
		}

		void Train([[maybe_unused]] std::vector<ClusteredTrains> train, [[maybe_unused]] std::vector<ClusteredTrains> validate, [[maybe_unused]] std::optional<JSL::Display::Progress::Bar *> PB = std::nullopt)
		{
			LOG(ERROR) << "Training routine must occur on a specialised branch";
			exit(1);
		}

		T Score(std::vector<ClusteredTrains> data, bool hardPrior = true)
		{
			SyncParameters();
			T prior = Prior(hardPrior);
			if (prior == Settings.Train.LogZero)
			{
				return Settings.Train.LogZero;
			}
			// return prior;
			T score = 0;
			for (auto &cluster : data)
			{
				sint N = cluster.Values.size();
				SetPosition(cluster.Position);
				for (sint j = 0; j < N; ++j)
				{
					T s = -9999999999999;
					AccumulateLogGaussian(cluster.Values[j], s, cluster.LogWeights[j]);
					score += s;
				}
			}
			return score + prior;
		}

		T CutPrior()
		{
			for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
			{
				for (sint k = 0; k < Nd; ++k)
				{
					auto cj = (Parameters.DepPosition(k, j));
					if (cj > Settings.Prior.PriorTopRight[j] || cj < Settings.Prior.PriorBottomLeft[j])
					{
						return Settings.Train.LogZero;
					}
				}
				for (sint e = 0; e < Ne; ++e)
				{
					auto sj = (Parameters.ExpertPosition(e, j));
					if (sj > Settings.Prior.PriorTopRight[j] || sj < Settings.Prior.PriorBottomLeft[j])
					{
						return Settings.Train.LogZero;
					}
				}
			}

			for (sint e = 0; e < Ne; ++e)
			{
				if (Parameters.ExpertParameter(e, 1) < 0.01)
				{
					return Settings.Train.LogZero;
				}
				if (Parameters.ExpertParameter(e, 0) > 3 || Parameters.ExpertParameter(e, 0) < -1)
				{
					return Settings.Train.LogZero;
				}
				if (Parameters.ExpertParameter(e, 2) < 0.01)
				{
					return Settings.Train.LogZero;
				}
			}
			return 0;
		}

		T Prior(bool hardPrior = true)
		{
			// if (hardPrior) { return CutPrior(); }
			// T prior = 0;
			T prior = hardPrior ? CutPrior() : 0;
			// return prior;
			double spring = 1e-3;
			if (prior == Settings.Train.LogZero) { return prior; }
			for (sint k = 0; k < Nd; ++k)
			{
				for (sint kk = k + 1; kk < Nd; ++kk)
				{
					T dist = 0;
					for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
					{
						auto diff = Parameters.DepPosition(k, j) - Parameters.DepPosition(kk, j);
						dist += diff * diff;
					}
					prior += spring * log(dist);
				}
				T lbdist = 0;
				T trdist = 0;
				for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
				{
					auto ldiff = Settings.Prior.PriorBottomLeft[j] - Parameters.DepPosition(k, j);
					auto rdiff = Settings.Prior.PriorTopRight[j] - Parameters.DepPosition(k, j);
					lbdist += spring * ldiff * ldiff;
					trdist += spring * rdiff * rdiff;
				}
				prior += 0.01 * spring * (log(lbdist) + log(trdist));

				for (sint i = 0; i < Parameters.MatrixSize; ++i)
				{
					T d = Parameters.Phi(k, i) / Settings.Prior.phiSigma;
					prior -= 0.5 * d * d;
				}
				T Tkmax = -9999999;
				for (auto &tk : TkExpert[k])
				{
					if (tk > Tkmax) { Tkmax = tk; }
				}
				prior += Settings.Prior.DepartmentAlpha * Tkmax;

				// // HACK:  this is a dumb prior
				for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
				{
					T d = Parameters.DepPosition(k, j) / 100;
					prior -= d * d;
				}
			}

			for (sint e = 0; e < Ne; ++e)
			{

				for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
				{
					T d = Parameters.ExpertPosition(e, j) / 100;
					prior -= d * d;
				}
				T dmu = (Parameters.ExpertParameter(e, 0) - Settings.Prior.ExpertMu) / Settings.Prior.ExpertPropertiesSigma;
				T dsig = (log(Parameters.ExpertParameter(e, 1)) - log(Settings.Prior.ExpertSigma)) / Settings.Prior.ExpertPropertiesSigma;
				prior -= 0.5 * (dmu * dmu + dsig * dsig);
				T &sig = Parameters.ExpertParameter(e, 1);
				double minSig = 0.02;
				if (Parameters.ExpertParameter(e, 1) < minSig)
				{
					prior += 1e8 * log(sig / minSig);
				}
				if (sig > 2)
				{
					T d = (sig - 2) / 0.1;
					prior -= d * d;
				}
				//
				// T &lambda = Parameters.ExpertParameter(e, 2);
				// double minL = 0.1;
				// if (lambda < minL)
				// {
				// 	prior += 1e8 * log(lambda / minL);
				// }
				// T d = (lambda - 1) / 100;
				// prior -= d * d;
			}
			// for (sint e = 0; e < Ne; ++e)
			// {
			// }

			return Settings.Prior.PriorStrength * prior;
		}

		double BestScore;
		void ComputeHessian(std::vector<ClusteredTrains> train);
		void QueryContributions()
		{
			for (sint e = 0; e < Ne; ++e)
			{
				LOG(INFO) << "\tExpert " << e + 1 << " contributes " << exp(ExpertWeights[e]);
			}
			// for (sint k = 0; k < Nd; ++k)
			// {
			// 	LOG(INFO) << "\tTk(x) " << k + 1 << " contributes " << TkPos[k];
			// 	for (sint e = 0; e < Ne; ++e)
			// 	{
			// 		LOG(INFO) << "\t\tTk(e) " << k + 1 << "-" << e + 1 << " contributes " << TkExpert[e][k];
			// 	}
			// }
		}

	  private:
		T mean;
		T variance;
		ModelSettings &Settings;
		std::vector<T> TkPos;
		std::vector<std::vector<T>> TkExpert;

		Eigen::MatrixXd CholeskyMat;

		template <class A, class B>
		T ComputeDistance(A a, B b, size_t dep)
		{
			T dk = 0;
			for (sint ni = 0; ni < Settings.Hyper.InputDimension; ++ni)
			{
				double Ld_i = 0;
				for (sint nj = ni; nj < Settings.Hyper.InputDimension; ++nj)
				{
					Ld_i += (a(nj) - b(nj)) * Parameters.L(dep, nj, ni);
				}

				dk += Ld_i * Ld_i;
			}
			return dk;
		}
		template <class U>
		void ComputeTkFromVec(U setOfVectors, std::vector<T> &output)
		{
			T sum = 0;
			for (sint k = 0; k < Nd; ++k)
			{
				T dk = ComputeDistance(setOfVectors, [&](size_t idx) { return Parameters.DepPosition(k, idx); }, k);
				output[k] = LogKernel(dk, 1);
				if (k == 0)
				{
					sum = output[k];
				}
				else
				{
					sum = ale(sum, output[k]);
				}
			}
			// then normalise the Tks
			// whilst keeping them in log space
			for (sint k = 0; k < Nd; ++k)
			{
				output[k] -= sum;
			}
		}
		std::vector<T> ExpertWeights;
		sint Nd;
		sint Ne;
		void SetSizes()
		{
			TkPos.resize(Nd);
			ExpertWeights.resize(Ne);
			TkExpert.resize(Ne, std::vector<T>(Nd));
		}

		void ComputeDecomp(Eigen::MatrixXd &Hessian)
		{
			Eigen::LLT<Eigen::MatrixXd> llt(Hessian);
			int count = 0;
			double dx = 0.01;
			while (llt.info() != Eigen::Success)
			{
				if (count > 100)
				{
					LOG(ERROR) << "Decomposition failed! Hessian is not positive-definite.";
					break;
					// exit(1);
				}
				for (int i = 0; i < Hessian.rows(); ++i)
				{
					Hessian(i, i) += dx;
				}
				dx += 0.01;
				llt = Eigen::LLT<Eigen::MatrixXd>(Hessian);
				++count;
			}
			CholeskyMat = llt.matrixU();
			if (count > 0)
			{
				LOG(WARN) << "Initial Hessian was non-SPD. " << count << " rounds of diagonal-augmentation were applied.";
			}
		}
	};

	// specialisations
	template <>
	void Submodel<double>::Train(std::vector<ClusteredTrains> train, std::vector<ClusteredTrains> validate, std::optional<JSL::Display::Progress::Bar *> PB);
	template <>
	void Submodel<double>::ComputeHessian(std::vector<ClusteredTrains> train);
} // namespace FADE

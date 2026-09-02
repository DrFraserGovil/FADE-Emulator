#include "FADE/Distributions/Distribution.h"
#include <FADE/Distributions/Kernels.h>
#include <FADE/Models/Submodel.h>
#include <FADE/Utility/ale.h>
namespace FADE
{
	Submodel::Submodel(ModelSettings &parentSettings, sint nd, sint ne) : Parameters(parentSettings.Hyper, nd, ne), Settings(parentSettings), Nd(nd), Ne(ne)
	{
		LOG(DEBUG) << "Constructing submodel (" << nd << ", " << ne << ")";
		SetSizes();
		Parameters.Randomise(Settings.Prior);
		SyncParameters();
	}
	void Submodel::SyncParameters()
	{
		Parameters.UpdateDerived();
		CacheExpertDep();
	}
	void Submodel::SetPosition(const std::vector<double> &x)
	{
		CacheQueryDep(x);

		CacheQueryWik(x);

		CacheExpertWeights();
		CacheParameters();
	}
	double Submodel::LogGaussian(double y)
	{
		double sum = log(Pis[0]) + LogGaussianDistribution(y, Mus[0], Vrs[0]);
		for (sint p = 1; p < Settings.Hyper.ModeCount; ++p)
		{
			sum = ale(sum, log(Pis[p]) + LogGaussianDistribution(y, Mus[p], Vrs[p]));
		}
		return sum;
	}
	void Submodel::Save(JSL::IO::VaultWriter &vault)
	{
		std::string root = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne);
		vault[root + "/position.dat"] << Parameters.ToString();
		vault[root + "/score.dat"] << BestScore;

		// Eigen::Index rows = CholeskyMat.rows();
		// Eigen::Index cols = CholeskyMat.cols();
		// std::ostringstream os;
		// os << rows << " " << cols << "\n";
		// os << std::setprecision(17);
		// for (Eigen::Index r = 0; r < CholeskyMat.rows(); ++r)
		// {
		// 	for (Eigen::Index c = 0; c < CholeskyMat.cols(); ++c)
		// 	{
		// 		os << CholeskyMat(r, c) << (c + 1 == CholeskyMat.cols() ? "" : " ");
		// 	}
		// 	os << "\n";
		// }
		// vault[root + "/cholesky.dat"] << os.str();
	}
	void Submodel::Load(JSL::IO::VaultReader &vault)
	{
		SetSizes();

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
	}

	double Submodel::Score(TrainingData &data, bool validationNotTraining)
	{
		SyncParameters();
		auto &dataset = (validationNotTraining) ? data.Validation : data.Training;

		double prior = Prior();
		if (prior == Settings.Train.LogZero)
		{
			return Settings.Train.LogZero;
		}
		double score = 0;
		sint tot = 0;
		for (auto &cluster : dataset)
		{
			sint N = cluster.Values.size();
			tot += N;
			SetPosition(cluster.Position);
			for (sint n = 0; n < N; ++n)
			{
				score += cluster.LogWeights[n] + LogGaussian(cluster.Values[n]);
			}
		}
		return score / tot + prior;
	}
	double Submodel::CutPrior()
	{
		for (sint e = 0; e < Ne; ++e)
		{
			for (sint d = 0; d < Settings.Hyper.InputDimension; ++d)
			{
				double x = Parameters.ExpertPosition(e, d);
				if (x > Settings.Prior.PriorTopRight[d] || x < Settings.Prior.PriorBottomLeft[d])
				{
					return Settings.Train.LogZero;
				}
			}
		}

		return 0;
	}

	double Submodel::Prior()
	{
		auto prior = CutPrior();
		if (prior == Settings.Train.LogZero) { return prior; }

		for (sint e = 0; e < Ne; ++e)
		{
			for (sint p = 0; p < Settings.Hyper.ModeCount; ++p)
			{
				double var = Parameters.ExpertV(e, p);
				prior += -(Settings.Prior.priorVarAlpha + 1) * (log(var) + Settings.Prior.priorVarValue / var);
			}
		}
		// LOG(INFO) << prior;
		return prior * Settings.Prior.PriorStrength;
	}
	std::vector<double> Submodel::QueryExperts(std::vector<double> pos)
	{
		SetPosition(pos);
		std::vector<double> out(Ne);
		for (sint i = 0; i < Ne; ++i)
		{
			out[i] = ExpertWeights[i];
		}
		return out;
	}

	double Submodel::ComputeDistance(std::function<double(sint)> a, std::function<double(sint)> b, size_t dep)
	{
		double dk = 0;
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
	void Submodel::SetSizes()
	{
		ExpertWeights.resize(Ne);
		LogQueryDepartmentWeight.resize(Nd);
		LogExpertDepartmentWeight.resize(Ne, std::vector<double>(Nd));
		LogPerDepartmentExpertWeights.resize(Ne, std::vector<double>(Nd));

		sint P = Settings.Hyper.ModeCount;
		Mus.resize(P);
		Pis.resize(P);
		Vrs.resize(P);
	}
	void Submodel::CacheQueryDep(const std::vector<double> &pos)
	{
		if (Nd == 1)
		{
			LogQueryDepartmentWeight[0] = 0;
			return;
		}
		double sum = 0;
		for (sint k = 0; k < Nd; ++k)
		{
			auto dist = ComputeDistance(
				[&](sint i) { return Parameters.DepPosition(k, i); },
				[&](sint j) { return pos[j]; }, k);
			double term = LogGaussianKernel(dist, 1);
			if (k == 0)
			{
				sum = term;
			}
			else
			{
				sum = ale(sum, term);
			}
			// store unnormalised
			LogQueryDepartmentWeight[k] = term;
		}

		for (sint k = 0; k < Nd; ++k)
		{
			LogQueryDepartmentWeight[k] -= sum;
		}
	}
	void Submodel::CacheQueryWik(const std::vector<double> &pos)
	{
		for (sint k = 0; k < Nd; ++k)
		{
			double sum = 0;

			for (sint i = 0; i < Ne; ++i)
			{
				auto dist = ComputeDistance([&](sint idx) { return Parameters.ExpertPosition(i, idx); }, [&](sint idx) { return pos[idx]; }, k);
				double scale = Parameters.ExpertScale(i);
				double term = LogGaussianKernel(dist, scale);
				if (i == 0)
				{
					sum = term;
				}
				else
				{
					sum = ale(sum, term);
				}
				LogPerDepartmentExpertWeights[i][k] = term;
			}
			for (sint i = 0; i < Ne; ++i)
			{
				LogPerDepartmentExpertWeights[i][k] -= sum;
			}
		}
	}

	void Submodel::CacheExpertDep()
	{
		for (sint i = 0; i < Ne; ++i)
		{
			if (Nd == 1)
			{
				LogExpertDepartmentWeight[i][0] = 0;
			}
			else
			{
				double sum = 0;
				for (sint k = 0; k < Nd; ++k)
				{
					auto dist = ComputeDistance(
						[&](sint idx) { return Parameters.DepPosition(k, idx); },
						[&](sint jdx) { return Parameters.ExpertPosition(i, jdx); }, k);
					double term = LogGaussianKernel(dist, 1);
					if (k == 0)
					{
						sum = term;
					}
					else
					{
						sum = ale(sum, term);
					}
					// store unnormalised
					LogExpertDepartmentWeight[i][k] = term;
				}
				// then normalise
				for (sint k = 0; k < Nd; ++k)
				{
					LogExpertDepartmentWeight[i][k] -= sum;
				}
			}
		}
	}
	void Submodel::CacheExpertWeights()
	{
		for (sint i = 0; i < Ne; ++i)
		{
			double s = LogQueryDepartmentWeight[0] + LogPerDepartmentExpertWeights[i][0];

			for (sint k = 1; k < Nd; ++k)
			{
				s = ale(s, LogQueryDepartmentWeight[k] + LogPerDepartmentExpertWeights[i][k]);
			}
			ExpertWeights[i] = exp(s);
		}
	}
	void Submodel::CacheParameters()
	{
		for (sint p = 0; p < Settings.Hyper.ModeCount; ++p)
		{
			Mus[p] = 0;
			Pis[p] = 0;
			Vrs[p] = 0;
			for (sint i = 0; i < Ne; ++i)
			{
				Pis[p] += ExpertWeights[i] * Parameters.ExpertPi(i, p);
				Mus[p] += ExpertWeights[i] * Parameters.ExpertMu(i, p);
				Vrs[p] += ExpertWeights[i] * Parameters.ExpertV(i, p);
			}
		}
	}

} // namespace FADE

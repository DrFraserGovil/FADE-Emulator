#pragma once
#include "Distribution.h"
#include "Kernels.h"
#include "ParameterVector.h"
#include <JSL.h>
template <class T = double>
class FixedFADE
{
  public:
	FixedFADE(HyperParameters &hyper, sint depCount, sint expertCount) : Parameters(hyper, depCount, expertCount), Hyper(hyper)
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

	void GuassianPrediction(double y)
	{
		double v = 0;
		for (sint e = 0; e < Ne; ++e)
		{
			v += ExpertWeights[e] * GaussianDistribution(y, e, Parameters);
		}
	}

	void SetPosition(std::vector<double> &x)
	{
		ComputeTkFromVec([&x](sint i) -> double & { return x[i]; }, TkPos);
		for (sint k = 0; k < Nd; ++k)
		{
			T wsum = 0;
			for (sint i = 0; i < Ne; ++i)
			{
				T dk = ComputeDistance([&x](sint idx) -> double & { return x[idx]; }, [&](sint idx) -> T & { return Parameters.ExpertPosition(i, idx); }, k);
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

					ExpertWeights[i] = exp(TkExpert[i][k] - wsum + TkPos[k]);
				}
				else
				{
					ExpertWeights[i] += exp(TkExpert[i][k] - wsum + TkPos[k]);
				}
				TkExpert[i][k] -= wsum;
			}
		}
	}

	void Save(JSL::IO::VaultWriter &vault)
	{
		std::string param = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne) + ".param";

		// std::string param = root + "/vector.param";
		vault[param] << Parameters.ToString();
	}

	void Load(JSL::IO::VaultReader &vault)
	{
		std::string param = "model_d" + JSL::String::makeFrom(Nd) + "_e" + JSL::String::makeFrom(Ne) + ".param";
		if (!vault.Files().contains(param))
		{
			LOG(ERROR) << "Provided input is missing a submodel entry for " << param << ", despite the metadata indicating it exists.\nThe model is most likely corrupted";
			exit(1);
		}
		LOG(DEBUG) << "  - Loading submodel " << param << " from file";
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
	}

	ParameterVector<T> Parameters;

	std::pair<T, T> EstimateMoments()
	{
		if (Settings.Hyper.Family == "gaussian")
		{
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
		}
	}

  private:
	T mean;
	T variance;
	HyperParameters &Hyper;

	std::vector<T> TkPos;
	std::vector<std::vector<T>> TkExpert;

	template <class A, class B>
	T ComputeDistance(A a, B b, size_t dep)
	{
		T dk = 0;
		for (sint ni = 0; ni < Hyper.InputDimension; ++ni)
		{
			double Ld_i = 0;
			for (sint nj = ni; nj < Hyper.InputDimension; ++nj)
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
};

template <class T = double>
class FADE
{
  public:
	FADE() : Hyper()
	{
		for (sint nd = Settings.Hyper.Departments.first; nd <= Settings.Hyper.Departments.second; ++nd)
		{
			for (sint ne = Settings.Hyper.Experts.first; ne <= Settings.Hyper.Departments.second; ++ne)
			{
				Models.try_emplace({nd, ne}, Hyper, nd, ne);
			}
		}
	}
	FixedFADE<T> &operator[](std::pair<sint, sint> idx)
	{
		// assert(Models
		assert(Models.contains(idx));
		return Models.at(idx);
	}
	void Save(JSL::IO::VaultWriter &vault)
	{
		std::string hyperfile = "hyper.param";
		vault[hyperfile] << Hyper.ToString();

		forModelInModels([&vault](auto &model) { model.Save(vault); });
	}

	template <class U>
	void forModelInModels(U callback)
	{
		for (auto &[_, model] : Models)
		{
			callback(model);
		}
	}

	// we assume that the settings portion has already been read in and modified; we are just populating the submodels at this point
	void Load(JSL::IO::VaultReader &vault)
	{
		forModelInModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
		// LOG(INFO) << "Loaded model
	}

  private:
	std::map<std::pair<sint, sint>, FixedFADE<T>> Models;

	HyperParameters Hyper;
};

#pragma once
#include "Distribution.h"
#include "Kernels.h"
#include "ParameterVector.h"
template <class T = double>
class FADE
{
  public:
	void SetHyper(HyperParameters newHyper)
	{
		Hyper = newHyper;
		TkPos.resize(Hyper.Nd);
		ExpertWeights.resize(Hyper.Ne);
		Parameters.SetHyper(newHyper);
		TkExpert.resize(Hyper.Ne, std::vector<T>(Hyper.Nd));
	}

	void DeriveFromParameters()
	{
		k1Scale = (Parameters.ExpertScale() < 1) ? exp(Parameters.ExpertScale()) : Parameters.ExpertScale();

		k2Scale = (Parameters.DepartmentScale() < 1) ? exp(Parameters.DepartmentScale()) : Parameters.DepartmentScale();

		Parameters.ConvertMatrix();
		for (sint i = 0; i < Hyper.Ne; ++i)
		{
			ComputeTkFromVec([&](sint j) -> T & { return Parameters.ExpertPosition(i, j); }, TkExpert[i]);
		}
	}

	void GuassianPrediction(double y)
	{
		double v = 0;
		for (sint e = 0; e < Hyper.Ne; ++e)
		{
			v += ExpertWeights[e] * GaussianDistribution(y, e, Parameters);
		}
	}

	void SetPosition(std::vector<double> &x)
	{
		ComputeTkFromVec([&x](sint i) -> double & { return x[i]; }, TkPos);
		for (sint k = 0; k < Hyper.Nd; ++k)
		{
			T wsum = 0;
			for (sint i = 0; i < Hyper.Ne; ++i)
			{
				T dk = ComputeDistance([&x](sint idx) -> double & { return x[idx]; }, [&](sint idx) -> T & { return Parameters.ExpertPosition(i, idx); }, k);
				TkExpert[i][k] += LogKernel(dk, k1Scale);
				if (i == 0) { wsum = TkExpert[i][k]; }
				else
				{
					wsum = ale(wsum, TkExpert[i][k]);
				}
			}
			for (sint i = 0; i < Hyper.Ne; ++i)
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
	FADE(HyperParameters hyper) : Hyper(hyper), Parameters(hyper)
	{
		SetHyper(Hyper);
	}

	HyperParameters Hyper;
	ParameterVector<T> Parameters;

	std::vector<T> TkPos;
	std::vector<std::vector<T>> TkExpert;

	// private:
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
		for (sint k = 0; k < Hyper.Nd; ++k)
		{
			T dk = ComputeDistance(setOfVectors, [&](size_t idx) { return Parameters.DepPosition(k, idx); }, k);
			output[k] = LogKernel(dk, k2Scale);
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
		for (sint k = 0; k < Hyper.Nd; ++k)
		{
			output[k] -= sum;
		}
	}
	std::vector<T> ExpertWeights;
	std::vector<T> logTkCi;
	T k1Scale;
	T k2Scale;
};

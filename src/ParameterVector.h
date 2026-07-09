#pragma once
#include <JSL/Log.h>
#include <JSL/Strings.h>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

typedef size_t sint;
class HyperParameters
{
  public:
	sint InputDimension;
	sint OutputDimension;
	sint ProbabilityDimension;
	sint MatrixSize;
	//! The number of departments
	sint Nd;
	//! The number of experts
	sint Ne;
	HyperParameters(sint dimIn, sint dimOut, sint dimProb, sint depCount, sint expCount)
	{
		InputDimension = dimIn;
		OutputDimension = dimOut;
		ProbabilityDimension = dimProb;
		Nd = depCount;
		Ne = expCount;
		MatrixSize = dimIn * (dimIn + 1) / 2;
	}
};

template <class T = double>
class ParameterVector
{
  public:
	T &DepartmentScale()
	{
		return Params[0];
	}
	T &ExpertScale()
	{
		return Params[1];
	}
	T &ExpertPosition(sint expert, sint index)
	{
		return Params[ExpertStart + (Hyper.InputDimension * expert) + index];
	}
	T &ExpertParameter(sint expert, sint index)
	{
		return Params[DistStart + (Hyper.ProbabilityDimension * expert) + index];
	}

	T &DepPosition(sint department, sint index)
	{
		return Params[LooseParam + (Hyper.InputDimension * department) + index];
	}
	T &Phi(sint department, sint i, sint j)
	{
		assert(i >= j);
		sint idx = (i + 1) * i / 2 + j;

		return Params[PhiStart + MatrixSize * department + idx];
	}
	T &L(sint department, sint i, sint j)
	{
		assert(i >= j);
		sint idx = (i + 1) * i / 2 + j;
		return Lks[MatrixSize * department + idx];
	}
	void SetHyper(HyperParameters hyper)
	{
		Hyper = hyper;

		MatrixSize = Hyper.InputDimension * (Hyper.InputDimension + 1) / 2;

		TotalSize = (MatrixSize + Hyper.InputDimension) * Hyper.Nd + (Hyper.InputDimension + Hyper.ProbabilityDimension) * Hyper.Ne;
		Params.resize(TotalSize, 0);
		PhiStart = LooseParam + Hyper.InputDimension * Hyper.Nd;
		ExpertStart = PhiStart + MatrixSize * Hyper.Nd;
		DistStart = ExpertStart + Hyper.Ne * Hyper.InputDimension;
		Lks.resize(MatrixSize * Hyper.Nd, 0);
	}

	void SetExpertPosition(sint expert, std::vector<double> pos)
	{
		assert(pos.size() == Hyper.InputDimension);
		assert(expert < Hyper.Ne);
		for (sint j = 0; j < Hyper.InputDimension; ++j)
		{
			ExpertPosition(expert, j) = pos[j];
		}
	}

	void SetDepartmentPosition(sint dep, std::vector<double> pos)
	{
		assert(pos.size() == Hyper.InputDimension);
		assert(dep < Hyper.Nd);
		for (sint j = 0; j < Hyper.InputDimension; ++j)
		{
			DepPosition(dep, j) = pos[j];
		}
	}
	sint Size() { return Params.size(); }
	operator std::string()
	{
		return JSL::String::makeFrom<std::vector<T>>(Params);
	}

	friend std::ostream &operator<<(std::ostream &os, const ParameterVector &obj)
	{
		// Force the explicit conversion to sidestep template deduction issues
		return os << std::string(obj);
	}

	ParameterVector(HyperParameters hyper) : Hyper(hyper) { SetHyper(hyper); }

	void ConvertMatrix()
	{
		for (sint k = 0; k < Hyper.Nd; ++k)
		{
			for (sint i = 0; i < Hyper.InputDimension; ++i)
			{
				for (sint j = 0; j < i; ++j)
				{
					L(k, i, j) = Phi(k, i, j);
				}
				// double v = Phi(k, i, i);
				// if (v < 1)
				// {
				// 	v = exp(v - 1);
				// }
				// L(k, i, i) = v;
				L(k, i, i) = exp(Phi(k, i, i));
			}
		}
	}

  private:
	std::vector<T> Params;
	std::vector<T> Lks;
	sint LooseParam = 2;
	sint TotalSize;
	sint MatrixSize;  // InputDimension *(InputDimension + 1)/2
	sint PhiStart;	  // Hyper.InputDimension * Nd
	sint ExpertStart; // PhiStart + MatrixSize * Nd
	sint DistStart;	  // ExpertStart + Ne * Hyper.InputDimension
	HyperParameters Hyper;
};

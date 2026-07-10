#pragma once
#include "HyperParameters.h"
#include <JSL/IO/Vault/VaultWriter.h>
#include <JSL/Strings.h>
#include <cassert>
#include <string>
#include <vector>
template <class T = double>
class ParameterVector
{
  public:
	ParameterVector(HyperParameters &hyper, sint departmentCount, sint expertCount) : Hyper(hyper)
	{
		Ne = expertCount;
		Nd = departmentCount;
		DeriveDimensions();
	}

	sint Size() { return Params.size(); }

	void UpdateDerived()
	{
		ConvertMatrix();
	}
	std::string ToString()
	{
		std::ostringstream os;
		for (auto &p : Params)
		{
			os << p << "\n";
		}
		return os.str();
	}
	/////////////////////
	/// ACCESS FUNCTIONS
	/////////////////////
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

	void Load(std::vector<std::string> &fileData)
	{
		for (sint i = 0; i < TotalSize; ++i)
		{
			Params[i] = JSL::String::ParseTo<double>(fileData[i]);
		}
	}

	/*
		void SetExpertPosition(sint expert, std::vector<double> pos)
		{
			assert(pos.size() == Hyper.InputDimension);
			assert(expert < Ne);
			for (sint j = 0; j < Hyper.InputDimension; ++j)
			{
				ExpertPosition(expert, j) = pos[j];
			}
		}

		void SetDepartmentPosition(sint dep, std::vector<double> pos)
		{
			assert(pos.size() == Hyper.InputDimension);
			assert(dep < Nd);
			for (sint j = 0; j < Hyper.InputDimension; ++j)
			{
				DepPosition(dep, j) = pos[j];
			}
		}
	*/
  private:
	std::vector<T> Params;
	std::vector<T> Lks;
	sint LooseParam = 0;
	sint TotalSize;
	sint MatrixSize;  // InputDimension *(InputDimension + 1)/2
	sint PhiStart;	  // Hyper.InputDimension * Nd
	sint ExpertStart; // PhiStart + MatrixSize * Nd
	sint DistStart;	  // ExpertStart + Ne * Hyper.InputDimension
	HyperParameters &Hyper;
	sint Ne;
	sint Nd;
	void DeriveDimensions()
	{

		MatrixSize = Hyper.InputDimension * (Hyper.InputDimension + 1) / 2;

		TotalSize = (MatrixSize + Hyper.InputDimension) * Nd + (Hyper.InputDimension + Hyper.ProbabilityDimension) * Ne;
		Params.resize(TotalSize, 0);
		PhiStart = LooseParam + Hyper.InputDimension * Nd;
		ExpertStart = PhiStart + MatrixSize * Nd;
		DistStart = ExpertStart + Ne * Hyper.InputDimension;
		Lks.resize(MatrixSize * Nd, 0);
	}

	void ConvertMatrix()
	{
		for (sint k = 0; k < Nd; ++k)
		{
			for (sint i = 0; i < Hyper.InputDimension; ++i)
			{
				for (sint j = 0; j < i; ++j)
				{
					L(k, i, j) = Phi(k, i, j);
				}
				L(k, i, i) = exp(Phi(k, i, i));
			}
		}
	}
};

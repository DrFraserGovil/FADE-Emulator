#pragma once
#include "FADE/Distributions/PriorSettings.h"
#include <FADE/Parameters/HyperSettings.h>
#include <JSL/IO/Vault/VaultWriter.h>
#include <JSL/Log.h>
#include <JSL/Strings.h>
#include <cassert>
#include <string>
#include <vector>
namespace FADE
{
	typedef size_t sint;
	class ParameterVector
	{
	  public:
		ParameterVector(HyperSettings &hyper, sint departmentCount, sint expertCount);

		sint Size();

		void UpdateDerived();

		std::string ToString();

		/////////////////////
		/// ACCESS FUNCTIONS
		/////////////////////
		double &ExpertPosition(sint expert, sint index)
		{
			return Params[ExpertStart + (Hyper.InputDimension * expert) + index];
		}
		// double &ExpertParameter(sint expert, sint index)
		// {
		// 	return Params[DistStart + (Hyper.ProbabilityDimension * expert) + index];
		// }
		double &ExpertPi(sint expert, sint mode)
		{
			sint offset = Hyper.ProbabilityDimension * (Hyper.ModeCount * expert + mode);
			return Params[DistStart + offset + 0];
		}
		double &ExpertMu(sint expert, sint mode)
		{
			sint offset = Hyper.ProbabilityDimension * (Hyper.ModeCount * expert + mode);
			return Params[DistStart + offset + 1];
		}
		double &ExpertV(sint expert, sint mode)
		{
			sint offset = Hyper.ProbabilityDimension * (Hyper.ModeCount * expert + mode);
			return Params[DistStart + offset + 2];
		}
		double &ExpertScale(sint expert)
		{
			return Params[ScaleStart + expert];
		}
		double &DepPosition(sint department, sint index)
		{
			return Params[LooseParam + (Hyper.InputDimension * department) + index];
		}
		double &Phi(sint department, sint i, sint j)
		{
			assert(i >= j);
			sint idx = (i + 1) * i / 2 + j;

			return Params[PhiStart + MatrixSize * department + idx];
		}
		double &Phi(sint department, sint i)
		{
			return Params[PhiStart + MatrixSize * department + i];
		}
		double &L(sint department, sint i, sint j)
		{
			assert(i >= j);
			sint idx = (i + 1) * i / 2 + j;
			return Lks[MatrixSize * department + idx];
		}

		void Load(std::vector<std::string> &fileData);

		void Randomise(PriorSettings &prior);

		HyperSettings &Hyper;

		void Copy(const ParameterVector &origin);

		std::vector<double> Params;

	  private:
		sint MatrixSize;  // InputDimension *(InputDimension + 1)/2
		sint ExpertStart; // PhiStart + MatrixSize * Nd
		sint TotalSize;
		std::vector<double> Lks;
		sint LooseParam = 0;
		sint ScaleStart = 0;
		sint PhiStart;	// Hyper.InputDimension * Nd
		sint DistStart; // ExpertStart + Ne * Hyper.InputDimension
		sint Ne;
		sint Nd;
		void DeriveDimensions();

		void ConvertMatrix();
	};
} // namespace FADE

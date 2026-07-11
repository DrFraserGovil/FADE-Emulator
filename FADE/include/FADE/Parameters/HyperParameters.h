
#pragma once
#include "HyperSettings.h"
#include "JSL/Strings/Cases.h"
#include <JSL/Log.h>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>
typedef size_t sint;
class HyperParameters
{
  public:
	sint InputDimension = 1;
	sint OutputDimension = 1;
	sint ProbabilityDimension = 2;
	sint MatrixSize;

	sint MaxNd;
	sint MinNd;
	sint MaxNe;
	sint MinNe;
	HyperParameters()
	{
	}

	void Reset(const HyperSettings &defaults)
	{
		InputDimension = defaults.InputDimension;
		OutputDimension = defaults.OutputDimension;
		MatrixSize = InputDimension * (InputDimension + 1) / 2;
		SetModel(defaults.Family);
	}

	void SetModel(std::string model)
	{
		JSL::String::toLower(model);
		sint badModel = -1;
		ProbabilityDimension = badModel;
		if (model == "gaussian")
		{
			ProbabilityDimension = 2;
		}

		if (ProbabilityDimension == badModel)
		{
			LOG(ERROR) << "Hyperparameter handling for model type '" << model << "' has not been set; cannot determine hyperparameter dimensions";
			exit(1);
		}
	}

	std::string ToString()
	{
		std::ostringstream os;
		for (auto &p : ParameterOrdering)
		{
			os << *p << "\n";
		}
		return os.str();
	}

  private:
	std::vector<size_t *> ParameterOrdering = {&InputDimension, &OutputDimension, &ProbabilityDimension, &MatrixSize, &MaxNd, &MinNd, &MaxNe, &MinNe};
};

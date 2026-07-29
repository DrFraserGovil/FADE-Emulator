#pragma once
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Model Hyperparameters
class HyperSettings : public Aggregator<HyperSettings>
{
  public:
	//! @brief The (min,max) number of departments to run with
	//! @alias dep, partition
	std::pair<size_t, size_t> Departments = {1, 5};

	//! @brief The (min,max) number of experts to run with
	//! @alias expert, node
	std::pair<size_t, size_t> Experts = {2, 10};

	//! @brief The dimension of the emulation space
	//! @alias input-dimension
	size_t InputDimension = 1;

	//! @brief The dimension of the prediction space
	//! @alias output-dimension
	size_t OutputDimension = 1;

	//! @brief The number of Gaussian modes used for the model
	//! @alias modes p
	size_t ModeCount = 3;

	/// DERIVED VALUEs - not set at runtime, but derived from the above values
	size_t MatrixSize = 0;
	const static size_t ProbabilityDimension = 3; // Number of parameters needed to define a mode of a Gaussian mixture
#include "HyperSettings.HyperSettings.autogen"
};

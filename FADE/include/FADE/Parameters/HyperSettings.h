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

	//! @brief The distribution family to use
	//! @detail Valid options are [\"gaussian\"]
	//! @alias family
	std::string Family = "gaussian";

	//! @brief The dimension of the emulation space
	//! @alias input-dimension
	size_t InputDimension = 2;

	//! @brief The dimension of the prediction space
	//! @alias output-dimension
	size_t OutputDimension = 1;

#include "HyperSettings.HyperSettings.autogen"
};

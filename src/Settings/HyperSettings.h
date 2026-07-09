
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Model Hyperparameters
class HyperSettings : public Aggregator<HyperSettings>
{
  public:
	//! @brief The number of departments to run with
	//! @alias dep, partition
	size_t Departments = 2;

	//! @brief The number of experts to run with
	//! @alias expert, node
	size_t Experts = 3;

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

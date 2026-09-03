#pragma once
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Model Hyperparameters
class HyperSettings : public Aggregator<HyperSettings>
{
  public:
	//! @brief Activates a single-department run, equivalent to calling -depRange n,n
	//! @alias dep, partition
	std::optional<size_t> Departments;
	//! @brief Activates a single-expert run, equivalent to calling -expRange n,n
	//! @alias expert, node
	std::optional<size_t> Experts;

	//! @brief The (min,max) number of departments to run with
	//! @alias depRange, partitionRange
	std::pair<size_t, size_t> DepartmentRange = {1, 5};

	//! @brief The (min,max) number of experts to run with
	//! @alias expRange, nodeRange
	std::pair<size_t, size_t> ExpertRange = {2, 10};

	//! @brief The dimension of the emulation space
	//! @alias input-dimension
	size_t InputDimension = 1;

	//! @brief The number of Gaussian modes used for the model
	//! @alias modes p
	size_t ModeCount = 3;

	/// DERIVED VALUEs - not set at runtime, but derived from the above values
	size_t MatrixSize = 0;
	const static size_t ProbabilityDimension = 3; // Number of parameters needed to define a mode of a Gaussian mixture

	void SetRanges()
	{
		if (Departments)
		{
			DepartmentRange = {Departments.value(), Departments.value()};
		}
		if (Experts)
		{
			ExpertRange = {Experts.value(), Experts.value()};
		}
	}
#include "HyperSettings.HyperSettings.autogen"
};

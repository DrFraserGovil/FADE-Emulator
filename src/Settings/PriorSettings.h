
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

class PriorSettings : public Aggregator<PriorSettings>
{
  public:
	//! @brief Specifies the BL-boundary of the uniform rectangle in hyper-dimensional space
	//! @details If not specified, the boundary will be inferred from the training data
	//! @alias prior-bl
	std::vector<double> PriorBottomLeft = {};

	//! @brief Specifies the TR-boundary of the uniform rectangle in hyper-dimensional space
	//! @details If not specified, the boundary will be inferred from the training data
	//! @alias prior-tr
	std::vector<double> PriorTopRight = {};

#include "PriorSettings.PriorSettings.autogen"
};

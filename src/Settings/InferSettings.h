#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class InferenceSettings: public Aggregator<InferenceSettings>
{
  public:
	//! @brief The maximum number of iterations before training is force-completed
	//! @alias max-iteration
	size_t MaxIteration = -1;
	#include "InferSettings.InferenceSettings.autogen"
};

#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class TrainingSettings : public Aggregator<TrainingSettings>
{
  public:
	//! @brief The maximum number of iterations before training is force-completed
	//! @alias max-iteration
	size_t MaxIteration = -1;
#include "TrainingSettings.TrainingSettings.autogen"
};

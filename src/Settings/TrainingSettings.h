#include "PriorSettings.h"
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class TrainingSettings : public Aggregator<TrainingSettings>
{
  public:
	PriorSettings Priors;

	//! @brief The name to which the model instance is saved. Loading this file at a later date allows inference with the trained model
	//! @alias save model-out
	std::string OutputFiles = "model.fde";

	//! @brief The maximum number of iterations before training is force-completed
	//! @alias max-iteration
	size_t MaxIteration = -1;

#include "TrainingSettings.TrainingSettings.autogen"
};

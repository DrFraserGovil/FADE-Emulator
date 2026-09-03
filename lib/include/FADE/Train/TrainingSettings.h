#include "AnnealingSettings.h"
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class TrainingSettings : public Aggregator<TrainingSettings>
{
  public:
	//! @brief The name to which the model instance is saved. Loading this file at a later date allows inference with the trained model
	//! @alias save model-out
	std::string OutputFiles = "model.fde";

	//! @brief The maximum number of iterations before training is force-completed
	//! @alias max-iteration
	size_t MaxIteration = -1;

	//! @brief The fraction of training data allocated to the validation pool
	//! @details The validation pool is used to generate the
	//! @alias validate
	double ValidationFraction = 0.2;

	double LogZero = -999999999999999;
	AnnealingSettings Annealing;

#include "TrainingSettings.TrainingSettings.autogen"
};

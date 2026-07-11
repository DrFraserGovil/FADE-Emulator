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
	//! @alias validate
	double ValidationFraction = 0.2;

	//! @brief Points Closer together than this are considered at the same point in the emulation space
	//! @detail This increases the efficiency of the training, at a marginal loss in accuracy
	//! @alias train-clustering
	double ClusteringRadius = 1e-6;

#include "TrainingSettings.TrainingSettings.autogen"
};

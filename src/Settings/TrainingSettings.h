#include "PriorSettings.h"
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class TrainingSettings : public Aggregator<TrainingSettings>
{
  public:
	PriorSettings Priors;
#include "TrainingSettings.TrainingSettings.autogen"
};

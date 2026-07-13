
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name Training Settings
class AnnealingSettings : public Aggregator<AnnealingSettings>
{
  public:
	//! @brief The probability of performing a big-leap during annealing proposation
	//! @alias prob-bigleap
	double BigLeapProbability = 0.2;

	//! @brief The factor applied to the movement distance during a big-leap
	//! @alias factor-bigleap
	double BigLeapFactor = 10;

	//! @brief The probability that a move will be accepted, even if it would normally be rejected
	//! @alias prob-forceaccept
	double ForceAcceptProbability = 0.005;

	//! @brief The fraction of dimensions which recieve an update during each annealing proposal
	//! @alias annealing-update-fraction
	double UpdateFraction = 0.1;

	double StartTemp = 1;
	double EndTemp = 0.0001;

	//! @brief The number of iterations to ruin in the annealing routine
	//! @alias anneal-steps
	size_t Iterations = 10000;

#include "AnnealingSettings.AnnealingSettings.autogen"
};

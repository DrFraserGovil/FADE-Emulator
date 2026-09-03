
#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

//! @name SimulatedAnnealing Settings
class AnnealingSettings : public Aggregator<AnnealingSettings>
{
  public:
	//! @brief The probability of performing a big-leap during annealing proposation
	//! @alias prob-bigleap
	double BigLeapProbability = 0.1;

	//! @brief The factor applied to the movement distance during a big-leap
	//! @alias factor-bigleap
	double BigLeapFactor = 10;

	//! @brief The probability that a move will be accepted, even if it would normally be rejected
	//! @alias prob-forceaccept
	double ForceAcceptProbability = 0.02;

	//! @brief The fraction of dimensions which recieve an update during each annealing proposal
	//! @details At higher dimensions, this value may need to be lower
	//! @alias annealing-update-fraction
	double UpdateFraction = 0.3;

	//! @brief The starting temperature of the Annealing routine
	//! @details Higher temperatures mean worse models are accepted earlier; thereby exploring more of the space
	//! @alias start-temp, T0
	double StartTemp = 1;

	//! @brief The final target temperature of the Annealing routine
	//! @details This temperature is the value used to determine the cooling rate: it may not represent the actual final temperature unless the model is perfectly well behaved
	//! @aliad end-temp
	double EndTemp = 0.001;

	//! @brief The number of iterations to ruin in the annealing routine
	//! @alias anneal-steps
	size_t Iterations = 10000;

#include "AnnealingSettings.AnnealingSettings.autogen"
};

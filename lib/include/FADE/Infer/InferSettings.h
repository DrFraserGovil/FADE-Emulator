#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

namespace FADE
{
	class InferenceSettings : public Aggregator<InferenceSettings>
	{
	  public:
		//! @brief The model to be used for prediction
		//! @detail If no value provided, the Files will be searched for a valid model file; otherwise the code will exit with an error.
		//! @alias model, m
		std::optional<std::string> ModelFile = std::nullopt;

		//! @brief The query resolution in y-space
		//! @alias query-resolution r resolution
		size_t Resolution = 100;

		//! @brief The CDF threshold at which the auto-inference takes place
		//! @brief If a bound is not specified on the y-range, the bounds are placed such that the CDF is bounded at [threshold, 1-threshold]
		//! @alias infer-bound
		double CDFBound = 0.001;

#include "InferSettings.InferenceSettings.autogen"
	};
} // namespace FADE

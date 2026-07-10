#include <JSL/Interface/Aggregator.h>
using namespace JSL::Interface;

class InferenceSettings : public Aggregator<InferenceSettings>
{
  public:
	//! @brief The model to be used for prediction
	//! @detail If no value provided, the Files will be searched for a valid model file; otherwise the code will exit with an error.
	//! @alias model, m
	std::optional<std::filesystem::path> ModelFile = std::nullopt;
#include "InferSettings.InferenceSettings.autogen"
};

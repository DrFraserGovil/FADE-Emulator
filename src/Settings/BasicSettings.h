
#include "HyperSettings.h"
#include "InferSettings.h"
#include "TrainingSettings.h"
#include <JSL/Interface/Aggregator.h>
#include <filesystem>
//! @name Basic Settings
//! @command test Enters the testing mode for basic diagnostics
//! @command train Activates training mode
//! @command [file] All other positional arguments are interpreted as input files (meaning varies by mode)
class BasicSettings : public JSL::Interface::Aggregator<BasicSettings>
{
  public:
	HyperSettings Hyper;
	TrainingSettings Training;
	InferenceSettings Inference;

	//! @alias v verbose
	//! @brief If true, recieve DEBUG level logs to the output stream. Takes priority over Quiet mode.
	bool Verbose = false;

	//! @alias q quiet
	//! @brief If true, recieve only ERROR logs to the output stream; everything else is suppressed unless Verbose is also set to true
	bool Quiet = false;

	//! @alias file, input
	//! @brief A set of files to be processed. Any files passed as positional arguments will also be stored here
	std::set<std::filesystem::path> Files = {};

	//! @alias settings
	//! @brief If set, this value is used to save the value of the configuration file
	std::optional<std::string> ExportFile = std::nullopt;
#include "BasicSettings.BasicSettings.autogen"
};

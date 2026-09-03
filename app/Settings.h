#include <FADE/ModelSettings.h>
#include <JSL/Interface/Aggregator.h>
#include <filesystem>
#include <set>
using namespace FADE;
//! @name Basic Settings
//! @command test Enters the testing mode for basic diagnostics
//! @command train Activates training mode
//! @command predict Activates inference mode
//! @command [file] All other positional arguments are interpreted as input files (meaning varies by mode)
class AppSettings : public JSL::Interface::Aggregator<AppSettings>
{
  public:
	//! @alias v verbose
	//! @brief If true, recieve DEBUG level logs to the output stream. Takes priority over Quiet mode.
	bool Verbose = false;

	//! @alias q quiet
	//! @brief If true, recieve only ERROR logs to the output stream; everything else is suppressed unless Verbose is also set to true
	bool Quiet = false;

	//! @alias file, input
	//! @brief A set of files to be processed. Any files passed as positional arguments will also be stored here
	std::set<std::filesystem::path> Files = {};

	ModelSettings Model;

	//! @alias settings
	//! @brief If set, this value is used to save the value of the configuration file
	std::optional<std::string> ExportFile = std::nullopt;

	//! @alias parallel N
	//! @brief The number of parallel threads that can be spawned in addition to the main thread.
	size_t ParallelThreads = 0;

	//! @brief The location that query data is saved to
	//! @alias query-out
	std::filesystem::path QueryOut = "QueryOutput.dat";

#include "Settings.AppSettings.autogen"
};

extern AppSettings Settings;

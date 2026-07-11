#include "Settings.h"
#include "modes.h"
#include <FADE.h>
#include <JSL.h>
// This is the complement to the extern declaration; this defines the global settings object
AppSettings Settings;

void WelcomeMessage()
{
	int n = 20;
	std::ostringstream os;
	std::string title = "FADE Emulator";
	int buffer = std::max(0, (n - (int)title.size()) / 2);
	os << std::string(n, '-') << "\n"
	   << std::string(buffer, ' ') << "FADE Emulator \n"
	   << std::string(n, '-') << "\n";

	LOG(INFO) << os.str();
}

enum class Mode
{
	Test,
	Train,
	Infer,
	Unpack,
};

void checkMode(std::optional<Mode> &mode, std::string cmd, std::string name, Mode pair)
{
	if (JSL::String::iEquals(cmd, name))
	{
		if (mode)
		{
			LOG(ERROR) << "Cannot set mode to " << cmd
					   << " as it has already been set";
			exit(1);
		}
		mode = pair;
	}
}

Mode Initialise(int argc, char **argv)
{
	auto cmds = Settings.Parse(argc, argv);

	// Set up the logger
	JSL::Log::Global().Level = INFO;
	JSL::Log::Global().ShowHeaders = false;
	JSL::Log::Global().DebugColour = JSL::Display::Colour(60, 60, 130);
	JSL::Log::Global().AlignSize(30);
	if (Settings.Verbose)
	{
		JSL::Log::Global().Level = DEBUG;
		JSL::Log::Global().ShowHeaders = true;
	}
	else if (Settings.Quiet)
	{
		JSL::Log::Global().Level = ERROR;
	}
	std::optional<Mode> out;
	for (auto cmd : cmds)
	{
		std::optional<Mode> tmp;

		checkMode(tmp, cmd, "test", Mode::Test);
		checkMode(tmp, cmd, "train", Mode::Train);
		checkMode(tmp, cmd, "infer", Mode::Infer);
		checkMode(tmp, cmd, "predict", Mode::Infer);
		checkMode(tmp, cmd, "unpack", Mode::Unpack);
		if (!tmp)
		{
			LOG(DEBUG) << cmd
					   << " not recognised as a command; appending to input files";
			Settings.Files.emplace(cmd);
		}
		else
		{
			out = tmp;
		}
	}
	if (!out)
	{
		Settings.Help();
		exit(0);
	}

	if (Settings.ExportFile)
	{
		LOG(DEBUG) << "Exporting config to " << Settings.ExportFile.value();
		Settings.Export(Settings.ExportFile.value());
	}

	return out.value();
}

int main(int argc, char **argv)
{
	auto mode = Initialise(argc, argv);
	WelcomeMessage();
	LOG(DEBUG) << "Processing input: " << Settings.Files;
	switch (mode)
	{
		case Mode::Train:
			TrainModel(Settings.Files);
			break;
		case Mode::Infer:
			Predict(Settings.Files);
			break;
		case Mode::Unpack:
			Unpack(Settings.Files);
			break;
		default:
			LOG(ERROR) << "Mode not yet implemented";
			exit(1);
			break;
	}

	LOG(INFO) << "FADE actions completed. Exiting with code 0";
	return 0;
}

#include "FADE.h"
#include "ParameterVector.h"
#include "Settings.h"
#include "Train.h"
#include "Unpack.h"
#include <JSL.h>
BasicSettings Settings;

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

void TestMode(std::optional<Mode> &mode, std::string cmd, std::string name,
	Mode pair)
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

		TestMode(tmp, cmd, "test", Mode::Test);
		TestMode(tmp, cmd, "train", Mode::Train);
		TestMode(tmp, cmd, "infer", Mode::Infer);
		TestMode(tmp, cmd, "predict", Mode::Infer);
		TestMode(tmp, cmd, "unpack", Mode::Unpack);
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
	return out.value();
}

int main(int argc, char **argv)
{
	auto mode = Initialise(argc, argv);
	WelcomeMessage();
	// HyperParameters hyper(2, 1, 2, Settings.Hyper.Departments,
	// Settings.Hyper.Experts);

	LOG(DEBUG) << "Processing input: " << Settings.Files;
	// FADE F(hyper);
	switch (mode)
	{
		case Mode::Train:
			Train();
			break;
		case Mode::Unpack:
			Unpack();
			break;
		default:
			LOG(ERROR) << "Mode not yet implemented";
			exit(1);
			break;
	}
	return 0;
}

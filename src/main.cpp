#include "FADE.h"
#include "ParameterVector.h"
#include "Settings.h"
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
};

void TestMode(std::optional<Mode> &mode, std::string cmd, std::string name, Mode pair)
{
	if (JSL::String::iEquals(cmd, name))
	{
		if (mode)
		{
			LOG(ERROR) << "Cannot set mode to " << cmd << " as it has already been set";
			exit(1);
		}
		mode = pair;
	}
}

void Initialise(int argc, char **argv)
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
		TestMode(out, cmd, "test", Mode::Test);
		TestMode(out, cmd, "train", Mode::Train);
		TestMode(out, cmd, "infer", Mode::Infer);
		TestMode(out, cmd, "predict", Mode::Infer);
	}
	if (!out)
	{
		Settings.Help();
		exit(0);
	}
}

int main(int argc, char **argv)
{
	Initialise(argc, argv);
	WelcomeMessage();
	HyperParameters hyper(2, 1, 2, Settings.Hyper.Departments, Settings.Hyper.Experts);

	FADE F(hyper);
	LOG(INFO) << "Fade initialised";
	return 0;
}

#include <FADE/Emulator.h>

namespace FADE
{
	Emulator::Emulator()
	{
		// Set the model to default settings
		Settings.Reset();
	}
	Emulator::Emulator(int argc, char **argv)
	{
		Settings.Parse(argc, argv);
	}
	Emulator::Emulator(ModelSettings &settings)
	{
		Settings = settings;
	}

} // namespace FADE

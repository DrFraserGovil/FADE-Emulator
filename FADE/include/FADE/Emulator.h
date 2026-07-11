#pragma once
#include <FADE/ModelSettings.h>
namespace FADE
{
	class Emulator
	{
	  public:
		//! Blank constructor; default initialises the settings
		Emulator();
		//! Parses the model settings directly from the command line
		Emulator(int argc, char **argv);

		//! Accepts an externally configured model settings as the input
		Emulator(ModelSettings &settings);

	  private:
		ModelSettings Settings;
	};
} // namespace FADE

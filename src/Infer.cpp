#include "Infer.h"
#include "Settings.h"
#include <JSL/IO/Vault.h>
#include <JSL/Log.h>
#include <JSL/Vectors/Search.h>
#include <filesystem>
std::filesystem::path findModel()
{
	std::filesystem::path out;
	if (Settings.Inference.ModelFile)
	{
		out = Settings.Inference.ModelFile.value();
	}
	else
	{
		LOG(INFO) << "No " << JSL::Display::Italics() << "model" << JSL::Display::Italics(false) << " key passed to settings\n\t-> Searching through the input files for a valid model";
		for (auto file : Settings.Files)
		{
			try
			{
				auto v = JSL::IO::VaultReader(file.string());
				if (JSL::Vector::contains(v.Files(), "train.config"))
				{
					if (!out.empty())
					{
						LOG(ERROR) << "Multiple model files passed at once (" << file.string() << ", " << out.string() << "). Choose one.";
						exit(1);
					}
					LOG(DEBUG) << file << " passed modelfile checks";
					out = file;
				}
			}
			catch (...)
			{
				LOG(DEBUG) << file << " failed modelfile checks";
			}
		}
	}

	if (out.empty())
	{
		LOG(ERROR) << "Could not locate a suitable model file.\nModel files should be passed via the --model flag";
		exit(1);
	}

	LOG(INFO) << "Loading model data from " << out.string();
	return out;
}

void Infer()
{
	LOG(INFO) << JSL::Display::Colour(100, 100, 30, true) << JSL::Display::White() << "Selected: Prediction Mode" << JSL::Display::ResetAll();
	auto file = findModel();
}

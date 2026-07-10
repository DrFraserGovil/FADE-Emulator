#include "Infer.h"
#include "FADE.h"
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
		LOG(INFO) << "No " << JSL::Display::Italics() << "model" << JSL::Display::Italics(false) << " key passed to settings\n\tSearching through the input files for a valid model";
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

void MergeSettings(JSL::IO::VaultReader &vault)
{
	BasicSettings copy;

	if (!vault.Files().contains("train.config"))
	{
		LOG(ERROR) << "Model file does not have a vaild configuration module. \nThe model file is most likley corrupted";
		exit(1);
	}

	auto lines = vault["train.config"].AsLines();
	copy.Configure(lines, " ");

	LOG(INFO) << "Local hyperparameter mirroring training values";
	Settings.Hyper = copy.Hyper;
}

void Load(FADE<double> &model)
{
}

void Infer()
{
	LOG(INFO) << JSL::Display::Colour(100, 100, 30, true) << JSL::Display::White() << "Selected: Prediction Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over

	FADE<double> model;

	auto file = findModel();

	JSL::IO::VaultReader vault(file.string());
	MergeSettings(vault);
	model.Load(vault);
	LOG(INFO) << "Model loading complete";

	LOG(INFO) << "Inference routine completed";
	JSL::Log::Global().IndentLevel--;
}

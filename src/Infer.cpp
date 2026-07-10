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
	bool needsErase = false;
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
					needsErase = true;
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
	if (needsErase)
	{
		// switch 'em over
		Settings.Files.erase(out);
		Settings.Inference.ModelFile = out;
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
	auto file = findModel();

	JSL::IO::VaultReader vault(file.string());
	MergeSettings(vault);
	model.Load(vault);
	LOG(INFO) << "Model loading complete";
}

typedef std::optional<std::pair<double, double>> queryRange;

std::map<std::vector<double>, queryRange> GetQueries()
{
	LOG(INFO) << "Loading query points";

	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	std::map<std::vector<double>, queryRange> out;
	for (auto &f : Settings.Files)
	{
		try
		{
			const size_t N = Settings.Hyper.InputDimension;
			JSL::IO::forConvertedLineIn<std::vector<double>>(f, [&out, N](auto vec) {
				if (vec.size() == N)
				{
					if (!out.contains(vec))
					{
						LOG(WARN) << "Duplicate queries for " << vec << "detected; defaulting to moment-based range";
					}
					out[vec] = std::nullopt;
				}
				else
				{
					if (vec.size() == N + 2)
					{
						std::pair p{std::move(vec[vec.size() - 2]), std::move(vec.back())};
						vec.resize(vec.size() - 2);
						out[vec] = p;
					}
					else
					{
						LOG(WARN) << "Ignoring " << vec << "; dimensions don't match";
					}
				}
			});
		}
		catch (...)
		{
			LOG(WARN) << f << " not a valid inference query file";
		}
	}

	if (out.empty())
	{
		LOG(ERROR) << "No valid query points were provided. No inference can occur.";
		exit(1);
	}

	return out;
}

void Infer()
{
	LOG(INFO) << JSL::Display::Colour(100, 100, 30, true) << JSL::Display::White() << "Selected: Prediction Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over

	FADE<double> model;

	// populates the model with contents
	Load(model);

	// get the query points
	auto q = GetQueries();

	LOG(INFO) << "Inference routine completed";
	JSL::Log::Global().IndentLevel--;
}

#include "../Settings.h"
#include "../modes.h"
#include "FADE/Infer/InferPoint.h"
#include <FADE/ModelSettings.h>
#include <FADE/Models/FADE.h>
#include <FADE/Train/Train.h>
#include <JSL.h>
#include <vector>
std::filesystem::path findModel(std::optional<std::filesystem::path> modelfile, std::set<std::filesystem::path> &files)
{
	std::filesystem::path out;
	bool needsErase = false;
	if (modelfile)
	{
		out = modelfile.value();
	}
	else
	{
		LOG(INFO) << "No " << JSL::Display::Italics() << "model" << JSL::Display::Italics(false) << " key passed to settings\n\tSearching through the input files for a valid model";
		for (auto file : files)
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
		files.erase(out);
	}

	LOG(INFO) << "Loading model data from " << out.string();
	return out;
}
std::map<std::vector<double>, FADE::QueryPoint> GetQueries()
{
	LOG(INFO) << "Loading query points";

	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	std::map<std::vector<double>, QueryPoint> out;
	for (auto &f : Settings.Files)
	{
		LOG(INFO) << "Searching " << f.string();
		try
		{
			const size_t N = Settings.Model.Hyper.InputDimension;
			JSL::IO::forLineIn(f, [&out, N](auto line) {
				auto vec = JSL::String::ParseTo<std::vector<double>>(line, " ");
				if (vec.size() == N)
				{
					if (out.contains(vec))
					{
						LOG(WARN) << "Duplicate queries for " << vec << " detected; defaulting to moment-based range";
					}
					out[vec] = {vec, {}};
				}
				else
				{
					if (vec.size() == N + 2)
					{
						std::vector<double> p{std::move(vec[vec.size() - 2]), std::move(vec.back())};
						vec.resize(vec.size() - 2);
						out[vec] = {vec, p};
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
	LOG(INFO) << out.size() << " query points found";
	return out;
}

void Predict(std::set<std::filesystem::path> paths)
{
	LOG(INFO) << JSL::Display::Colour(40, 130, 130, true) << JSL::Display::White() << "Selected: Prediction Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over

	auto mfile = findModel(Settings.Model.Infer.ModelFile, paths);

	LOG(INFO) << "Spooling up model";
	FADE::Model<double> model(Settings.Model.Hyper);
	model.Load(mfile, Settings.Model);

	auto out = GetQueries();

	LOG(ERROR) << "Prediction not yet fully implemented";
	exit(1);
}

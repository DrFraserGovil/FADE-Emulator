#include "../Settings.h"
#include "../modes.h"
#include "FADE/Infer/InferPoint.h"
#include <FADE/ModelSettings.h>
#include <FADE/Models/Model.h>
#include <FADE/Train/Train.h>
#include <JSL.h>
#include <optional>

bool isVault(std::string path)
{
	try
	{
		auto v = JSL::IO::VaultReader(path);
		if (JSL::Vector::contains(v.Files(), "train.config"))
		{
			return true;
		}
	}
	catch (...)
	{
		LOG(DEBUG) << path << " failed modelfile checks";
	}
	return false;
}

std::filesystem::path findModel(std::optional<std::filesystem::path> modelfile, std::set<std::filesystem::path> &files)
{
	std::filesystem::path out;
	bool needsErase = false;
	if (modelfile)
	{

		out = modelfile.value();
		if (!isVault(out))
		{
			LOG(ERROR) << out << " passed as --model flag, but is not a valid model file";
			exit(1);
		}
	}
	else
	{
		LOG(INFO) << "No " << JSL::Display::Italics() << "model" << JSL::Display::Italics(false) << " key passed to settings\n\tSearching through the input files for a valid model";
		for (auto file : files)
		{
			if (isVault(file))
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
std::set<FADE::QueryPoint> GetQueries()
{
	LOG(INFO) << "Loading query points";

	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	std::set<QueryPoint> out;
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
					if (out.contains({vec, {}, {}}))
					{
						LOG(WARN) << "Duplicate queries for " << vec << " detected; defaulting to moment-based range";
					}
					QueryPoint qp{vec, {}, {}};
					out.insert(qp);
				}
				else
				{
					if (vec.size() == N + 2)
					{
						std::vector<double> p{std::move(vec[vec.size() - 2]), std::move(vec.back())};
						auto grid = JSL::Vector::range(p[0], p[1], Settings.Resolution);
						vec.resize(vec.size() - 2);
						out.insert({vec, grid, {}});
						// out[vec] = {vec, p};
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

	LOG(INFO) << "Spooling up model from " << mfile.string();
	FADE::Model model(Settings.Model);
	Settings.Model = model.GetSettings();
	std::set<QueryPoint> out = GetQueries();

	bool haveWarned = false;
	for (auto &p : out)
	{
		if (p.PredictionGrid.empty())
		{
			if (!haveWarned)
			{
				LOG(WARN) << "Adaptive grid sizes are not yet supported; defaulting to a preset grid";
				haveWarned = true;
			}
			p.PredictionGrid = JSL::Vector::range(-0.1, 1.5, Settings.Resolution);
		}
	}

	model.Predict(out);

	auto nd = Settings.Model.Hyper.Departments;
	auto ne = Settings.Model.Hyper.Experts;
	for (sint k = nd.first; k <= nd.second; ++k)
	{
		for (sint i = ne.first; i <= ne.second; ++i)
		{
			auto file = Settings.QueryOut;
			auto ext = file.extension();
			file.replace_extension("");
			file = file.string() + "_" + std::to_string(k) + "_" + std::to_string(i) + ext.string();
			auto stream = JSL::IO::openStream(file);

			for (auto &q : out)
			{
				stream << "New query: " << q.EmulationPoint[0];
				for (sint r = 1; r < q.EmulationPoint.size(); ++r)
				{
					stream << " " << q.EmulationPoint[r];
				}
				stream << "\n";
				stream << q.PredictionGrid[0];
				sint N = q.PredictionGrid.size();
				for (sint r = 1; r < N; ++r)
				{
					stream << " " << q.PredictionGrid[r];
				}
				auto &pred = q.SubmodelValues[{k, i}];
				N = pred.size();
				stream << "\n"
					   << pred[0];
				for (sint r = 1; r < N; ++r)
				{
					stream << " " << pred[r];
				}
				stream << "\n";
			}
			stream.close();
		}
	}
	// for (auto &q : out)
	// {
	// 	stream << "New query: " << q.EmulationPoint[0];
	// 	for (sint r = 1; r < q.EmulationPoint.size(); ++r)
	// 	{
	// 		stream << " " << q.EmulationPoint[r];
	// 	}
	// 	stream << "\n";
	// 	stream << q.PredictionGrid[0];
	// 	sint N = q.PredictionGrid.size();
	// 	for (sint r = 1; r < N; ++r)
	// 	{
	// 		stream << " " << q.PredictionGrid[r];
	// 	}
	// 	N = q.PredictionValues.size();
	// 	stream << "\n"
	// 		   << q.PredictionValues[0];
	// 	for (sint r = 1; r < N; ++r)
	// 	{
	// 		stream << " " << q.PredictionValues[r];
	// 	}
	// 	stream << "\n";
	// }
	// stream.close();
}

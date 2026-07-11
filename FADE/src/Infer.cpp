#include <FADE/Infer/Infer.h>
#include <FADE/Models/FADE.h>
#include <JSL/IO/Vault.h>
#include <JSL/Log.h>
#include <JSL/Vectors/Search.h>
#include <filesystem>
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

void MergeSettings(JSL::IO::VaultReader &vault)
{
	LOG(ERROR) << "Settings merge not fixed under new paradigm";
	exit(1);
	// BasicSettings copy;
	//
	// if (!vault.Files().contains("train.config"))
	// {
	// 	LOG(ERROR) << "Model file does not have a vaild configuration module. \nThe model file is most likley corrupted";
	// 	exit(1);
	// }
	//
	// auto lines = vault["train.config"].AsLines();
	// copy.Configure(lines, " ");
	//
	// Settings.Hyper = copy.Hyper;
	// LOG(INFO) << "Local hyperparameter mirroring training values";
}

// void Load(FADE<double> &model)
// {
// 	auto file = findModel();
//
// 	JSL::IO::VaultReader vault(file.string());
// 	MergeSettings(vault);
// 	model.Load(vault);
// 	LOG(INFO) << "Model loading complete";
// }

typedef std::optional<std::pair<double, double>> queryRange;

std::map<std::vector<double>, queryRange> GetQueries()
{
	// LOG(INFO) << "Loading query points";
	//
	// auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	// std::map<std::vector<double>, queryRange> out;
	// for (auto &f : Settings.Files)
	// {
	// 	LOG(INFO) << "Searching " << f.string();
	// 	try
	// 	{
	// 		const size_t N = Settings.Hyper.InputDimension;
	// 		JSL::IO::forLineIn(f, [&out, N](auto line) {
	// 			auto vec = JSL::String::ParseTo<std::vector<double>>(line, " ");
	// 			if (vec.size() == N)
	// 			{
	// 				if (out.contains(vec))
	// 				{
	// 					LOG(WARN) << "Duplicate queries for " << vec << " detected; defaulting to moment-based range";
	// 				}
	// 				out[vec] = std::nullopt;
	// 			}
	// 			else
	// 			{
	// 				if (vec.size() == N + 2)
	// 				{
	// 					std::pair p{std::move(vec[vec.size() - 2]), std::move(vec.back())};
	// 					vec.resize(vec.size() - 2);
	// 					out[vec] = p;
	// 				}
	// 				else
	// 				{
	// 					LOG(WARN) << "Ignoring " << vec << "; dimensions don't match";
	// 				}
	// 			}
	// 		});
	// 	}
	// 	catch (...)
	// 	{
	// 		LOG(WARN) << f << " not a valid inference query file";
	// 	}
	// }
	//
	// if (out.empty())
	// {
	// 	LOG(ERROR) << "No valid query points were provided. No inference can occur.";
	// 	exit(1);
	// }
	// LOG(INFO) << out.size() << " query points found";
	// return out;
}

namespace FADE
{
	void Infer(ModelSettings &settings, std::vector<QueryPoint> queries)
	{
		LOG(INFO) << JSL::Display::Colour(100, 100, 30, true) << JSL::Display::White() << "Selected: Prediction Mode" << JSL::Display::ResetAll();
		auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over

		LOG(ERROR) << "Inference not restablished under new model";
		FADE<double> model(settings);
		//
		// // populates the model with contents
		// Load(model);
		//
		// // get the query points
		// auto q = GetQueries();
		//
		// std::vector<double> out;
		// for (auto &[point, range] : q)
		// {
		// 	if (range)
		// 	{
		// 		model.SetPosition(point);
		// 		auto pred = JSL::Vector::range(range.value().first, range.value().second, Settings.Inference.Resolution);
		// 		std::vector<double> out(pred.size());
		// 		for (sint i = 0; i < pred.size(); ++i)
		// 		{
		// 			out[i] = model[{5, 5}].GuassianPrediction(pred[i]);
		// 		}
		// 		LOG(INFO) << out;
		// 	}
		// }
		//
		// LOG(INFO) << "Inference routine completed";
		// JSL::Log::Global().IndentLevel--;
	}
} // namespace FADE

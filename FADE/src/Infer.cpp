#include <FADE/Infer/Infer.h>
#include <FADE/Models/FADE.h>
#include <JSL/IO/Vault.h>
#include <JSL/Log.h>
#include <JSL/Vectors/Search.h>
#include <filesystem>

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
typedef std::optional<std::pair<double, double>> queryRange;

namespace FADE
{
	void Infer(ModelSettings &settings, std::vector<QueryPoint> queries)
	{

		LOG(ERROR) << "Inference not restablished under new model";
		FADE::Model<double> model(settings);
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

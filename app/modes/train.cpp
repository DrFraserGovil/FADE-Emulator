#include "../Settings.h"
#include "../modes.h"
#include <FADE/ModelSettings.h>
#include <FADE/Models/FADE.h>
#include <FADE/Train/Train.h>
#include <JSL.h>
#include <vector>
using namespace FADE;
std::vector<TrainingPoint> ExtractData(std::set<std::filesystem::path> files, ModelSettings &model)
{
	std::vector<TrainingPoint> out;
	auto tmp = JSL::Log::Indent();

	for (auto &f : files)
	{
		size_t warnCount = 0;
		LOG(INFO) << "Searching " << f.string();
		try
		{
			const size_t minSize = model.Hyper.InputDimension + 2;
			JSL::IO::forLineIn(f, [&out, &minSize, &warnCount](auto line) {
				auto vec = JSL::String::ParseTo<std::vector<double>>(line, " ");
				if (vec.size() >= minSize)
				{
					out.emplace_back(vec, minSize - 2);
				}
				else
				{
					++warnCount;
				}
			});
		}
		catch (...)
		{
			LOG(WARN) << f << " not a valid inference query file";
		}
		if (warnCount > 0)
		{
			LOG(WARN) << "Skipped " << warnCount << " invalid datapoints in " << f;
		}
	}

	if (out.empty())
	{
		LOG(ERROR) << "No training data found";
		exit(1);
	}
	LOG(INFO) << "Loaded " << out.size() << " training points";
	return out;
}

void TrainModel(std::set<std::filesystem::path> paths)
{
	LOG(INFO) << JSL::Display::Colour(30, 100, 30, true) << JSL::Display::White() << "Selected: Training Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over

	LOG(INFO) << "Loading training data";
	auto data = ExtractData(paths, Settings.Model);

	LOG(INFO) << "Creating model instance";
	FADE::Model<double> Model(Settings.Model);

	Model.Train(data);
}

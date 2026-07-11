#include "train.h"
#include "../Settings.h"
#include <FADE/ModelSettings.h>
#include <FADE/Train/Train.h>
#include <JSL.h>
#include <vector>
using namespace FADE;
std::vector<TrainingPoint> ExtractData(std::set<std::filesystem::path> files, ModelSettings &model)
{
	std::vector<TrainingPoint> out;
	LOG(INFO) << "Loading training data";
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
	LOG(WARN) << "No training routines are currently implemented. The current placeholder randomly generates a FADE instance.";
	auto data = ExtractData(paths, Settings.Model);

	LOG(ERROR) << "Model does not yet train itself....";
	exit(1);
}

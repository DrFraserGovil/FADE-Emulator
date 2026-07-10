#include "Train.h"
#include "FADE.h"
#include "Settings.h"
#include <JSL/IO/Vault.h>

using namespace JSL::String;
template <class T>
void SaveModel(FADE<T> &model)
{
	JSL::IO::VaultWriter Output(Settings.Training.OutputFiles, JSL::IO::Policy::Generous);

	// save the current settings to file
	Output.NewFile("train.config");
	Output << Settings.ExportAsString();

	model.Save(Output);

	Output.Close();
}

struct TrainingPoint
{
	std::vector<double> Position;
	double Weight;
	double Value;
	TrainingPoint(std::vector<double> &vec, const size_t &size)
	{
		Position.resize(size);
		for (sint i = 0; i < size; ++i)
		{
			Position[i] = vec[i];
		}
		Weight = vec[size];
		Value = vec[size + 1];
	}
};

std::pair<std::vector<TrainingPoint>, std::vector<TrainingPoint>> ExtractData()
{
	std::vector<TrainingPoint> out;
	std::vector<TrainingPoint> validate;
	LOG(INFO) << "Loading training data";
	auto tmp = JSL::Log::Indent();

	double rfac = 1.0 / RAND_MAX;
	for (auto &f : Settings.Files)
	{
		size_t warnCount = 0;
		LOG(INFO) << "Searching " << f.string();
		try
		{
			const size_t minSize = Settings.Hyper.InputDimension + 2;
			JSL::IO::forLineIn(f, [&out, &validate, &minSize, &warnCount, &rfac](auto line) {
				auto vec = JSL::String::ParseTo<std::vector<double>>(line, " ");
				if (vec.size() >= minSize)
				{
					double r = random() * rfac;
					if (r <= Settings.Training.ValidationFraction)
					{
						validate.emplace_back(vec, minSize - 2);
					}
					else
					{
						out.emplace_back(vec, minSize - 2);
					}
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
	if (validate.empty())
	{
		LOG(ERROR) << "No validation data found";
		exit(1);
	}
	LOG(INFO) << "Loaded " << out.size() << " training points and " << validate.size() << " validation points";
	return {out, validate};
}

void Train()
{
	LOG(INFO) << JSL::Display::Colour(30, 100, 30, true) << JSL::Display::White() << "Selected: Training Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	LOG(WARN) << "No training routines are currently implemented. The current placeholder randomly generates a FADE instance.";
	//
	FADE model;

	auto [data, validate] = ExtractData();

	std::vector<double> maxPos = data[0].Position;
	std::vector<double> minPos = data[1].Position;
	double minY = data[0].Value;
	double maxY = data[0].Value;
	for (auto &d : data)
	{
		if (d.Position > maxPos) { maxPos = d.Position; }
		if (d.Position < minPos) { minPos = d.Position; }
		if (d.Value > maxY) { maxY = d.Value; }
		if (d.Value < minY) { minY = d.Value; }
	}
	LOG(INFO) << "Loaded data is bounded by\n"
			  << "\tPositions: " << minPos << " -> " << maxPos << "\n"
			  << "\tValues:    " << minY << " -> " << maxY;
	if (Settings.Training.Priors.PriorBottomLeft.empty())
	{
		Settings.Training.Priors.PriorBottomLeft = minPos;
	}
	if (Settings.Training.Priors.PriorTopRight.empty())
	{
		Settings.Training.Priors.PriorTopRight = maxPos;
	}

	model.forModelInModels([&](FixedFADE<double> &model) {
		for (sint r = 0; r < model.Parameters.Size(); ++r)
		{
			double q = rand() * 1.0 / RAND_MAX;
			model.Parameters.Params[r] = q;
		}
	});

	SaveModel(model);
}

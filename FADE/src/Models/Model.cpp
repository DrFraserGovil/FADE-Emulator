#include <FADE/Models/Model.h>

namespace FADE
{

	Model::Model(ModelSettings &settings) : Settings(settings)
	{
		if (Settings.Infer.ModelFile)
		{
			Load(Settings.Infer.ModelFile.value());
		}
		else
		{
			ConstructModels();
		}
	}
	void Model::Train([[maybe_unused]] TrainingData &data, [[maybe_unused]] sint extraThreads)
	{
		auto [bottomLeft, topRight] = data.GetBounds();
		if (Settings.Prior.PriorBottomLeft.size() != bottomLeft.size())
		{
			Settings.Prior.PriorBottomLeft = bottomLeft;
			LOG(INFO) << "The BL-bound has been infered to be " << bottomLeft;
		}
		if (Settings.Prior.PriorTopRight.size() != topRight.size())
		{
			Settings.Prior.PriorTopRight = topRight;
			LOG(INFO) << "The TR-bound has been infered to be " << topRight;
		}

		// i.e. if we didn't just load in a pre-existing  model
		if (!Settings.Infer.ModelFile)
		{
			forAllModels([&](auto &model) { model.Parameters.Randomise(Settings.Prior); });
		}

		forAllModels([&](auto &model) { model.Train(data); });

		Save();
	}
	void Model::Train(std::vector<TrainingPoint> &data, sint extraThreads)
	{

		TrainingData group(data, Settings.Train.ValidationFraction, Settings.Train.ClusteringRadius);
		Train(group, extraThreads);
	}

	Submodel &Model::operator[](std::pair<sint, sint> idx)
	{
		assert(Models.contains(idx));
		return Models.at(idx);
	}

	void Model::SetPosition(std::vector<double> pos)
	{
		forAllModels([&pos](auto &model) { model.SetPosition(pos); });
	}

	void Model::Load(std::filesystem::path vaultPath)
	{
		LOG(INFO) << "Loading settings from file " << vaultPath;
		auto vault = JSL::IO::VaultReader(vaultPath.string());
		if (!vault.Files().contains("train.config"))
		{
			LOG(ERROR) << "Model file does not have a vaild configuration module. \nThe model file is most likley corrupted";
			exit(1);
		}

		auto lines = vault["train.config"].AsLines();
		Settings.Configure(lines, " ");
		ConstructModels();
		forAllModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
	}

	ModelSettings Model::GetSettings() { return Settings; }

	void Model::Predict(std::set<QueryPoint> &queries)
	{
		LOG(INFO) << "Beginning inference loop";
		auto tmp = JSL::Log::Indent();

		std::vector<double> out;
		for (auto &query : queries)
		{
			sint N = query.PredictionGrid.size();
			out.resize(N);
			// query.PredictionValues.resize(N);
			LOG(INFO) << "Inferring at position" << query.EmulationPoint;
			// forAllModels([&](auto &model) {
			for (auto &[id, model] : Models)
			{
				model.SetPosition(query.EmulationPoint);
				std::ostringstream os;

				for (sint ne = 0; ne < id.second; ++ne)
				{
					os << "Expert " << ne << "\n";
					for (sint p = 0; p < Settings.Hyper.ModeCount; ++p)
					{
						os << "\t(pi,mu,sigma) = " << model.Parameters.ExpertPi(ne, p) << " / " << model.Parameters.ExpertMu(ne, p) << " / " << model.Parameters.ExpertV(ne, p) << "\n";
					}
					os << "\n";
				}
				LOG(INFO) << os.str();

				for (sint j = 0; j < N; ++j)
				{
					out[j] = exp(model.LogGaussian(query.PredictionGrid[j]));
				}

				//! HACK: This is just whilst we're on single-only models
				query.SubmodelValues[id] = out;
			};
		}
	}

	void Model::ConstructModels()
	{

		for (sint nd = Settings.Hyper.Departments.first; nd <= Settings.Hyper.Departments.second; ++nd)
		{
			for (sint ne = Settings.Hyper.Experts.first; ne <= Settings.Hyper.Experts.second; ++ne)
			{
				Models.try_emplace({nd, ne}, Settings, nd, ne);
			}
		}
	}
	void Model::Save()
	{
		auto vault = JSL::IO::VaultWriter(Settings.Train.OutputFiles, JSL::IO::Policy::Generous);
		vault["train.config"] << Settings.ExportAsString();
		forAllModels([&vault](auto &model) { model.Save(vault); });
	}

} // namespace FADE

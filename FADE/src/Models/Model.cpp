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

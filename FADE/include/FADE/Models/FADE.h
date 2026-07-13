#pragma once
#include "Submodel.h"

namespace FADE
{

	template <class T = double>
	class Model
	{
	  public:
		Model(ModelSettings &settings)
		{
			Settings = settings;
			if (settings.Infer.ModelFile)
			{
				Load(settings);
			}
			Settings.Hyper.MatrixSize = Settings.Hyper.InputDimension * (Settings.Hyper.InputDimension + 1) / 2;
			Settings.Hyper.ProbabilityDimension = 1;
			for (sint nd = settings.Hyper.Departments.first; nd <= settings.Hyper.Departments.second; ++nd)
			{
				for (sint ne = settings.Hyper.Experts.first; ne <= settings.Hyper.Departments.second; ++ne)
				{
					Models.try_emplace({nd, ne}, Settings, nd, ne);
				}
			}
		}

		void Train(std::vector<TrainingPoint> &trainingData)
		{
			auto [train, validate] = ProcessTrainingData(trainingData, Settings.Train.ValidationFraction, Settings.Train.ClusteringRadius, Settings.Train.MaximumClusterCount);
			forModelInModels([&](auto &model) { model.Train(train, validate); });
		}

		Submodel<T> &operator[](std::pair<sint, sint> idx)
		{
			// assert(Models
			assert(Models.contains(idx));
			return Models.at(idx);
		}
		void Save(JSL::IO::VaultWriter &vault)
		{
			// std::string settings.Hyper.ile = "settings.Hyper.param";
			// vault[settings.Hyper.ile] << Hyper.ToString();

			forModelInModels([&vault](auto &model) { model.Save(vault); });
		}

		template <class U>
		void forModelInModels(U callback)
		{
			for (auto &[_, model] : Models)
			{
				callback(model);
			}
		}

		void SetPosition(std::vector<double> pos)
		{
			forModelInModels([&pos](auto &model) { model.SetPosition(pos); });
		}

		// we assume that the settings portion has already been read in and modified; we are just populating the submodels at this point
		void Load(JSL::IO::VaultReader &vault, HyperSettings &newsettings)
		{
			// Hyper.Reset(newsettings.Hyper.;
			forModelInModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
		}

		void Load(ModelSettings &settings)
		{
			Settings = settings;
		}

	  private:
		std::map<std::pair<sint, sint>, Submodel<T>> Models;

		// HyperParameters Hyper;
		ModelSettings Settings;
	};
} // namespace FADE

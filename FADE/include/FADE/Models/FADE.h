#pragma once
#include "FADE/Infer/InferPoint.h"
#include "Submodel.h"
#include <thread>
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
				Load(settings.Infer.ModelFile.value());
			}
			else
			{
				ConstructModels();
			}
		}

		void Train(std::vector<TrainingPoint> &trainingData, size_t extraThreads = 0)
		{

			auto [train, validate] = ProcessTrainingData(trainingData, Settings.Train.ValidationFraction, Settings.Train.ClusteringRadius, Settings.Train.MaximumClusterCount, Settings.Prior);
			LOG(INFO) << "Beginning training of submodels";
			auto tmp = JSL::Log::Indent();

			auto PB = JSL::Display::Progress::Bar(std::vector<size_t>{Models.size(), Settings.Train.Annealing.Iterations + Settings.Train.Annealing.OptimIterations}, 80);
			std::string idt(16, ' ');
			auto pre = std::vector<std::string>{idt, idt};
			PB.SetPrefix(pre);
			forModelInModels([&](auto &model) {
				std::vector<Submodel<T>> trainers(extraThreads + 1, model);
				std::vector<std::thread> cores;
				for (sint j = 0; j < extraThreads; ++j)
				{
					cores.emplace_back(&Submodel<T>::Train, &trainers[j], train, validate, std::nullopt);
				}
				trainers[extraThreads].Train(train, validate, &PB);

				for (auto &t : cores)
				{
					t.join();
				}

				// copy the best spawned instance into the 'true' submodel
				auto bestE = trainers[0].Score(train);
				sint bestI = 0;
				for (sint i = 0; i < extraThreads; ++i)
				{
					auto test = trainers[i].Score(train);
					if (test > bestE)
					{
						bestE = test;
						bestI = i;
					}
				}
				model.CopyPosition(trainers[bestI]);
				model.BestScore = trainers[bestI].Score(train);
				model.ComputeHessian(train);
			});
			LOG(INFO) << "Model training complete";
			Save();
		}

		Submodel<T> &operator[](std::pair<sint, sint> idx)
		{
			// assert(Models
			assert(Models.contains(idx));
			return Models.at(idx);
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

		void Load(std::filesystem::path vaultPath)
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
			forModelInModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
		}

		ModelSettings GetSettings()
		{
			return Settings;
		}

		void Predict(std::set<QueryPoint> &queries)
		{
			LOG(INFO) << "Beginning inference loop";
			auto tmp = JSL::Log::Indent();

			for (auto &query : queries)
			{
				sint N = query.PredictionGrid.size();
				// query.PredictionValues.resize(N);
				LOG(INFO) << "Inferring at position" << query.EmulationPoint;
				forModelInModels([&](auto &model) {
					model.SetPosition(query.EmulationPoint);
					model.QueryContributions();
					std::vector<double> out(N, 0);
					for (sint j = 0; j < N; ++j)
					{
						out[j] = exp(model.LogGaussian(query.PredictionGrid[j]));
					}

					//! HACK: This is just whilst we're on single-only models
					query.PredictionValues = out;
				});
			}
		}

	  private:
		std::map<std::pair<sint, sint>, Submodel<T>> Models;

		// HyperParameters Hyper;
		ModelSettings Settings;

		void ConstructModels()
		{
			Settings.Hyper.MatrixSize = Settings.Hyper.InputDimension * (Settings.Hyper.InputDimension + 1) / 2;
			for (sint nd = Settings.Hyper.Departments.first; nd <= Settings.Hyper.Departments.second; ++nd)
			{
				for (sint ne = Settings.Hyper.Experts.first; ne <= Settings.Hyper.Experts.second; ++ne)

				{
					Models.try_emplace({nd, ne}, Settings, nd, ne);
				}
			}
		}

		void Save()
		{
			auto vault = JSL::IO::VaultWriter(Settings.Train.OutputFiles, JSL::IO::Policy::Generous);

			vault["train.config"] << Settings.ExportAsString();

			forModelInModels([&vault](auto &model) { model.Save(vault); });
		}

		void PrepareForTrain()
		{
		}
	};
} // namespace FADE

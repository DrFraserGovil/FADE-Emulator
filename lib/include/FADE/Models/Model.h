#pragma once
#include "Submodel.h"
#include <Eigen/Dense>
#include <FADE/Infer/QueryPoint.h>
#include <filesystem>
namespace FADE
{
	class Model
	{
	  public:
		Model(ModelSettings &settings);

		void Train(TrainingData &data, sint extraThreads = 0);
		void Train(std::vector<ClusteredData> &data, sint extraThreads = 0);

		Submodel &operator[](std::pair<sint, sint> idx);

		void SetPosition(std::vector<double> pos);

		void Load(const std::filesystem::path &vaultPath);

		ModelSettings GetSettings();
		void Predict(std::set<QueryPoint> &queries);
		std::map<std::pair<sint, sint>, Submodel> Models;

	  private:
		ModelSettings Settings;

		void BLPFit(TrainingData &data, std::vector<double> &bl, std::vector<double> &tr);
		Eigen::VectorXd KinvY;
		std::vector<Eigen::VectorXd> BLPPos;

		template <class U>
		void forAllModels(U callback)
		{
			for (auto &[_, model] : Models)
			{
				callback(model);
			}
		}

		void ConstructModels();

		void Save(TrainingData &data);
		void BLPSave(TrainingData &data, JSL::IO::VaultWriter &vault);
		void BLPLoad(JSL::IO::VaultReader &vault);
	};
} // namespace FADE

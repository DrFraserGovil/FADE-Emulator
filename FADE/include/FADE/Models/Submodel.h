#pragma once

#include <Eigen/Dense>
#include <FADE/ModelSettings.h>
#include <FADE/Parameters/ParameterVector.h>
#include <FADE/Train/TrainingData.h>
#include <JSL/IO/Vault.h>
#include <functional>
namespace FADE
{

	class Submodel
	{
	  public:
		Submodel(ModelSettings &parentSettings, sint depCount, sint expertCount);

		//! @brief Sets the internal cache values that can be determined solely from expert/department positions
		void SyncParameters();

		void SetPosition(const std::vector<double> &x);
		// void CopyPosition(Submodel &model);

		double LogGaussian(double y);

		void Save(JSL::IO::VaultWriter &vault);

		void Load(JSL::IO::VaultReader &vault);

		ParameterVector Parameters;

		void Train(TrainingData &data);

		double Score(TrainingData &data, bool validationNotTraining = false);

		// double CutPrior();
		//
		// double Prior(bool hardPrior = true);
		std::vector<double> QueryExperts(std::vector<double> pos);

	  private:
		double BestScore;
		ModelSettings &Settings;
		std::vector<double> LogQueryDepartmentWeight;
		std::vector<std::vector<double>> LogExpertDepartmentWeight;
		std::vector<std::vector<double>> LogPerDepartmentExpertWeights;

		std::vector<double> Mus;
		std::vector<double> Pis;
		std::vector<double> Vrs;

		void EMFit();

		double ComputeDistance(std::function<double(sint)> a, std::function<double(sint)> b, size_t dep);
		std::vector<double> ExpertWeights;
		const sint Nd;
		const sint Ne;
		void SetSizes();

		void ComputeDecomp(Eigen::MatrixXd &Hessian);
		void CacheQueryDep(const std::vector<double> &pos);
		void CacheQueryWik(const std::vector<double> &pos);
		void CacheExpertDep();
		void CacheExpertWeights();
		void CacheParameters();
	};

} // namespace FADE

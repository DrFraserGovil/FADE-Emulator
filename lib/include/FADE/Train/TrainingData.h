#pragma once
#include <array>
#include <vector>
namespace FADE
{

	struct ClusteredData
	{
		std::vector<double> Position;
		std::vector<double> Values;
		std::vector<double> LogWeights;
		ClusteredData(std::vector<double> &vec, const size_t &xDimension);

		// double DistanceTo(TrainingPoint &x);
		// void Add(TrainingPoint &x);
	};

	class TrainingData
	{
	  public:
		std::vector<ClusteredData> Training;
		std::vector<ClusteredData> Validation;
		TrainingData(std::vector<ClusteredData> &data, double fraction);
		std::array<std::vector<double>, 2> GetBounds();

		double minExpectedMu = -5;
		double maxExpectedMu = 5;

	  private:
		std::vector<double> bottomLeft;
		std::vector<double> topRight;
	};
} // namespace FADE

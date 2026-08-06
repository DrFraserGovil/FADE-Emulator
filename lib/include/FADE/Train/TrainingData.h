#pragma once
#include <array>
#include <vector>
namespace FADE
{
	struct TrainingPoint
	{
		std::vector<double> Position;
		double Weight;
		double Value;
		TrainingPoint(std::vector<double> &vec, const size_t &size);
	};

	struct ClusteredData
	{
		std::vector<double> Position;
		std::vector<double> Values;
		std::vector<double> LogWeights;
		ClusteredData(TrainingPoint &x);
		double DistanceTo(TrainingPoint &x);
		void Add(TrainingPoint &x);
	};

	class TrainingData
	{
	  public:
		std::vector<ClusteredData> Training;
		std::vector<ClusteredData> Validation;
		TrainingData(std::vector<TrainingPoint> &data, double fraction, double clusterSize);
		std::array<std::vector<double>, 2> GetBounds();

	  private:
		std::vector<double> bottomLeft;
		std::vector<double> topRight;
	};
} // namespace FADE

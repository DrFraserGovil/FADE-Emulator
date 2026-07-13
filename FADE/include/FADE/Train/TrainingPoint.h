#pragma once
#include <vector>
namespace FADE
{

	struct TrainingPoint
	{
		std::vector<double> Position;
		double Weight;
		double Value;
		TrainingPoint(std::vector<double> &vec, const size_t &size)
		{
			Position.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				Position[i] = vec[i];
			}
			Weight = vec[size];
			Value = vec[size + 1];
		}
	};

	struct ClusteredTrains
	{
		std::vector<double> Position;
		std::vector<double> Values;
		std::vector<double> LogWeights;
		bool ReachedLimit = false;
		ClusteredTrains(TrainingPoint &x);
		double DistanceTo(TrainingPoint &x);
		void Add(TrainingPoint &x, size_t limit);
	};

	std::pair<std::vector<ClusteredTrains>, std::vector<ClusteredTrains>> ProcessTrainingData(std::vector<TrainingPoint> &data, double fraction, double clusterSize, size_t clusterLimit);
} // namespace FADE

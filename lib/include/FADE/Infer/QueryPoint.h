#pragma once
#include <map>
#include <vector>
namespace FADE
{
	struct QueryPoint
	{
		QueryPoint(std::vector<double> &vec);
		QueryPoint(std::vector<double> &vec, double lowerbound, double upperbound, int res);
		std::vector<double> EmulationPoint;
		mutable std::vector<double> PredictionGrid;
		mutable std::map<std::pair<size_t, size_t>, std::vector<double>> SubmodelProbability;
		mutable std::map<std::pair<size_t, size_t>, std::vector<double>> SubmodelCDF;
		friend bool operator<(const QueryPoint &lhs, const QueryPoint &rhs)
		{
			return lhs.EmulationPoint < rhs.EmulationPoint;
		}
	};
} // namespace FADE

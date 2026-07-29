#pragma once
#include <map>
#include <vector>
namespace FADE
{
	struct QueryPoint
	{
		std::vector<double> EmulationPoint;
		mutable std::vector<double> PredictionGrid;
		mutable std::vector<double> PosteriorPredictive;
		mutable std::map<std::pair<size_t, size_t>, std::vector<double>> SubmodelValues;
		friend bool operator<(const QueryPoint &lhs, const QueryPoint &rhs)
		{
			return lhs.EmulationPoint < rhs.EmulationPoint;
		}
	};
} // namespace FADE

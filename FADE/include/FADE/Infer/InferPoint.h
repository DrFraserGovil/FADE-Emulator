#pragma once
#include <vector>
namespace FADE
{
	struct QueryPoint
	{
		std::vector<double> EmulationPoint;
		mutable std::vector<double> PredictionGrid;
		mutable std::vector<double> PredictionValues;
		friend bool operator<(const QueryPoint &lhs, const QueryPoint &rhs)
		{
			return lhs.EmulationPoint < rhs.EmulationPoint;
		}
	};
} // namespace FADE

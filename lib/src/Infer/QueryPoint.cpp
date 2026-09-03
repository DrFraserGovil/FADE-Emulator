#include "FADE/Infer/QueryPoint.h"
#include <JSL/Vectors/Range.h>

namespace FADE
{
	QueryPoint::QueryPoint(std::vector<double> &vec)
	{
		EmulationPoint = std::move(vec);
		PredictionGrid.resize(0);
	}
	QueryPoint::QueryPoint(std::vector<double> &vec, double lowerbound, double upperbound, int res)
	{
		EmulationPoint = std::move(vec);
		PredictionGrid = JSL::Vector::range(lowerbound, upperbound, res);
	}
} // namespace FADE

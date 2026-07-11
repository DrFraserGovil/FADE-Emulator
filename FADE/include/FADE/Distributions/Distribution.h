#pragma once
#include <FADE/Parameters/ParameterVector.h>

namespace FADE
{
	const double pi_norm = 1.0 / sqrt(2 * M_PI);
	template <class T>
	T GaussianDistribution(double x, sint expertID, ParameterVector<T> &param)
	{
		T d = (x - param.ExpertPosition(expertID, 0)) / param.ExpertPosition(expertID, 1);
		return pi_norm / param.ExpertPosition(expertID, 1) * exp(-0.5 * d * d);
	}
} // namespace FADE

#pragma once
#include <FADE/Parameters/ParameterVector.h>

namespace FADE
{
	const double log_pi_norm = log(1.0 / sqrt(2 * M_PI));
	template <class T>
	T LogGaussianDistribution(double x, sint expertID, ParameterVector<T> &param)
	{
		T d = (x - param.ExpertPosition(expertID, 0)) / param.ExpertPosition(expertID, 1);
		return log_pi_norm - log(param.ExpertPosition(expertID, 1)) - 0.5 * d * d;
	}
} // namespace FADE

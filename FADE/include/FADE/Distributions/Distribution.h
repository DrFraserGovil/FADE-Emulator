#pragma once
#include <FADE/Parameters/ParameterVector.h>
#include <JSL/Log.h>
namespace FADE
{
	const double log_pi_norm = log(1.0 / sqrt(2 * M_PI));
	template <class T>
	T LogGaussianDistribution(double x, sint expertID, ParameterVector<T> &param)
	{
		T sigma = abs(param.ExpertParameter(expertID, 1)) + 0.01;
		T d = (x - param.ExpertParameter(expertID, 0)) / sigma;
		return log_pi_norm - log(sigma) - 0.5 * d * d;
	}
} // namespace FADE

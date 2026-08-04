#pragma once
#include <cmath>

namespace FADE
{
	double inline LogGaussianKernel(double squaredDistance, double scale)
	{
		return -0.5 * squaredDistance / (1e-10 + scale * scale);
		// return
	}

	// #include <vector>
	// std::vector<double> inline exp(std::vector<double> input)
	// {
	// 	std::vector<double> out(input.size());
	// 	for (size_t i = 0; i < input.size(); ++i)
	// 	{
	// 		out[i] = exp(input[i]);
	// 	}
	// 	return out;
	// }
} // namespace FADE

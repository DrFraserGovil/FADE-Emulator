#pragma once
#include <cmath>

template <class T = double, class U = double>
T inline LogKernel(T squaredDistance, U boundary)
{
	return -0.5 * squaredDistance / (0.0001 + boundary * boundary);
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
template <class T>
T ale(T logx, T logy)
{
	if (logx > logy)
	{
		return logx + log1p(exp(logy - logx));
	}
	else
	{
		return logy + log1p(exp(logx - logy));
	}
}

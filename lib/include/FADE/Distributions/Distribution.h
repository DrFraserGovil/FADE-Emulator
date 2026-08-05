#pragma once
#include <algorithm>
#include <cmath>
#include <numbers>
namespace FADE
{
	const double log_pi_norm = log(1.0 / sqrt(2 * std::numbers::pi));
	const double invSqrt2 = 1.0 / sqrt(2);
	const double logpi = log(std::numbers::pi);
	const double log2 = log(2);
	inline double LogGaussianDistribution(double x, double mu, double var)
	{
		double s2 = std::max(var, 1e-6);
		double d = (x - mu);
		return log_pi_norm - 0.5 * log(s2) - 0.5 * d * d / var;
	}

	// inline double lerfc(double x)
	// {
	// 	if (x < 5.0)
	// 	{
	// 		return log(erfc(x));
	// 	}
	// 	double invx2 = 1.0 / (x * x);
	// 	double invx4 = invx2 * invx2;
	// 	double invx6 = invx2 * invx4;
	// 	double invx8 = invx6 * invx2;
	// 	return -x * x - log(x) - 0.5 * logpi + log1p(-0.5 * invx2 + 0.75 * invx4 - 15.0 / 8 * invx6 + 105.6 / 16 * invx8);
	// }
	// inline double log1mexp(double delta)
	// {
	// 	if (delta < log2)
	// 	{
	// 		return log(exp(delta) - 1);
	// 	}
	// }
	// inline double LogGaussianCDD(double lower, double upper, double mu, double var)
	// {
	// 	double sigma = sqrt(var);
	// 	double a = (lower - mu) / sigma;
	// 	double b = (upper - mu) / sigma;
	// 	double za = a * invSqrt2;
	//
	// 	double u = lerfc(-a * invSqrt2);
	// 	double v = lerfc(-b * invSqrt2);
	//
	// 	return v + log1mexp(v - u);
	// }
} // namespace FADE

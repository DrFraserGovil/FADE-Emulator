#pragma once
#include <vector>

namespace FADE
{
	struct TrainCache
	{
		std::vector<double> Mus;
		std::vector<double> Pis;
		std::vector<double> Vrs;
		std::vector<double> Wi;

		std::vector<std::vector<double>> Contribution;
	};
}; // namespace FADE

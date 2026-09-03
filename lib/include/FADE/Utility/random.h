#pragma once
#include <JSL/Log.h>
#include <optional>
#include <random>

namespace FADE
{
	class RandomGenerator
	{
	  public:
		void Initialise(std::optional<int> seed);

		double Uniform();
		double Uniform(double lower, double upper);

		double Normal();
		double Normal(double mean, double deviation);

		bool DiceRoll(double prob);

		std::mt19937 RandomGen;

	  private:
		std::uniform_real_distribution<double> UnifDist;
		std::uniform_real_distribution<double> NormalDist;
	};

	extern RandomGenerator Random;
} // namespace FADE

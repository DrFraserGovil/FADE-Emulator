#include <FADE/Utility/random.h>

namespace FADE
{
	void RandomGenerator::Initialise(std::optional<int> seed)
	{
		if (!seed)
		{
			seed = std::random_device{}();
		}
		LOG(DEBUG) << "Random seed set to " << seed.value();
		RandomGen = std::mt19937(seed.value());
	}
	double RandomGenerator::Uniform(double lower, double upper)
	{
		return lower + (upper - lower) * UnifDist(RandomGen);
	}
	double RandomGenerator::Normal()
	{
		return NormalDist(RandomGen);
	}

	double RandomGenerator::Normal(double mean, double deviation)
	{
		return mean + deviation * NormalDist(RandomGen);
	}
	bool RandomGenerator::DiceRoll(double prob)
	{
		return UnifDist(RandomGen) < prob;
	}

	double RandomGenerator::Uniform()
	{
		return UnifDist(RandomGen);
	}

	RandomGenerator Random;
} // namespace FADE

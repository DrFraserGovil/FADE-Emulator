#include <FADE/Models/Submodel.h>
#include <random>

std::mt19937 gen(std::random_device{}());		   // engine, seeded from a true random source
std::normal_distribution<double> normal(0.0, 1.0); // mean, stddev
std::uniform_real_distribution<double> unif(0, 1);
namespace FADE
{
	void GenerateProposal(const std::vector<double> &current, std::vector<double> &proposal, AnnealingSettings &settings, double stepSize)
	{
		double type = unif(gen);
		double dist = (type > settings.BigLeapProbability) ? stepSize : stepSize * settings.BigLeapFactor;

		for (sint i = 0; i < proposal.size(); ++i)
		{
			proposal[i] = current[i];
			bool getsUpdate = (unif(gen) < settings.UpdateFraction);
			if (getsUpdate)
			{
				proposal[i] += dist * normal(gen);
			}
		}
	}

	template <>
	void Submodel<double>::Train(std::vector<ClusteredTrains> train, std::vector<ClusteredTrains> validate)
	{
		LOG(INFO) << "Beginning training routine for submodel " << Nd << "-" << Ne;

		std::vector<double> Position(Parameters.Size());
		for (sint i = 0; i < Parameters.Size(); ++i)
		{
			Position[i] = unif(gen);
		}
		Parameters.Params = Position;
		Parameters.UpdateDerived();
		double currentE = Score(train);

		double bestE = Score(train);
		auto BestPos = Position;

		auto &anneal = Settings.Train.Annealing;
		sint steps = anneal.Iterations;
		double T = anneal.StartTemp;
		double Tmin = anneal.EndTemp;
		double quenchingRate = pow(Tmin / T, 3.0 / steps);
		int accept = 0;
		int count = 0;
		double stepSize = 0.1;
		double acceptValidate = Score(validate);
		double acceptanceRate = 0;
		int timeSinceBest = 0;
		int reversionCount = 100;

		double decay = 0.2 / steps;
		auto lbound = [&](sint x) { return 0.2 - x * decay; };

		for (sint l = 0; l < steps; ++l)
		{
			GenerateProposal(Position, Parameters.Params, anneal, stepSize);
			Parameters.UpdateDerived();
			double newE = Score(train);
			if (!std::isfinite(newE))
			{
				newE = Settings.Train.LogZero;
			}

			++timeSinceBest;
			bool update = false;
			if (newE > currentE)
			{
				update = true;
				if (newE > bestE)
				{
					LOG(INFO) << JSL::Display::Green() << "New best score " << newE << " (previous best " << bestE << ")";
					bestE = newE;
					BestPos = Position;
					acceptValidate = Score(validate);
					timeSinceBest = 0;
				}
			}
			else if (newE != Settings.Train.LogZero)
			{
				if (l < 0.8 * steps && unif(gen) < anneal.ForceAcceptProbability)
				{
					update = true;
					timeSinceBest -= 100;
				}
				else
				{
					double pAccept = exp(-(currentE - newE) / T);
					if (pAccept > unif(gen))
					{
						update = true;
					}
				}
			}
			++count;
			if (update)
			{
				++accept;
				currentE = newE;
				Position = Parameters.Params;
			}
			else if (timeSinceBest > reversionCount)
			{
				Position = BestPos;
				currentE = bestE;
				timeSinceBest = 0;
				count = 0;
				accept = 0;
				reversionCount += 50;
			}

			if (count > 1000)
			{
				// LOG(DEBUG) << "Cleared memory";
				count = 0;
				accept = 0;
				reversionCount = std::max(100, reversionCount - 20);
			}
			if (count > 30)
			{
				acceptanceRate = accept * 1.0 / count;
				if (acceptanceRate < lbound(l))
				{
					// LOG(WARN) << "Heating at " << acceptanceRate;
					T *= 2;
					// Position = BestPos;
					// currentE = bestE;
				}
				else if (acceptanceRate > 0.4)
				{
					T *= quenchingRate;
				}
			}

			stepSize *= 0.999;

			if (l % 1000 == 0)
			{
				LOG(INFO) << l * 1.0 / steps << " " << T << "  " << bestE << " " << acceptValidate << " " << stepSize << "  " << acceptanceRate;
			}
			// LOG(DEBUG) << "At step " << l << " the temp is " << T;
		}
		Parameters.Params = BestPos;
		Parameters.UpdateDerived();
	}
} // namespace FADE

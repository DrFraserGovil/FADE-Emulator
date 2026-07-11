
#include <FADE/Train/Train.h>
#include <FADE/Train/TrainingPoint.h>
#include <JSL/IO/Vault.h>
#include <JSL/Log.h>
#include <cassert>
typedef size_t sint;
namespace FADE
{
	ClusteredTrains::ClusteredTrains(TrainingPoint &x)
	{
		Position = x.Position;
		Value = {x.Value};
		Weights = {x.Weight};
	}
	double ClusteredTrains::DistanceTo(TrainingPoint &x)
	{
		double s = 0;
		assert(x.Position.size() == Position.size());
		for (sint i = 0; i < Position.size(); ++i)
		{
			s += pow(x.Position[i] - Position[i], 2);
		}
		return sqrt(s);
	}
	void ClusteredTrains::Add(TrainingPoint &x, size_t limit)
	{
		sint n = Value.size();
		for (sint i = 0; i < Position.size(); ++i)
		{
			Position[i] = (n * Position[i] + x.Position[i]) / (n + 1);
		}
		Value.push_back(x.Value);
		Weights.push_back(x.Weight);
		ReachedLimit = (n + 1) >= limit;
	}

	std::pair<std::vector<ClusteredTrains>, std::vector<ClusteredTrains>> ProcessTrainingData(std::vector<TrainingPoint> &data, double fraction, double clusterSize, size_t clusterLimit)
	{
		std::vector<ClusteredTrains> train;
		std::vector<ClusteredTrains> validate;
		sint tcount = 0;
		sint vcount = 0;
		srand(time(NULL));
		for (sint i = 0; i < data.size(); ++i)
		{
			double r = rand() * 1.0 / RAND_MAX;
			std::vector<ClusteredTrains> *group;
			if (r > fraction)
			{
				group = &train;
				++tcount;
			}
			else
			{
				group = &validate;
				++vcount;
			}

			bool found = false;
			for (auto &cluster : *group)
			{
				if (!cluster.ReachedLimit)
				{
					double d = cluster.DistanceTo(data[i]);
					if (d < clusterSize)
					{
						cluster.Add(data[i], clusterLimit);
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				group->emplace_back(data[i]);
			}
		}
		LOG(INFO) << "The data has been clustered into:\n"
				  << "\t" << train.size() << " training clusters containing " << tcount << " datapoints.\n"
				  << "\t" << validate.size() << " validation clusters containing " << vcount << " datapoints.";

		return {train, validate};
	}
} // namespace FADE

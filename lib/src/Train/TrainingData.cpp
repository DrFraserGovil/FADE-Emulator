#include <FADE/Train/TrainingData.h>
#include <FADE/Utility/random.h>
#include <cassert>
#include <cmath>
namespace FADE
{
	TrainingPoint::TrainingPoint(std::vector<double> &vec, const size_t &size)
	{
		Position.resize(size);
		for (size_t i = 0; i < size; ++i)
		{
			Position[i] = vec[i];
		}
		Weight = vec[size];
		Value = vec[size + 1];
	}
	ClusteredData::ClusteredData(TrainingPoint &x)
	{
		Position = x.Position;
		Values = {x.Value};
		LogWeights = {log(x.Weight) + 1e-100};
	}
	double ClusteredData::DistanceTo(TrainingPoint &x)
	{
		double s = 0;
		assert(x.Position.size() == Position.size());
		for (size_t i = 0; i < Position.size(); ++i)
		{
			s += pow(x.Position[i] - Position[i], 2);
		}
		return sqrt(s);
	}

	void ClusteredData::Add(TrainingPoint &x)
	{
		size_t n = Values.size();
		for (size_t i = 0; i < Position.size(); ++i)
		{
			Position[i] = (n * Position[i] + x.Position[i]) / (n + 1);
		}
		Values.push_back(x.Value);
		LogWeights.push_back(log(x.Weight + 1e-100));
	}

	TrainingData::TrainingData(std::vector<TrainingPoint> &data, double fraction, double clusterSize)
	{
		size_t tcount = 0;
		size_t vcount = 0;
		for (size_t i = 0; i < data.size(); ++i)
		{
			if (i == 0)
			{
				bottomLeft = data[i].Position;
				topRight = data[i].Position;
			}
			else
			{
				for (size_t j = 0; j < bottomLeft.size(); ++j)
				{
					double xj = data[i].Position[j];
					if (xj < bottomLeft[j])
					{
						bottomLeft[j] = xj;
					}
					if (xj > topRight[j])
					{
						topRight[j] = xj;
					}
				}
			}
			double r = rand() * 1.0 / RAND_MAX;
			std::vector<ClusteredData> *group;
			if (r > fraction)
			{
				group = &Training;
				++tcount;
			}
			else
			{
				group = &Validation;
				++vcount;
			}

			bool found = false;
			for (auto &cluster : *group)
			{
				double d = cluster.DistanceTo(data[i]);
				if (d < clusterSize)
				{
					cluster.Add(data[i]);
					found = true;
					break;
				}
			}
			if (!found)
			{
				group->emplace_back(data[i]);
			}
		}
		LOG(INFO) << "The data has been clustered into:\n"
				  << "\t" << Training.size() << " training clusters containing " << tcount << " datapoints.\n"
				  << "\t" << Validation.size() << " validation clusters containing " << vcount << " datapoints.";
	}
	std::array<std::vector<double>, 2> TrainingData::GetBounds()
	{
		return {bottomLeft, topRight};
	}

} // namespace FADE

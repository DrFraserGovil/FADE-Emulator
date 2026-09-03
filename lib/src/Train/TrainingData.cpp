#include <FADE/Train/TrainingData.h>
#include <FADE/Utility/random.h>
#include <cassert>
#include <cmath>
namespace FADE
{

	TrainingData::TrainingData(std::vector<ClusteredData> &data, double fraction)
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
			if (Random.DiceRoll(fraction))
			{
				Validation.push_back(data[i]);
				vcount += data[i].Values.size();
			}
			else
			{
				Training.push_back(data[i]);
				tcount += data[i].Values.size();
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

	ClusteredData::ClusteredData(std::vector<double> &vec, const size_t &xDimension)
	{
		Position.resize(xDimension);

		for (size_t i = 0; i < xDimension; ++i)
		{
			Position[i] = vec[i];
		}
		size_t leftOver = vec.size() - xDimension;
		if (leftOver == 0)
		{
			LOG(ERROR) << "The provided training data contains no y values: the vector is equal in size to the input dimension";
			exit(1);
		}
		if (leftOver % 2 != 0)
		{
			LOG(ERROR) << "There are an odd number of remaining data points: this indicates there is not a perfect prior/value split, and this training data is malformed";
			exit(1);
		}
		int yCount = leftOver / 2;
		Values.resize(yCount);
		LogWeights.resize(yCount);
		for (int j = 0; j < yCount; ++j)
		{
			int idx = xDimension + 2 * j;
			if (vec[idx] > 1e-100)
			{
				LogWeights[j] = log(vec[idx]);
				Values[j] = (vec[idx + 1]);
			}
		}
	}
} // namespace FADE

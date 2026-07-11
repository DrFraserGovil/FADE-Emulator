#pragma once
#include <vector>
namespace FADE
{

	struct TrainingPoint
	{
		std::vector<double> Position;
		double Weight;
		double Value;
		TrainingPoint(std::vector<double> &vec, const size_t &size)
		{
			Position.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				Position[i] = vec[i];
			}
			Weight = vec[size];
			Value = vec[size + 1];
		}
	};
} // namespace FADE

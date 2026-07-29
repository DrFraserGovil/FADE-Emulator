#include <FADE/Parameters/ParameterVector.h>
#include <FADE/Utility/random.h>
namespace FADE
{
	ParameterVector::ParameterVector(HyperSettings &hyper, sint departmentCount, sint expertCount) : Hyper(hyper)
	{
		Ne = expertCount;
		Nd = departmentCount;
		DeriveDimensions();
	}

	sint ParameterVector::Size()
	{
		return Params.size();
	}

	void ParameterVector::UpdateDerived()
	{
		ConvertMatrix();
	}
	std::string ParameterVector::ToString()
	{
		std::ostringstream os;
		for (auto &p : Params)
		{
			os << p << "\n";
		}
		return os.str();
	}
	void ParameterVector::Load(std::vector<std::string> &fileData)
	{
		for (sint i = 0; i < TotalSize; ++i)
		{
			Params[i] = JSL::String::ParseTo<double>(fileData[i]);
		}
	}
	void ParameterVector::Randomise(PriorSettings &prior)
	{
		std::vector<double> lower(Hyper.InputDimension, 0);
		std::vector<double> upper(Hyper.InputDimension, 1);
		if (prior.PriorBottomLeft.size() == Hyper.InputDimension)
		{
			lower = prior.PriorBottomLeft;
		}
		if (prior.PriorTopRight.size() == Hyper.InputDimension)
		{
			upper = prior.PriorTopRight;
		}
		for (sint k = 0; k < Nd; ++k)
		{
			for (sint idx = 0; idx < Hyper.InputDimension; ++idx)
			{
				DepPosition(k, idx) = Random.Uniform(lower[idx], upper[idx]);
			}
			for (sint idx = 0; idx < MatrixSize; ++idx)
			{
				Phi(k, idx) = Random.Uniform(0, 1);
			}
		}
		for (sint i = 0; i < Ne; ++i)
		{
			for (sint idx = 0; idx < Hyper.InputDimension; ++idx)
			{
				ExpertPosition(i, idx) = Random.Uniform(lower[idx], upper[idx]);
			}
			ExpertScale(i) = Random.Uniform(0, 0.1);
			;
			double s = 0;
			for (sint p = 0; p < Hyper.ModeCount; ++p)
			{
				ExpertMu(i, p) = Random.Uniform();
				ExpertPi(i, p) = Random.Uniform();
				s += ExpertPi(i, p);
				ExpertV(i, p) = Random.Uniform();
			}
			for (sint p = 0; p < Hyper.ModeCount; ++p)
			{
				ExpertPi(i, p) /= s;
			}
		}
		// for (sint i = 0; i < TotalSize; ++i)
		// {
		// 	Params[i] = Random.Uniform();
		// }
	}

	void ParameterVector::DeriveDimensions()
	{
		MatrixSize = Hyper.InputDimension * (Hyper.InputDimension + 1) / 2;
		TotalSize = (MatrixSize + Hyper.InputDimension) * Nd + (Hyper.InputDimension + Hyper.ModeCount * Hyper.ProbabilityDimension + 1) * Ne;
		LooseParam = 0;
		PhiStart = LooseParam + Hyper.InputDimension * Nd;
		ExpertStart = PhiStart + MatrixSize * Nd;
		ScaleStart = ExpertStart + Ne * Hyper.InputDimension;
		DistStart = ScaleStart + Ne;

		Params.resize(TotalSize, 0);
		Lks.resize(MatrixSize * Nd, 0);
	}
	void ParameterVector::ConvertMatrix()
	{
		for (sint k = 0; k < Nd; ++k)
		{
			for (sint i = 0; i < Hyper.InputDimension; ++i)
			{
				for (sint j = 0; j < i; ++j)
				{
					L(k, i, j) = Phi(k, i, j);
				}
				L(k, i, i) = exp(Phi(k, i, i));
			}
		}
	}

} // namespace FADE

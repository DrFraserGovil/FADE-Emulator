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
	void ParameterVector::Randomise()
	{
		for (sint i = 0; i < TotalSize; ++i)
		{
			Params[i] = Random.Uniform();
		}
	}

	void ParameterVector::DeriveDimensions()
	{
		MatrixSize = Hyper.InputDimension * (Hyper.InputDimension + 1) / 2;

		TotalSize = (MatrixSize + Hyper.InputDimension) * Nd + (Hyper.InputDimension + Hyper.ModeCount * Hyper.ProbabilityDimension) * Ne;
		Params.resize(TotalSize, 0);
		PhiStart = LooseParam + Hyper.InputDimension * Nd;
		ExpertStart = PhiStart + MatrixSize * Nd;
		DistStart = ExpertStart + Ne * Hyper.InputDimension;
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

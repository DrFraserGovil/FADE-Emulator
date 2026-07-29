#include "../Settings.h"
#include "../modes.h"
#include "FADE/Models/Model.h"
#include <JSL.h>
void TestModel()
{
	LOG(INFO) << "Executing model test suite";
	Settings.Model.Hyper.InputDimension = 1;
	FADE::Model test(Settings.Model);

	auto x = JSL::Vector::range(-0., 1.1, 1000);

	for (auto &[id, model] : test.Models)
	{
		std::ostringstream os;
		os << "test/weights_" << id.first << "_" << id.second << ".dat";
		JSL::IO::mkdir("test", JSL::IO::Policy::Generous);
		auto file = JSL::IO::openStream(os.str());

		for (sint i = 0; i < id.second; ++i)
		{
			file << model.Parameters.ExpertPosition(i, 0) << " ";
		}
		file << "\n";

		std::vector<std::vector<double>> Weights(id.second, std::vector<double>(x.size(), 0.0));
		for (sint i = 0; i < x.size(); ++i)
		{
			file << x[i];
			auto W = model.QueryExperts({x[i]});
			for (auto &w : W)
			{
				file << " " << w;
			}
			file << "\n";
		}
		file.close();
	}

	system("cd test; python testplotter.py");
}

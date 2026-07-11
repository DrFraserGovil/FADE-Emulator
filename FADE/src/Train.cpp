// #include <FADE/Models/FADE.h>
// #include <string>
// template <class T>
// void SaveModel(FADE<T> &model)
// {
// 	JSL::IO::VaultWriter Output(Settings.Training.OutputFiles, JSL::IO::Policy::Generous);
//
// 	// save the current settings to file
// 	Output.NewFile("train.config");
// 	Output << Settings.ExportAsString();
//
// 	model.Save(Output);
//
// 	Output.Close();
// }

//

void Train()
{
	// //
	// FADE model;
	//
	// auto [data, validate] = ExtractData();
	//
	// std::vector<double> maxPos = data[0].Position;
	// std::vector<double> minPos = data[1].Position;
	// double minY = data[0].Value;
	// double maxY = data[0].Value;
	// for (auto &d : data)
	// {
	// 	if (d.Position > maxPos) { maxPos = d.Position; }
	// 	if (d.Position < minPos) { minPos = d.Position; }
	// 	if (d.Value > maxY) { maxY = d.Value; }
	// 	if (d.Value < minY) { minY = d.Value; }
	// }
	// LOG(INFO) << "Loaded data is bounded by\n"
	// 		  << "\tPositions: " << minPos << " -> " << maxPos << "\n"
	// 		  << "\tValues:    " << minY << " -> " << maxY;
	// if (Settings.Training.Priors.PriorBottomLeft.empty())
	// {
	// 	Settings.Training.Priors.PriorBottomLeft = minPos;
	// }
	// if (Settings.Training.Priors.PriorTopRight.empty())
	// {
	// 	Settings.Training.Priors.PriorTopRight = maxPos;
	// }
	//
	// model.forModelInModels([&](FixedFADE<double> &model) {
	// 	for (sint r = 0; r < model.Parameters.Size(); ++r)
	// 	{
	// 		double q = rand() * 1.0 / RAND_MAX;
	// 		model.Parameters.Params[r] = q;
	// 	}
	// });
	//
	// SaveModel(model);
}

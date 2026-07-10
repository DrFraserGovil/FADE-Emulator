#include "Train.h"
#include "FADE.h"
#include "Settings.h"
#include <JSL/IO/Vault.h>

using namespace JSL::String;
template <class T>
void SaveModel(FADE<T> &model)
{
	JSL::IO::VaultWriter Output(Settings.Training.OutputFiles, JSL::IO::Policy::Generous);

	// save the current settings to file
	Output.NewFile("train.config");
	Output << Settings.ExportAsString();

	model.Save(Output);

	Output.Close();
}
void Train()
{
	LOG(INFO) << JSL::Display::Colour(30, 100, 30, true) << JSL::Display::White() << "Selected: Training Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	LOG(WARN) << "No training routines are currently implemented. The current placeholder randomly generates a FADE instance.";

	FADE model;

	sint i = 0;
	model.forModelInModels([&](FixedFADE<> &model) {
		model.Parameters.ExpertParameter(0, 0) = model.Parameters.Size();
		++i;
	});

	SaveModel(model);
	JSL::Log::Global().IndentLevel--;
}

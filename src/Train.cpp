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

	// save the current model; we're currently assuming a fixed Ne/Nd pairing, but this will change in future to be a full spectrum, so we reflect that here

	// for (ne ...){ for (nd...)
	model.Save(Output);

	Output.Close();
}
void Train()
{
	LOG(INFO) << JSL::Display::Colour(30, 100, 30, true) << JSL::Display::White() << "Selected: Training Mode" << JSL::Display::ResetAll();
	LOG(WARN) << "No training routines are currently implemented. The current placeholder randomly generates a FADE instance.";

	// HyperParameters hyper(Settings.Hyper.InputDimension, Settings.Hyper.OutputDimension, 2, Settings.Hyper.Departments, Settings.Hyper.Experts);
	//
	// FixedFADE model(hyper);
	FADE model;

	model[std::pair<sint, sint>{2, 4}].SyncParameters();
	SaveModel(model);
}

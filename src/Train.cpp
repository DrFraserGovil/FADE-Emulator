#include "Train.h"
#include "FADE.h"
#include "Settings.h"
#include <JSL.h>
void Train()
{
	LOG(INFO) << JSL::Display::Colour(30, 100, 30, true) << JSL::Display::White() << "Selected: Training Mode" << JSL::Display::ResetAll();
	LOG(WARN) << "No training routines are currently implemented. The current placeholder randomly generates a FADE instance.";

	LOG(WARN) << "FIX: Dimensions of probability model are currently hardcoded to assume a Gaussian; this needs to be fixed in future";

	HyperParameters hyper(Settings.Hyper.InputDimension, Settings.Hyper.OutputDimension, 2, Settings.Hyper.Departments, Settings.Hyper.Experts);

	FADE model(hyper);
}

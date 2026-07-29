#include "../Settings.h"
#include "../modes.h"
#include "FADE/Models/Model.h"
#include <JSL.h>
void TestModel()
{
	LOG(INFO) << "Executing model test suite";
	FADE::Model test(Settings.Model);

	auto x = JSL::Vector::range(-0.5, 1.2, 1000);
}

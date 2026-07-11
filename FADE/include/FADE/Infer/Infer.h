#pragma once

#include "InferPoint.h"
#include <FADE/ModelSettings.h>
namespace FADE
{
	void Infer(FADE::ModelSettings &settings, std::vector<QueryPoint> queries);
}

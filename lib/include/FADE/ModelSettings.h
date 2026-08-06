#pragma once
#include <FADE/Distributions/PriorSettings.h>
#include <FADE/Infer/InferSettings.h>
#include <FADE/Parameters/HyperSettings.h>
#include <FADE/Train/TrainingSettings.h>
#include <JSL/Interface/Aggregator.h>
namespace FADE
{
	class ModelSettings : public JSL::Interface::Aggregator<ModelSettings>
	{
	  public:
		InferenceSettings Infer;
		HyperSettings Hyper;
		TrainingSettings Train;
		PriorSettings Prior;

#include "ModelSettings.ModelSettings.autogen"
	};
} // namespace FADE

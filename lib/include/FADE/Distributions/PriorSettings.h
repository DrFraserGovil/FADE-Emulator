#pragma once
#include <JSL/Interface/Aggregator.h>

namespace FADE
{
	class PriorSettings : public JSL::Interface::Aggregator<PriorSettings>
	{
	  public:
		//! @brief Specifies the BL-boundary of the uniform rectangle in hyper-dimensional space
		//! @details If not specified, the boundary will be inferred from the training data
		//! @alias prior-bl
		std::vector<double> PriorBottomLeft = {};

		//! @brief Specifies the TR-boundary of the uniform rectangle in hyper-dimensional space
		//! @details If not specified, the boundary will be inferred from the training data
		//! @alias prior-tr
		std::vector<double> PriorTopRight = {};

		//! @brief The strength of the prior which pushes variances away from zero in the absence of data
		//! @alias prior-var-alpha
		double priorVarAlpha = 1;

		//! @brief The variance that the prior pulls towards
		//! @alias prior-var-value
		double priorVarValue = 1;

		//! @brief The overall strength of the prior
		//! @alias prior-strength
		double PriorStrength = 1e-7;

		//! @brief The length scale of the BLP fitting
		//! @details The per-dimensional scale length is blpScale * range(dim), where the range is the maximum extend of the input data in that dimension. I.e. a scale of 0.1 means that 10 scale lengths fit into each dimension
		//! @alias blp-scale
		double blpScale = 0.1;

		//! @brief Overrides for the BLP-scale lengths. If set, blpScale is ignored
		//! @alias blp-lengths
		std::optional<std::vector<double>> blpLengths;

		double minMu;
		double maxMu;
#include "PriorSettings.PriorSettings.autogen"
	};
} // namespace FADE

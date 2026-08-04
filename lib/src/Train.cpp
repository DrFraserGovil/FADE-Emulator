#include <FADE/Models/Submodel.h>
#include <JSL/Display/ProgressBar.h>
namespace FADE
{
	// void GenerateProposal(const std::vector<double> &current, std::vector<double> &proposal, AnnealingSettings &settings, double stepSize, std::set<sint> &sigmas)
	// {
	// 	double type = unif(gen);
	// 	double dist = (type > settings.BigLeapProbability) ? stepSize : stepSize * settings.BigLeapFactor;
	//
	// 	for (sint i = 0; i < proposal.size(); ++i)
	// 	{
	// 		proposal[i] = current[i];
	// 		bool getsUpdate = (unif(gen) < settings.UpdateFraction);
	// 		if (getsUpdate)
	// 		{
	// 			proposal[i] += dist * normal(gen);
	// 		}
	// 	}
	//
	// 	for (auto s : sigmas)
	// 	{
	// 		if (unif(gen) < settings.SigmaIncreaseProb)
	// 		{
	// 			proposal[s] = 1;
	// 		}
	// 	}
	// }
	//
	// template <>
	// void Submodel_Old<double>::Train(std::vector<ClusteredTrains> train, std::vector<ClusteredTrains> validate, std::optional<JSL::Display::Progress::Bar *> PB)
	// {
	// 	if (PB)
	// 	{
	// 		std::ostringstream os;
	// 		os << "Model " << Nd << "-" << Ne;
	// 		os << "; training on " << train.size() << "/" << validate.size();
	// 		PB.value()->SetSuffix(os.str(), 0);
	// 	}
	//
	// 	std::vector<double> Position(Parameters.Size());
	// 	for (sint i = 0; i < Parameters.Size(); ++i)
	// 	{
	// 		Position[i] = 1 + 2 * unif(gen);
	// 	}
	// 	Parameters.Params = Position;
	// 	Parameters.UpdateDerived();
	// 	double currentE = Score(train);
	//
	// 	double bestE = Score(train);
	// 	auto BestPos = Position;
	//
	// 	auto &anneal = Settings.Train.Annealing;
	// 	sint steps = anneal.Iterations;
	// 	double T = anneal.StartTemp;
	// 	double Tmin = anneal.EndTemp;
	// 	double quenchingRate = pow(Tmin / T, 3.0 / steps);
	// 	int accept = 0;
	// 	int count = 0;
	// 	double stepSize = 0.2;
	// 	double acceptanceRate = 0;
	// 	int timeSinceBest = 0;
	// 	int reversionCount = 200;
	//
	// 	double decay = 0.3333 / steps;
	// 	auto lbound = [&](sint x) { return 0.3 - x * decay; };
	//
	// 	std::set<sint> sigmas;
	// 	for (sint e = 0; e < Ne; ++e)
	// 	{
	// 		sigmas.insert(Parameters.ExpertStart + Settings.Hyper.ProbabilityDimension * e + 1);
	// 	}
	// 	for (sint l = 0; l < steps; ++l)
	// 	{
	// 		GenerateProposal(Position, Parameters.Params, anneal, stepSize, sigmas);
	// 		SyncParameters();
	// 		double newE = Score(train);
	// 		if (!std::isfinite(newE))
	// 		{
	// 			newE = Settings.Train.LogZero;
	// 		}
	//
	// 		++timeSinceBest;
	// 		bool update = false;
	// 		if (PB && l % 25 == 0)
	// 		{
	// 			std::ostringstream os;
	// 			os << l << "/" << steps << " -- " << acceptanceRate << " -- " << T << " -- " << stepSize;
	// 			PB.value()->SetSuffix(os.str(), 0);
	// 		}
	// 		if (newE > currentE)
	// 		{
	// 			update = true;
	// 			if (newE > bestE)
	// 			{
	// 				bestE = newE;
	// 				BestPos = Position;
	// 				timeSinceBest = 0;
	// 				if (PB)
	// 				{
	// 					PB.value()->SetSuffix("Best Score: " + std::to_string(bestE), 1);
	// 				}
	// 			}
	// 		}
	// 		else if (newE != Settings.Train.LogZero)
	// 		{
	// 			if (l < 0.8 * steps && unif(gen) < anneal.ForceAcceptProbability)
	// 			{
	// 				update = true;
	// 				timeSinceBest -= 100;
	// 			}
	// 			else
	// 			{
	// 				double pAccept = exp(-(currentE - newE) / T);
	// 				if (pAccept > unif(gen))
	// 				{
	// 					update = true;
	// 				}
	// 			}
	// 		}
	// 		++count;
	// 		if (update)
	// 		{
	// 			++accept;
	// 			currentE = newE;
	// 			Position = Parameters.Params;
	// 		}
	// 		else if (timeSinceBest > reversionCount)
	// 		{
	// 			Position = BestPos;
	// 			currentE = bestE;
	// 			timeSinceBest = 0;
	// 			count = 0;
	// 			accept = 0;
	// 			reversionCount += 50;
	// 		}
	//
	// 		if (count > 1000)
	// 		{
	// 			// LOG(DEBUG) << "Cleared memory";
	// 			count = 0;
	// 			accept = 0;
	// 			reversionCount = std::max(100, reversionCount - 20);
	// 		}
	// 		if (count > 30)
	// 		{
	// 			acceptanceRate = accept * 1.0 / count;
	// 			if (acceptanceRate < lbound(l))
	// 			{
	// 				T *= 1.2;
	// 				// Position = BestPos;
	// 				// currentE = bestE;
	// 			}
	// 			else if (acceptanceRate > 0.3)
	// 			{
	// 				T *= quenchingRate;
	// 			}
	// 		}
	//
	// 		stepSize *= 0.9999;
	// 		if (PB)
	// 		{
	// 			PB.value()->Tick();
	// 		}
	// 	}
	// 	Parameters.Params = BestPos;
	// 	SyncParameters();
	// 	BestScore = Score(train, true);
	//
	// 	////// and then....
	// 	double baseScore = BestScore;
	// 	sint optimSteps = Settings.Train.Annealing.OptimIterations;
	// 	sint N = BestPos.size();
	// 	std::vector<double> grad(BestPos.size());
	// 	std::vector<double> m(BestPos.size());
	// 	std::vector<double> v(BestPos.size());
	// 	std::vector<double> step(BestPos.size());
	// 	double b1 = 0.5;
	// 	double b2 = 0.9;
	// 	double dx = 1e-4;
	// 	double alpha = 1;
	//
	// 	for (sint l = 0; l < optimSteps; ++l)
	// 	{
	// 		double c1 = 1.0 / (1.0 - pow(b1, l + 1));
	// 		double c2 = 1.0 / (1.0 - pow(b2, l + 1));
	// 		for (sint j = 0; j < N; ++j)
	// 		{
	// 			double hold = Parameters.Params[j];
	// 			double ddx = dx * std::max(1.0, abs(hold));
	// 			Parameters.Params[j] = hold + ddx;
	// 			double dScore = Score(train);
	// 			grad[j] = (dScore - baseScore) / ddx;
	// 			Parameters.Params[j] = hold;
	// 		}
	// 		// LOG(DEBUG) << gs << baseScore - Settings.Train.LogZero << " " << alpha;
	// 		for (sint j = 0; j < N; ++j)
	// 		{
	// 			m[j] = b1 * m[j] + (1.0 - b1) * grad[j];
	// 			v[j] = b2 * v[j] + (1.0 - b2) * grad[j] * grad[j];
	// 			step[j] = m[j] * c1 / (sqrt(v[j] * c2 + 1e-10));
	// 			double old = Parameters.Params[j];
	// 			Parameters.Params[j] = old + alpha * step[j];
	// 		}
	// 		SyncParameters();
	// 		baseScore = Score(train);
	// 		int count = 0;
	// 		while (!std::isfinite(baseScore) || baseScore == Settings.Train.LogZero)
	// 		{
	// 			if (count > 230)
	// 			{
	// 				l = optimSteps;
	// 				break;
	// 			}
	// 			Parameters.Params = BestPos;
	// 			for (sint i = 0; i < BestPos.size(); ++i)
	// 			{
	// 				if (unif(gen) < 0.4 - 0.01 * count)
	// 				{
	// 					Parameters.Params[i] += 0.1 * alpha * normal(gen);
	// 				}
	// 			}
	// 			SyncParameters();
	// 			baseScore = Score(train);
	// 			++count;
	// 			alpha *= 0.99;
	// 		}
	// 		if (count == 0) { alpha *= 1.1; }
	// 		if (baseScore > BestScore)
	// 		{
	// 			BestScore = baseScore;
	// 			BestPos = Parameters.Params;
	// 			if (PB)
	// 			{
	// 				PB.value()->SetSuffix("Best Score: " + std::to_string(BestScore));
	// 			}
	// 			// alpha *= 2;
	// 		}
	// 		if (PB)
	// 		{
	// 			PB.value()->Tick();
	// 		}
	// 	}
	//
	// 	Parameters.Params = BestPos;
	// 	SyncParameters();
	// }
	//
	// // template <>
	// // void Submodel<double>::ComputeHessian(std::vector<ClusteredTrains> train)
	// // {
	// // 	auto savedParameters = Parameters.Params;
	// // 	sint N = savedParameters.size();
	// // 	// Now we compute the Hessian
	// // 	SyncParameters();
	// // 	auto Pos1 = savedParameters;
	// // 	auto Pos2 = savedParameters;
	// // 	Eigen::MatrixXd Hessian = Eigen::MatrixXd::Zero(N, N);
	// // 	for (sint i = 0; i < N; ++i)
	// // 	{
	// // 		Hessian(i, i) = 2;
	// // 	}
	// // 	ComputeDecomp(Hessian);
	// // }
	// template <>
	// void Submodel_Old<double>::ComputeHessian(std::vector<ClusteredTrains> train)
	// {
	// 	auto savedParameters = Parameters.Params;
	// 	sint N = savedParameters.size();
	// 	// Now we compute the Hessian
	// 	SyncParameters();
	// 	auto Pos1 = savedParameters;
	// 	auto Pos2 = savedParameters;
	// 	Eigen::MatrixXd Hessian = Eigen::MatrixXd::Zero(N, N);
	// 	std::ostringstream os;
	// 	os << "Computing the Hessian for the model with the following properties:\n";
	// 	for (sint k = 0; k < Nd; ++k)
	// 	{
	// 		os << "\tDep. " << k + 1 << " position: (";
	// 		for (sint q = 0; q < Settings.Hyper.InputDimension; ++q)
	// 		{
	// 			os << Parameters.DepPosition(k, q);
	// 		}
	// 		os << ")\n";
	// 	}
	// 	for (sint k = 0; k < Nd; ++k)
	// 	{
	// 		os << JSL::Display::Yellow();
	// 		os << "\tDep. " << k + 1 << " metric: (";
	// 		for (sint q = 0; q < Settings.Hyper.MatrixSize; ++q)
	// 		{
	// 			os << Parameters.Phi(k, q);
	// 		}
	// 		os << ")\n";
	// 	}
	// 	for (sint e = 0; e < Ne; ++e)
	// 	{
	// 		os << JSL::Display::Green();
	// 		os << "\tExp. " << e + 1 << " position: (";
	// 		for (sint q = 0; q < Settings.Hyper.InputDimension; ++q)
	// 		{
	// 			os << Parameters.ExpertPosition(e, q);
	// 		}
	// 		os << ")\n";
	// 	}
	// 	for (sint e = 0; e < Ne; ++e)
	// 	{
	// 		os << JSL::Display::Purple();
	// 		os << "\tExp. " << e + 1 << " parameters: (";
	// 		for (sint q = 0; q < Settings.Hyper.ProbabilityDimension; ++q)
	// 		{
	// 			if (q > 0) os << ", ";
	// 			os << Parameters.ExpertParameter(e, q);
	// 		}
	// 		os << ")\n";
	// 	}
	// 	LOG(INFO) << os.str();
	//
	// 	// Evaluate f(x) for a given parameter vector
	// 	auto Eval = [&](const std::vector<double> &x) -> double {
	// 		Parameters.Params = x;
	// 		SyncParameters();
	// 		return Score(train, false);
	// 	};
	//
	// 	// Step size: cbrt(machine eps) is the standard balance point between
	// 	// truncation error (~h^2) and floating point round-off (~eps/h^2)
	// 	// for second-derivative central differences.
	// 	const double eps = std::numeric_limits<double>::epsilon();
	// 	const double baseStep = std::cbrt(eps);
	// 	std::vector<double> h(N);
	// 	for (sint i = 0; i < N; ++i)
	// 		h[i] = baseStep * std::max(1.0, std::abs(savedParameters[i]));
	//
	// 	const double f0 = Eval(savedParameters);
	//
	// 	// Single-sided perturbations, cached and reused for both the diagonal
	// 	// entries and every off-diagonal entry that touches index i.
	// 	std::vector<double> fPlus(N), fMinus(N);
	// 	for (sint i = 0; i < N; ++i)
	// 	{
	// 		Pos1[i] += h[i];
	// 		fPlus[i] = Eval(Pos1);
	// 		Pos1[i] = savedParameters[i];
	//
	// 		Pos2[i] -= h[i];
	// 		fMinus[i] = Eval(Pos2);
	// 		Pos2[i] = savedParameters[i];
	// 	}
	//
	// 	// Diagonal: H_ii = [f(x+h_i) - 2f(x) + f(x-h_i)] / h_i^2
	// 	for (sint i = 0; i < N; ++i)
	// 		Hessian(i, i) = (fPlus[i] - 2.0 * f0 + fMinus[i]) / (h[i] * h[i]);
	//
	// 	// Off-diagonal: symmetric 4-point central stencil
	// 	//   H_ij = [f(x+h_i+h_j) - f(x+h_i-h_j) - f(x-h_i+h_j) + f(x-h_i-h_j)] / (4 h_i h_j)
	// 	for (sint i = 0; i < N; ++i)
	// 	{
	// 		for (sint j = i + 1; j < N; ++j)
	// 		{
	// 			Pos1[i] += h[i];
	// 			Pos1[j] += h[j];
	// 			double fpp = Eval(Pos1);
	// 			Pos1[i] = savedParameters[i];
	// 			Pos1[j] = savedParameters[j];
	//
	// 			Pos1[i] += h[i];
	// 			Pos1[j] -= h[j];
	// 			double fpm = Eval(Pos1);
	// 			Pos1[i] = savedParameters[i];
	// 			Pos1[j] = savedParameters[j];
	//
	// 			Pos2[i] -= h[i];
	// 			Pos2[j] += h[j];
	// 			double fmp = Eval(Pos2);
	// 			Pos2[i] = savedParameters[i];
	// 			Pos2[j] = savedParameters[j];
	//
	// 			Pos2[i] -= h[i];
	// 			Pos2[j] -= h[j];
	// 			double fmm = Eval(Pos2);
	// 			Pos2[i] = savedParameters[i];
	// 			Pos2[j] = savedParameters[j];
	//
	// 			double val = (fpp - fpm - fmp + fmm) / (4.0 * h[i] * h[j]);
	// 			// if (abs(val) < 1e-10) val = -0;
	// 			Hessian(i, j) = val;
	// 			Hessian(j, i) = val;
	// 		}
	// 	}
	// 	// Restore original parameter state before continuing
	// 	Parameters.Params = savedParameters;
	// 	SyncParameters();
	// 	Hessian = -1 * Hessian;
	// 	ComputeDecomp(Hessian);
	// }
} // namespace FADE

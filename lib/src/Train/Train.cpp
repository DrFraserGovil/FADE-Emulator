#include <FADE/Models/Submodel.h>
#include <FADE/Utility/ale.h>
#include <FADE/Utility/random.h>
#include <numbers>
namespace FADE
{
	void StochasticWalk(double &newV, double &oldV, double step, double prob)
	{
		if (Random.DiceRoll(prob))
		{
			newV = Random.Normal(oldV, step);
		}
		else
		{
			newV = oldV;
		}
	}

	void GenerateProposal(ParameterVector &current, ParameterVector &proposal, AnnealingSettings &settings, double stepSize, sint Ne, sint Nd)
	{
		double step = (Random.DiceRoll(settings.BigLeapProbability) ? settings.BigLeapFactor : 1) * stepSize;
		double pmod = settings.UpdateFraction;

		double scaleShrinkProb = 1.0 - pow(0.9, 1.0 / Ne);
		for (sint i = 0; i < Ne; ++i)
		{
			StochasticWalk(proposal.ExpertScale(i), current.ExpertScale(i), step, pmod);
			proposal.ExpertScale(i) = std::max(0.01, proposal.ExpertScale(i));

			for (sint idx = 0; idx < current.Hyper.InputDimension; ++idx)
			{
				StochasticWalk(proposal.ExpertPosition(i, idx), current.ExpertPosition(i, idx), step, pmod);
			}

			// for (sint p = 0; p < proposal.Hyper.ModeCount; ++p)
			// {
			//
			// 	if (Random.DiceRoll(settings.SigmaIncreaseProb))
			// 	{
			// 		proposal.ExpertV(i, p) *= 1.1;
			// 	}
			// }

			if (Random.DiceRoll(0.1))
			{
				double maxpi = current.ExpertPi(i, 0);
				sint maxp = 0;
				double vsmall = 1e-10;
				for (sint p = 1; p < proposal.Hyper.ModeCount; ++p)
				{
					auto pi = proposal.ExpertPi(i, p);
					if (pi > maxpi)
					{
						maxpi = pi;
						maxp = p;
					}
				}
				for (sint p = 0; p < proposal.Hyper.ModeCount; ++p)
				{
					proposal.ExpertPi(i, p) = vsmall;
				}
				proposal.ExpertPi(i, maxp) = 1.0 - (proposal.Hyper.ModeCount - 1) * 1e-10;
			}
			if (Random.DiceRoll(scaleShrinkProb))
			{
				proposal.ExpertScale(i) = std::max(0.01, 0.1 * proposal.ExpertScale(i));
			}
		}

		for (sint k = 0; k < Nd; ++k)
		{
			for (sint idx = 0; idx < current.Hyper.InputDimension; ++idx)
			{
				StochasticWalk(proposal.DepPosition(k, idx), current.DepPosition(k, idx), step, pmod);
			}
			for (sint j = 0; j < current.Hyper.MatrixSize; ++j)
			{
				StochasticWalk(proposal.Phi(k, j), current.Phi(k, j), step, pmod);
			}
		}
	}
	void Submodel::Train(TrainingData &data)
	{
		CreateTrainingCache(data);
		if (Random.DiceRoll(0.25))
			EMFit(data, 100, 1e-3);
		auto anneal = Settings.Train.Annealing;
		double bestScore = Score(data);
		auto bestPos = Parameters;

		auto currentPos = Parameters;
		auto currentScore = bestScore;
		LOG(DEBUG) << "Initial score: " << Score(data);

		sint steps = anneal.Iterations;
		double alpha = 0.1;
		double T = anneal.StartTemp;

		double quenchingRate = pow(anneal.EndTemp / T, 3.0 / steps);
		int timeSinceBest = 0;
		int revertsSinceBest = 0;
		std::deque<int> acceptance;
		sint memorySize = 100;

		double acceptanceRate = 0;
		double threshhold = 1e-3;
		for (sint l = 0; l < steps; ++l)
		{
			GenerateProposal(currentPos, Parameters, Settings.Train.Annealing, alpha, Ne, Nd);
			if (Random.DiceRoll(0.25))
			{
				EMFit(data, 100, threshhold);
				threshhold *= 0.999;
			}

			double newScore = Score(data);
			if (!std::isfinite(newScore))
			{
				newScore = Settings.Train.LogZero;
			}

			if (newScore == Settings.Train.LogZero)
			{
				LOG(ERROR) << "Degeneracy\n"
						   << Parameters.Params;
				alpha *= 0.5;
			}

			// work out if we want to accept this new proposal
			bool accept = false;
			bool forceRevert = false;
			++timeSinceBest;
			if (newScore > currentScore)
			{
				accept = true;
				if (newScore > bestScore)
				{
					bestPos.Copy(Parameters);

					bestScore = newScore;

					LOG(INFO) << JSL::Display::Green() << "New best: " << bestScore;
					timeSinceBest = 0;
					revertsSinceBest = 0;
				}
			}
			else if (newScore != Settings.Train.LogZero)
			{
				if (l < 0.5 * steps && Random.DiceRoll(anneal.ForceAcceptProbability))
				{
					accept = true;
				}
				else
				{
					double pAccept = exp(-(currentScore - newScore) / T);
					if (Random.DiceRoll(pAccept))
					{
						accept = true;
					}
				}
			}
			else
			{
				forceRevert = true;
			}
			if (accept)
			{
				currentPos.Copy(Parameters);
				currentScore = newScore;
			}
			else
			{
				if (forceRevert || timeSinceBest > 25)
				{
					++revertsSinceBest;
					LOG(WARN) << "No progress; reversion " << revertsSinceBest;
					currentPos.Copy(bestPos);
					currentScore = bestScore;
					timeSinceBest = 0;
					alpha *= 0.5;
					// acceptance.clear();

					if (revertsSinceBest > 10)
					{
						break;
					}
				}
			}

			/// now do rate checks
			acceptance.push_back((int)accept);
			sint n = acceptance.size();
			acceptanceRate = ((n - 1) * acceptanceRate + (int)accept) * 1.0 / n;
			if (acceptance.size() > memorySize)
			{
				acceptanceRate = (n * acceptanceRate - acceptance.front()) * 1.0 / (n - 1);
				acceptance.pop_front();
				if (acceptanceRate > 0.3)
				{
					T *= quenchingRate;
					alpha *= 1.01;
				}
				if (acceptanceRate < 0.2)
				{
					T *= 1.05;
					alpha *= 0.99;
				}
			}

			if (l % 50 == 0)
			{
				LOG(INFO) << l << " " << acceptanceRate << " " << T << " " << alpha << " " << quenchingRate;
			}
		}

		Parameters.Copy(bestPos);
		EMFit(data, 1000, 0);
		double fin = Score(data);
		LOG(DEBUG) << "Final score: " << fin;
	}

	void Submodel::EMFit(TrainingData &data, sint steps, double earlyStop)
	{
		double score = Score(data);
		for (sint l = 0; l < steps; ++l)
		{
			EPhase(data);
			MPhase_Weights(data);
			MPhase_Mu(data);
			MPhase_Vars(data);

			double newScore = Score(data);
			double diff = (newScore - score) / abs(score);
			if (diff < earlyStop && diff > 0) // diff is mathematically guaraneteed to be positive, but it can't hurt to validate this
			{
				break;
			}
			score = newScore;
		}
	}

	void Submodel::CreateTrainingCache(TrainingData &data)
	{
		auto Nt = data.Training.size();
		auto P = Settings.Hyper.ModeCount;

		bVec = Eigen::VectorXd::Zero(Ne);
		muMatrix = Eigen::MatrixXd::Zero(Ne, Ne);
		Cvec.resize(P, 0);
		Cache.resize(Nt);

		for (sint t = 0; t < Nt; ++t)
		{
			auto N = data.Training[t].Values.size();
			Cache[t].Mus.resize(P);
			Cache[t].Pis.resize(P);
			Cache[t].Vrs.resize(P);
			Cache[t].Wi.resize(Ne);

			Cache[t].Contribution = std::vector<std::vector<double>>(P, std::vector<double>(N, 0));
		}
	}

	void Submodel::EPhase(TrainingData &data)
	{
		const auto P = Settings.Hyper.ModeCount;
		double log2pi = log(2 * std::numbers::pi);
		for (sint t = 0; t < data.Training.size(); ++t)
		{
			SetPosition(data.Training[t].Position);
			for (sint p = 0; p < P; ++p)
			{
				Cache[t].Mus[p] = Mus[p];
				Cache[t].Pis[p] = Pis[p];
				Cache[t].Vrs[p] = Vrs[p];
			}
			for (sint i = 0; i < Ne; ++i)
			{
				Cache[t].Wi[i] = ExpertWeights[i];
			}
			sint Na = data.Training[t].Values.size();
			for (sint a = 0; a < Na; ++a)
			{
				double s = 0;
				for (sint p = 0; p < P; ++p)
				{
					double d = Mus[p] - data.Training[t].Values[a];
					double cont = log(Pis[p]) - 0.5 * log2pi - 0.5 * log(Vrs[p]) - 0.5 * d * d / Vrs[p];
					if (p == 0)
					{
						s = cont;
					}
					else
					{
						s = ale(s, cont);
					}
					Cache[t].Contribution[p][a] = cont;
				}
				// now normalise
				for (sint p = 0; p < P; ++p)
				{
					Cache[t].Contribution[p][a] = exp(Cache[t].Contribution[p][a] - s);
				}
			}
		}
	}

	void Submodel::MPhase_Mu(TrainingData &data)
	{
		const auto P = Settings.Hyper.ModeCount;
		for (sint p = 0; p < P; ++p)
		{
			bVec *= 0;
			muMatrix *= 0;
			for (sint t = 0; t < Cache.size(); ++t)
			{
				auto &Ct = Cache[t];
				for (sint a = 0; a < data.Training[t].Values.size(); ++a)
				{
					auto y = data.Training[t].Values[a];
					for (sint i = 0; i < Ne; ++i)
					{
						double ToV = Ct.Wi[i] * Ct.Contribution[p][a] / Ct.Vrs[p];
						bVec(i) += y * ToV;
						for (sint j = i; j < Ne; ++j)
						{
							muMatrix(i, j) += Ct.Wi[j] * ToV;
						}
					}
				}
			}
			for (sint i = 0; i < Ne; ++i)
			{
				muMatrix(i, i) += 1e-5; // conditioing
				for (sint j = 0; j < i; ++j)
				{
					muMatrix(i, j) = muMatrix(j, i);
				}
			}
			auto solver = muMatrix.llt();
			if (solver.info() != Eigen::Success)
			{
				LOG(WARN) << "bad mu update; skipping";
			}
			else
			{
				auto mu = solver.solve(bVec);
				for (sint i = 0; i < Ne; ++i)
				{
					if (!std::isfinite(mu(i)))
					{
						LOG(ERROR) << "Bad!" << " " << Parameters.ExpertMu(i, p);
						LOG(INFO) << Parameters.Params;
						exit(1);
					}
					else
					{
						Parameters.ExpertMu(i, p) = mu(i);
					}
				}
			}
		}
		// update the cache
		for (sint t = 0; t < Cache.size(); ++t)
		{
			for (sint p = 0; p < Settings.Hyper.ModeCount; ++p)
			{
				Cache[t].Mus[p] = 0;
				for (sint i = 0; i < Ne; ++i)
				{
					Cache[t].Mus[p] += Cache[t].Wi[i] * Parameters.ExpertMu(i, p);
				}
			}
		}
	}

	void Submodel::MPhase_Weights(TrainingData &data)
	{
		const sint P = Settings.Hyper.ModeCount;
		for (sint i = 0; i < Ne; ++i)
		{
			std::fill(Cvec.begin(), Cvec.end(), 0.0);
			double s = 0;
			for (sint p = 0; p < P; ++p)
			{
				for (sint t = 0; t < Cache.size(); ++t)
				{
					auto &Ct = Cache[t];
					double piV = Parameters.ExpertPi(i, p) / (1e-15 + Ct.Pis[p]);
					if (!std::isfinite(piV))
					{
						piV = 0;
					}
					for (sint a = 0; a < data.Training[t].Values.size(); ++a)
					{
						double cont = Ct.Contribution[p][a] * Ct.Wi[i] * piV;
						Cvec[p] += cont;
						s += cont;
					}
				}
			}
			for (sint p = 0; p < P; ++p)
			{
				Parameters.ExpertPi(i, p) = std::max(1e-10, Cvec[p] / s);
			}
		}
		// update the cache
		for (sint t = 0; t < Cache.size(); ++t)
		{
			for (sint p = 0; p < Settings.Hyper.ModeCount; ++p)
			{
				Cache[t].Pis[p] = 0;
				for (sint i = 0; i < Ne; ++i)
				{
					Cache[t].Pis[p] += Cache[t].Wi[i] * Parameters.ExpertPi(i, p);
				}
			}
		}
	}

	void Submodel::MPhase_Vars(TrainingData &data)
	{
		const sint P = Settings.Hyper.ModeCount;
		for (sint p = 0; p < P; ++p)
		{
			for (sint i = 0; i < Ne; ++i)
			{
				for (sint it = 0; it < 5; ++it)
				{
					double grad = 0;
					double curve = 0;
					sint Nt = 0;
					for (sint t = 0; t < Cache.size(); ++t)
					{
						sint Na = data.Training[t].Values.size();
						Nt += Na;
						auto &Ct = Cache[t];
						auto &vp = Ct.Vrs[p];
						double invV = 1.0 / vp;
						double invVSq = invV * invV;
						for (sint a = 0; a < Na; ++a)
						{
							double d = data.Training[t].Values[a] - Ct.Mus[p];
							double pref = Ct.Contribution[p][a] * Ct.Wi[i];
							double b1 = (0.5 * d * d * invVSq - 0.5 * invV);
							double b2 = -0.5 * invVSq;
							// double b2 = 0.5 * invVSq - d * d * invVCu;

							grad += pref * b1;
							curve += pref * b2 * Ct.Wi[i];
						}
					}
					double vip = Parameters.ExpertV(i, p);
					double vinv = 1.0 / vip;
					double priorGrad = (Settings.Prior.priorVarAlpha + 1) * vinv * (-1.0 + Settings.Prior.priorVarValue * vinv);
					double priorCurve = (Settings.Prior.priorVarAlpha + 1) * vinv * vinv * (1.0 - 2 * Settings.Prior.priorVarValue * vinv);

					// LOG(INFO) << grad << " " << curve << " " << priorGrad << " " << priorCurve << " / vip = " << vip;
					grad = grad / Nt + Settings.Prior.PriorStrength * priorGrad;
					curve = curve / Nt + Settings.Prior.PriorStrength * priorCurve;

					Parameters.ExpertV(i, p) = std::min(1e3, std::max(1e-6, Parameters.ExpertV(i, p) - grad / curve));

					// update the cache much more often for sigma
					for (sint t = 0; t < Cache.size(); ++t)
					{
						Cache[t].Vrs[p] = 0;
						for (sint i = 0; i < Ne; ++i)
						{
							Cache[t].Vrs[p] += Cache[t].Wi[i] * Parameters.ExpertV(i, p);
						}
					}
				}
			}
		}
	}
} // namespace FADE

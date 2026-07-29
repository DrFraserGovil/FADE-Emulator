#include <FADE/Models/Submodel.h>
#include <FADE/Utility/ale.h>
namespace FADE
{

	void Submodel::Train(TrainingData &data)
	{
		CreateTrainingCache(data);

		double score = Score(data);

		double earlyStop = 1e-5;
		for (int l = 0; l < 250; ++l)
		{
			EMFit(data);
			double newScore = Score(data);
			double diff = (newScore - score) / abs(score);
			if (diff < earlyStop && diff > 0) // diff is mathematically guaraneteed to be positive, but it can't hurt to validate this
			{
				break;
			}
			score = newScore;
		}
	}

	void Submodel::EMFit(TrainingData &data)
	{
		EPhase(data);
		MPhase_Mu(data);
		MPhase_Weights(data);
		MPhase_Vars(data);
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
		double log2pi = log(2 * M_PI);
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
				muMatrix(i, i) += 1e-8; // conditioing
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
					Parameters.ExpertMu(i, p) = mu(i);
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
					double piV = Parameters.ExpertPi(i, p) / Ct.Pis[p];
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
				Parameters.ExpertPi(i, p) = Cvec[p] / s;
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
				double vip = Parameters.ExpertV(i, p);
				for (sint it = 0; it < 5; ++it)
				{
					double grad = 0;
					double curve = 0;

					for (sint t = 0; t < Cache.size(); ++t)
					{
						auto &Ct = Cache[t];
						auto &vp = Ct.Vrs[p];
						double invV = 1.0 / vp;
						double invVSq = invV * invV;
						double invVCu = invVSq * invV;
						for (sint a = 0; a < data.Training[t].Values.size(); ++a)
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

					Parameters.ExpertV(i, p) = std::max(1e-6, Parameters.ExpertV(i, p) - grad / curve);

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

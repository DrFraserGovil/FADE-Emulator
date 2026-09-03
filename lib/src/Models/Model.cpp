#include <FADE/Models/Model.h>
#include <JSL/Vectors/Range.h>
namespace FADE
{

	Model::Model(ModelSettings &settings) : Settings(settings)
	{
		if (Settings.Infer.ModelFile)
		{
			Load(Settings.Infer.ModelFile.value());
		}
		else
		{
			ConstructModels();
		}
	}
	void Model::Train([[maybe_unused]] TrainingData &data, [[maybe_unused]] sint extraThreads)
	{
		auto [bottomLeft, topRight] = data.GetBounds();
		if (Settings.Prior.PriorBottomLeft.size() != bottomLeft.size())
		{
			Settings.Prior.PriorBottomLeft = bottomLeft;
			LOG(INFO) << "The BL-bound has been infered to be " << bottomLeft;
		}
		if (Settings.Prior.PriorTopRight.size() != topRight.size())
		{
			Settings.Prior.PriorTopRight = topRight;
			LOG(INFO) << "The TR-bound has been infered to be " << topRight;
		}

		BLPFit(data);
		// i.e. if we didn't just load in a pre-existing  model
		if (!Settings.Infer.ModelFile)
		{
			forAllModels([&](auto &model) { model.Parameters.Randomise(Settings.Prior); });
		}

		forAllModels([&](auto &model) { model.Train(data); });

		Save(data);
	}
	void Model::Train(std::vector<ClusteredData> &data, sint extraThreads)
	{

		TrainingData group(data, Settings.Train.ValidationFraction);
		Train(group, extraThreads);
	}

	Submodel &Model::operator[](std::pair<sint, sint> idx)
	{
		assert(Models.contains(idx));
		return Models.at(idx);
	}

	void Model::SetPosition(std::vector<double> pos)
	{
		forAllModels([&pos](auto &model) { model.SetPosition(pos); });
	}

	void Model::Load(const std::filesystem::path &vaultPath)
	{
		LOG(INFO) << "Loading settings from file " << vaultPath;
		auto vault = JSL::IO::VaultReader(vaultPath.string());
		if (!vault.Files().contains("train.config"))
		{
			LOG(ERROR) << "Model file does not have a vaild configuration module. \nThe model file is most likley corrupted";
			exit(1);
		}

		auto oldInfer = Settings.Infer;
		auto lines = vault["train.config"].AsLines();
		Settings.Configure(lines, " ");
		Settings.Infer = oldInfer; // inference settings are *not* overwritten from file
		ConstructModels();
		BLPLoad(vault);
		forAllModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
	}

	ModelSettings Model::GetSettings() { return Settings; }

	void Model::Predict(std::set<QueryPoint> &queries)
	{
		LOG(INFO) << "Beginning inference loop";
		auto tmp = JSL::Log::Indent();

		std::vector<double> probVec;
		std::vector<double> cdfVec;
		for (auto &query : queries)
		{
			sint N = query.PredictionGrid.size();
			bool requiresBounding = false;
			if (N == 0)
			{
				requiresBounding = true;
				N = Settings.Infer.Resolution;
			}
			probVec.resize(N);
			cdfVec.resize(N);
			// query.PredictionValues.resize(N);
			LOG(INFO) << "Inferring at position" << query.EmulationPoint;
			std::vector<double> p = query.EmulationPoint;

			Eigen::VectorXd pos = Eigen::Map<Eigen::VectorXd>(p.data(), p.size());
			Eigen::VectorXd k = Eigen::VectorXd::Zero(KinvY.rows());
			for (sint i = 0; i < BLPPos.size(); ++i)
			{
				auto dsq = (pos - BLPPos[i]).squaredNorm();
				double l = Settings.Prior.blpScale;
				k(i) = exp(-0.5 * dsq / (l * l));
			}
			double prediction = k.dot(KinvY);
			LOG(DEBUG) << "The BLP predicts a mean of " << prediction << " at " << p;
			// forAllModels([&](auto &model) {
			for (auto &[id, model] : Models)
			{
				model.SetPosition(query.EmulationPoint);
				if (requiresBounding)
				{
					LOG(INFO) << JSL::Display::Green() << "Auto-bounding was requested.";
					double thresh = Settings.Infer.CDFBound;
					auto [l, u] = model.GetBounds(thresh);
					l += prediction;
					u += prediction;
					LOG(INFO) << "\tThe [" << thresh << ", " << 1.0 - thresh << "] boundary is [" << l << ", " << u << "]";
					query.PredictionGrid = JSL::Vector::range(l, u, N);
				}
				for (sint j = 0; j < N; ++j)
				{
					double x = query.PredictionGrid[j] - prediction;
					probVec[j] = exp(model.LogGaussian(x));
					cdfVec[j] = model.CDF(x);
				}

				//! HACK: This is just whilst we're on single-only models
				query.SubmodelProbability[id] = std::move(probVec);
				query.SubmodelCDF[id] = std::move(cdfVec);
			};
		}
	}

	void Model::BLPFit(TrainingData &data)
	{
		sint Nval = data.Validation.size();
		Eigen::MatrixXd K = Eigen::MatrixXd::Zero(Nval, Nval);
		Eigen::VectorXd Yvec = Eigen::VectorXd::Zero(Nval);

		double lscle = Settings.Prior.blpScale;

		for (sint tx = 0; tx < Nval; ++tx)
		{
			// Eigen::VectorXd px(data.Training[tx].Position);
			auto &posx = data.Validation[tx].Position;
			Eigen::Map<Eigen::VectorXd> px(posx.data(), posx.size());
			for (sint ty = 0; ty < Nval; ++ty)
			{
				auto &posy = data.Validation[ty].Position;
				Eigen::Map<Eigen::VectorXd> py(posy.data(), posy.size());

				auto diff = (px - py);
				double dsq = diff.squaredNorm();
				K(tx, ty) = exp(-0.5 * dsq / (lscle * lscle));
				K(ty, tx) = K(tx, ty);
			}

			sint Nrepl = data.Validation[tx].Values.size();
			double ysqSum = 0;
			for (sint a = 0; a < Nrepl; ++a)
			{
				double y = data.Validation[tx].Values[a];
				Yvec(tx) += y;
				ysqSum += y * y;
			}
			Yvec(tx) = Yvec(tx) / Nrepl;
			double variance = ysqSum / Nrepl - Yvec(tx) * Yvec(tx);
			variance = std::max(variance / Nrepl, 1e-3);
			K(tx, tx) += variance;
		}
		auto solve = K.llt();
		if (solve.info() != Eigen::Success)
		{
			LOG(WARN) << "Could not perform BLP-fitting; using raw data";
			KinvY = Eigen::VectorXd::Zero(Nval);
		}
		else
		{
			KinvY = solve.solve(Yvec);
		}

		/// DATA CORRECTION
		sint Ntrain = data.Training.size();
		bool first = true;
		double minDev = 0;
		double maxDev = 0;
		Eigen::VectorXd k = Eigen::VectorXd::Zero(Nval);
		for (sint i = 0; i < Ntrain; ++i)
		{
			auto &posx = data.Training[i].Position;
			Eigen::Map<Eigen::VectorXd> px(posx.data(), posx.size());
			for (sint j = 0; j < Nval; ++j)
			{
				auto &posj = data.Validation[j].Position;
				Eigen::Map<Eigen::VectorXd> py(posj.data(), posj.size());
				double dsq = (px - py).squaredNorm();
				k(j) = exp(-0.5 * dsq / (lscle * lscle));
			}
			double prediction = k.dot(KinvY);

			for (sint a = 0; a < data.Training[i].Values.size(); ++a)
			{
				data.Training[i].Values[a] -= prediction;
				double corr = data.Training[i].Values[a];
				if (first || corr < minDev)
				{
					minDev = corr;
				}
				if (first || corr > maxDev)
				{
					maxDev = corr;
				}
				first = false;
			}
		}
		LOG(INFO) << "Residual-Correction completed";
		data.minExpectedMu = minDev;
		data.maxExpectedMu = maxDev;
		Settings.Prior.minMu = minDev;
		Settings.Prior.maxMu = maxDev;
		LOG(INFO) << "The data has residual range [" << minDev << ", " << maxDev << "]";
	}
	void Model::ConstructModels()
	{

		for (sint nd = Settings.Hyper.Departments.first; nd <= Settings.Hyper.Departments.second; ++nd)
		{
			for (sint ne = Settings.Hyper.Experts.first; ne <= Settings.Hyper.Experts.second; ++ne)
			{
				Models.try_emplace({nd, ne}, Settings, nd, ne);
			}
		}
	}
	void Model::Save(TrainingData &data)
	{
		auto vault = JSL::IO::VaultWriter(Settings.Train.OutputFiles, JSL::IO::Policy::Generous);
		vault["train.config"] << Settings.ExportAsString();
		forAllModels([&vault](auto &model) { model.Save(vault); });

		BLPSave(data, vault);
	}

	void Model::BLPSave(TrainingData &data, JSL::IO::VaultWriter &vault)
	{
		auto &file = vault.NewFile("blp.data", true);

		for (sint t = 0; t < data.Validation.size(); ++t)
		{
			file << std::setprecision(17) << KinvY(t);
			for (sint j = 0; j < Settings.Hyper.InputDimension; ++j)
			{
				file << " " << data.Validation[t].Position[j];
			}
			file << "\n";
		}
	}
	void Model::BLPLoad(JSL::IO::VaultReader &vault)
	{
		std::vector<double> kinvy;
		vault.ForLineIn("blp.data", [&](auto line) {
			auto sp = JSL::String::split_view(line, " ");
			kinvy.push_back(JSL::String::ParseTo<double>(sp[0]));
			std::vector<double> pos;
			for (sint i = 1; i < sp.size(); ++i)
			{
				pos.push_back(JSL::String::ParseTo<double>(sp[i]));
			}
			Eigen::VectorXd p = Eigen::Map<Eigen::VectorXd>(pos.data(), pos.size());
			BLPPos.push_back(p);
		});

		KinvY = Eigen::Map<Eigen::VectorXd>(kinvy.data(), kinvy.size());
	}

} // namespace FADE

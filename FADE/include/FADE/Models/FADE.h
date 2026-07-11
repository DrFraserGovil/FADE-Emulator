#pragma once
#include "Submodel.h"
namespace FADE
{

	template <class T = double>
	class Model
	{
	  public:
		Model(HyperSettings &hyper) : Hyper()
		{
			for (sint nd = hyper.Departments.first; nd <= hyper.Departments.second; ++nd)
			{
				for (sint ne = hyper.Experts.first; ne <= hyper.Departments.second; ++ne)
				{
					Models.try_emplace({nd, ne}, Hyper, nd, ne);
				}
			}
		}
		Model(ModelSettings &mset) : Model(mset.Hyper) {};
		FixedFADE<T> &operator[](std::pair<sint, sint> idx)
		{
			// assert(Models
			assert(Models.contains(idx));
			return Models.at(idx);
		}
		void Save(JSL::IO::VaultWriter &vault)
		{
			std::string hyperfile = "hyper.param";
			vault[hyperfile] << Hyper.ToString();

			forModelInModels([&vault](auto &model) { model.Save(vault); });
		}

		template <class U>
		void forModelInModels(U callback)
		{
			for (auto &[_, model] : Models)
			{
				callback(model);
			}
		}

		void SetPosition(std::vector<double> pos)
		{
			forModelInModels([&pos](auto &model) { model.SetPosition(pos); });
		}

		// we assume that the settings portion has already been read in and modified; we are just populating the submodels at this point
		void Load(JSL::IO::VaultReader &vault, HyperSettings &newhyper)
		{
			Hyper.Reset(newhyper);
			forModelInModels([&vault](auto &model) { model.Load(vault); model.SyncParameters(); });
		}

		void Load(std::filesystem::path modelfile, ModelSettings &settings)
		{
		}

	  private:
		std::map<std::pair<sint, sint>, FixedFADE<T>> Models;

		HyperParameters Hyper;
	};
} // namespace FADE

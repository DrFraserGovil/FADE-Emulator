#include <FADE/Other/Unpack.h>
#include <JSL/IO.h>
#include <JSL/Log.h>
namespace fs = std::filesystem;

fs::path deExtension(fs::path in)
{
	while (!in.extension().empty())
	{
		in.replace_extension("");
	}
	return in;
}

void performUnpacking(fs::path file)
{
	LOG(INFO) << "Unpacking " << file.string();
	try
	{
		JSL::IO::VaultReader vault(file.string(), JSL::IO::Policy::Generous);

		LOG(DEBUG) << "Found the following files in the vault:\n"
				   << vault.Files();

		auto outfile = deExtension(file);
		LOG(INFO) << "Output will be written to " << outfile;
		JSL::IO::mkdir(outfile, JSL::IO::Policy::Generous);
		for (auto &vf : vault.Files())
		{
			auto tmp2 = JSL::Log::Indent();
			LOG(INFO) << "Extracting " << vf;
			if (vf.back() != fs::path::preferred_separator)
			{
				auto parent = fs::path(vf).parent_path().string();
				if (!parent.empty())
				{
					LOG(INFO) << "\t= Writing " << (outfile / parent).string();
					JSL::IO::mkdir(outfile / parent, JSL::IO::Policy::Generous);
				}
				JSL::IO::writeString(outfile / vf, vault.AsText(vf));
			}
		}
	}
	catch (std::exception &e)
	{
		LOG(WARN) << "Error in unpacking; message is:\n"
				  << e.what() << "\n"
				  << "The most likely cause of this error is that " << file << " is not a valid Vault";
	}
}

namespace FADE
{
	void Unpack(std::set<std::filesystem::path> targets)
	{
		if (targets.empty())
		{
			LOG(ERROR) << "No files to unpack";
			exit(1);
		}

		for (auto file : targets)
		{
			if (fs::exists(file))
			{
				performUnpacking(file);
			}
			else
			{
				LOG(WARN) << "Cannot find " << file.string() << " to unpack it";
			}
		}
	}
} // namespace FADE

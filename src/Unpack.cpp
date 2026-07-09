#include "Unpack.h"
#include "Settings.h"
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
	LOG(INFO) << " - Unpacking " << file.string();
	try
	{
		JSL::IO::VaultReader vault(file.string(), JSL::IO::Policy::Generous);

		LOG(DEBUG) << "Found the following files in the vault:\n"
				   << vault.Files();

		auto outfile = deExtension(file);
		LOG(DEBUG) << "Output will be written to " << outfile;
		JSL::IO::mkdir(outfile, JSL::IO::Policy::Generous);
		for (auto &vf : vault.Files())
		{
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

void Unpack()
{
	if (Settings.Files.empty())
	{
		LOG(ERROR) << "No files to unpack\nUse either positional arguments or input to --input to assign targets";
		exit(1);
	}

	LOG(INFO) << JSL::Display::Colour(30, 30, 100, true) << JSL::Display::White() << "Selected: Unpacking Mode" << JSL::Display::ResetAll();

	for (auto file : Settings.Files)
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

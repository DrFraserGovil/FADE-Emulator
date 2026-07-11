#include "unpack.h"
#include <FADE/Other/Unpack.h>
#include <JSL.h>
void Unpack(std::set<std::filesystem::path> paths)
{
	LOG(INFO) << JSL::Display::Colour(30, 30, 100, true) << JSL::Display::White() << "Selected: Unpacking Mode" << JSL::Display::ResetAll();
	auto tmp = JSL::Log::Indent(); // increases indent level until Infer is over
	FADE::Unpack(paths);
}

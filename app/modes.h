
#pragma once

#include <filesystem>
#include <set>
void Unpack(std::set<std::filesystem::path> paths);

void TrainModel(std::set<std::filesystem::path> paths);

void Predict(std::set<std::filesystem::path> paths);

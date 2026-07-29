
#pragma once

#include <filesystem>
#include <set>
void UnpackData(std::set<std::filesystem::path> paths);

void TrainModel(std::set<std::filesystem::path> paths);

void Predict(std::set<std::filesystem::path> paths);

void TestModel();

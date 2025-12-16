
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <cstdio>
#include <nlohmann/json.hpp>
#include "ir.hpp"
using json = nlohmann::json;

std::vector<GateDef> loadGates(const std::string& filename);
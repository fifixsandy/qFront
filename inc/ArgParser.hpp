/**
 * @file ArgParser.hpp
 * @author Filip Novak
 * @date 2026-01-29
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class ArgParser {
public:
    struct Args {
        std::string target = "stim";
        std::string input_file = "";
        std::string output_file = "";
    };

    static Args parse(int argc, const char* argv[]);

    static void printUsage(const char* program_name);

private:
    ArgParser() = default;
};

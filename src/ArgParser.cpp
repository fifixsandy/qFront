/**
 * @file ArgParser.cpp
 * @author Filip Novak
 * @date 2026-01-29
 */
#include "../inc/ArgParser.hpp"
#include <iostream>
#include <algorithm>

void ArgParser::printUsage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  -t, --target <target>    Output target (default: stim)\n";
    std::cerr << "                           Available: stim, autoq-para\n";
    std::cerr << "  -f, --file <input.qasm>  Input OpenQASM file (default: stdin)\n";
    std::cerr << "  -o, --output <output>    Output file (default: stdout)\n";
    std::cerr << "  -h, --help               Show this help message\n\n";
    std::cerr << "Examples:\n";
    std::cerr << "  " << program_name << " -t stim -f circuit.qasm -o circuit.stim\n";
    std::cerr << "  " << program_name << " -f circuit.qasm < input.qasm\n";
    std::cerr << "  " << program_name << " < input.qasm > output.stim\n";
}

ArgParser::Args ArgParser::parse(int argc, const char* argv[]) {
    Args args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        }
        else if (arg == "-t" || arg == "--target") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: -t/--target requires an argument");
            }
            args.target = argv[++i];
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: -f/--file requires an argument");
            }
            args.input_file = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                throw std::runtime_error("Error: -o/--output requires an argument");
            }
            args.output_file = argv[++i];
        }
        else {
            throw std::runtime_error("Unknown option: " + arg);
        }
    }

    if (args.target != "stim" && args.target != "autoq-para") {
        throw std::runtime_error("Unknown target: " + args.target + 
                                 " (valid: stim, autoq-para)");
    }

    return args;
}


/* EOF ArgParser.cpp*/
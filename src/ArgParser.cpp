/**
 * @file ArgParser.cpp
 * @author Filip Novak
 * @date 2026-03-02
 */
#include "../inc/ArgParser.hpp"
#include <iostream>
#include <algorithm>

void ArgParser::printUsage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  -t, --target <target>        Output target (default: stim)\n";
    std::cerr << "                               Available: stim, autoq-para\n";
    std::cerr << "  -f, --file <input.qasm>      Input OpenQASM file (default: stdin)\n";
    std::cerr << "  -o, --output <output>        Output file (default: stdout)\n";
    std::cerr << "  -a, --algebraic <precision>  Enable algebraic matrices (default: off, 32)\n";
    std::cerr << "  -h, --help                   Show this help message\n";
    std::cerr << "  --decompose-mcx              Decompose mcx gates into x, cx, and ccx gates (ancilla qubits added as needed)\n";
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
                throw std::invalid_argument("Error: -t/--target requires an argument");
            }
            args.target = argv[++i];
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Error: -f/--file requires an argument");
            }
            args.input_file = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Error: -o/--output requires an argument");
            }
            args.output_file = argv[++i];
        }
        else if (arg == "-a" || arg == "--algebraic") {
            args.use_algebraic = true;
    
            if (i + 1 < argc) {
                try {
                    args.algebraic_precision = std::stoul(argv[++i]);
                    if (args.algebraic_precision == 0) {
                        throw std::invalid_argument("Precision must be >= 1");
                    }
                } catch (const std::exception&) {
                    --i;
                }
            }
        } else if (arg == "--decompose-mcx") {
            args.decompose_mcx = true;
        }
        else {
            throw std::invalid_argument("Unknown option: " + arg);
        }
    }

    if (args.target != "stim" && 
        args.target != "autoq-para" && 
        args.target != "openqasm3" && 
        args.target != "openqasm2") {
        throw std::invalid_argument("Unknown target: " + args.target + 
                                 " (valid: stim, autoq-para, openqasm3, openqasm2)");
    }

    return args;
}


/* EOF ArgParser.cpp*/
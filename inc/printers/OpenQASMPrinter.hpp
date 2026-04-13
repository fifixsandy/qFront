/**
 * @file OpenQASMPrinter.hpp
 * @author Filip Novak
 * @date 2026-04-10
 */
#pragma once
#include "ir.hpp"
#include "Printer.hpp"
#include <ostream>
#include <stdexcept>

class OpenQASMPrinter : public Printer {
public:
    float version = 3.0; // default to 3.0, can be set to 2.0 for compatibility mode

    OpenQASMPrinter() = default;
    ~OpenQASMPrinter() = default;

    void print(const IR& ir, std::ostream& out) override;

    std::string name()        const override { return "OpenQASM." + std::to_string(version); }
    std::string extension()   const override { return "qasm"; }
    std::string description() const override { return "OpenQASM circuit format"; }

private:
    void printHeader(std::ostream& out);
    void printRegisters(const IR& ir, std::ostream& out);
    void printProgram(const IR& ir, std::ostream& out);

    void printBlock(const Block& block, const IR& ir, std::ostream& out, int depth);
    void printNode(const ProgramNodeBase& node, const IR& ir, std::ostream& out, int depth);
    void printGateApplication(const GateApplication& app, const IR& ir, std::ostream& out, int depth);
    void printLoopApplication(const LoopApplication& loop, const IR& ir, std::ostream& out, int depth);
    void printConditionalApplication(const ConditionalApplication& cond, const IR& ir, std::ostream& out, int depth);
};

/* EOF OpenQASMPrinter.hpp */
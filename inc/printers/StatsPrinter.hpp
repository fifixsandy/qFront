/**
 * @file StatsPrinter.hpp
 * @author Filip Novak
 * @date 2026-04-14
 */
#pragma once
#include "ir.hpp"
#include "Printer.hpp"
#include <ostream>
#include <stdexcept>

class StatsPrinter : public Printer {
public:
    StatsPrinter() = default;
    ~StatsPrinter() = default;

    void print(const IR& ir, std::ostream& out) override;

    std::string name()        const override { return "Stats."; }
    std::string extension()   const override { return "stats"; }
    std::string description() const override { return "Circuit statistics (gate counts, depth, etc.)"; }
private:
    void printHeader(std::ostream& out);
    void printRegisters(const IR& ir, std::ostream& out);
    void printGates(const IR& ir, std::ostream& out);
    void printSubroutines(const IR& ir, std::ostream& out);
    void collectGateCallCounts(const Block& block, const IR& ir, 
        std::ostream& out, std::unordered_map<std::string, long long>& gate_counts, long long multiplier=1);
};

/* EOF StatsPrinter.hpp */
/**
 * @file StimPrinter.hpp
 * @author Filip Novak
 * @date 2026-01-29
 */
#pragma once
#include "ir.hpp"
#include "Printer.hpp"
#include <ostream>
#include <unordered_map>
#include <vector>

class StimPrinter : public Printer {
public:
    void print(const IR& ir, std::ostream& out) override;
    
    std::string name() const override { return "Stim"; }
    std::string extension() const override { return "stim"; }
    std::string description() const override { 
        return "Stim stabilizer circuit simulator format (.stim)"; 
    }
private:

    size_t resolveQubit(const RegisterRef& ref, const IR& ir) const;
    void printAtomicGate(const GateApplication& app, const GateDef &gdef, const IR& ir, std::ostream& out);
    void printCompositeGate(const GateApplication& app, const GateDef &gdef, const IR& ir, std::ostream& out);
    void printGate(const GateApplication& app, const IR& ir, std::ostream& out);
    void printToffoli(const std::vector<RegisterRef>& ops, const IR& ir, std::ostream& out);
    void printBlock(const Block& block, const IR& ir, std::ostream& out);
    void printProgramNode(const ProgramNodeBase& node, const IR& ir, std::ostream& out);
    
    const std::unordered_map<std::string, std::string> _gate_map = {
        {"h", "H"}, {"x", "X"}, {"y", "Y"}, {"z", "Z"},
        {"s", "S"}, {"sdg", "S_DAG"}, {"t", "T"}, {"tdg", "T_DAG"},
        {"cx", "CNOT"}, {"cz", "CZ"}, {"measure", "M"}
    };
    
    std::unordered_map<std::string, size_t> _qubit_base;
};

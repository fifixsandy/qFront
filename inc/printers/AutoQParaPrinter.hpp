/**
 * @file AutoQParaPrinter.hpp
 * @author Filip Novak
 * @date 2026-02-11
 */
#pragma once
#include "ir.hpp"
#include "Printer.hpp"
#include <ostream>
#include <unordered_map>
#include <vector>

/**
 * @class AutoQParaPrinter
 * @brief Printer for converting IR to AutoQPara format
 * 
 * This class provides functionality to serialize the internal representation (IR)
 * to a human-readable AutoQPara format with proper indentation and structure.
 */
class AutoQParaPrinter : public Printer {
public:
    void print(const IR& ir, std::ostream& out) override;
    
    std::string name() const override { return "AutoQ-Para"; }
    std::string extension() const override { return "AutoQ-Para"; }
    std::string description() const override { 
        return "AutoQ-Para circuit format"; 
    }

    AutoQParaPrinter() = default;

    ~AutoQParaPrinter() = default;

    void printHeader(std::ostream& out) const;

    void printFooter(std::ostream& out) const;

    void printRegister(const RegisterDef& reg, int indentLvl, std::ostream& out) const;

    void printRegisterTable(const IR& ir, std::ostream& out) const;

    void printGateStmt(const GateStmt& stmt, int indentLvl, const IR& ir, std::ostream& out) const;

    void printGatePlacement(const GatePlacement& p, int indentLvl, const IR& ir, std::ostream& out) const;

    void printRepeatBlock(const RepeatBlock& r, int indentLvl, const IR& ir, std::ostream& out) const;

    void printGate(const GateDef& gate, const IR& ir, int indentLvl, std::ostream& out);

    void printGateTable(const IR& ir, std::ostream& out);

    void printVariables(const std::vector<VariableDef>& vars, std::ostream& out, int indentLvl) const;

    void printGlobalVariables(const IR& ir, std::ostream& out) const;

    void printLoopValues(const LoopApplication& loop, int indentLvl, std::ostream& out) const;

    void printTransducerDefs(const IR& ir, std::ostream& out) const;

    void printProgram(const IR& ir, std::ostream& out) const;

    void printSubroutineTable(const IR& ir, std::ostream& out) const;


private:

    idGate getLocalGateId(idGate irGateId) const;
    idGate addLocalGateId(idGate irGateId);

    std::unordered_map<idGate, idGate> local_gates;
};

/**
 * This file will be replaced by proper output (i hope lol)
 */


#include <iostream>
#include "../inc/temp_printer.hpp"
static std::string indent(int n) {
    return std::string(n, ' ');
}
void printGatePlacement(const GatePlacement& p, int indentLvl) {
    std::cout << indent(indentLvl)
              << "GatePlacement(.gate_id = " << p.gate_id
              << ", .inputs = {";

    for (size_t i = 0; i < p.relativeInputs.size(); ++i) {
        std::cout << p.relativeInputs[i];
        if (i + 1 < p.relativeInputs.size())
            std::cout << ", ";
    }
    std::cout << "})";
}

void printGate(const GateDef& gate, const IR& ir, int indentLvl) {
    std::cout << indent(indentLvl) << "Gate(\n";

    std::cout << indent(indentLvl + 2)
              << ".name = \"" << gate.name << "\",\n";

    std::cout << indent(indentLvl + 2)
              << ".num_qubits = " << gate.argument_qubits.size() << ",\n";

    if (gate.kind == GateKind::Atomic) {
        std::cout << indent(indentLvl + 2)
                  << "Matrix(...)\n";
    } else {
        const auto& body =
            std::get<CompositeGateBody>(gate.semantics).body;

        std::cout << indent(indentLvl + 2)
                  << "Composition({\n";

        for (const auto& placement : body) {
            printGatePlacement(placement, indentLvl + 4);
            std::cout << ",\n";
        }

        std::cout << indent(indentLvl + 2)
                  << "})\n";
    }

    std::cout << indent(indentLvl) << ")";
}


void printGateTable(const IR& ir) {
    std::cout << ".gates = {\n";

    const auto& gates = ir.getAllGates();
    for (size_t i = 0; i < gates.size(); ++i) {
        printGate(gates[i], ir, 2);
        if (i + 1 < gates.size())
            std::cout << ",";
        std::cout << "\n";
    }

    std::cout << "}\n";
}


const char* toString(RegisterKind kind) {
    switch (kind) {
        case RegisterKind::Nonparametric: return "NONPARAMETRIC";
        case RegisterKind::Parametric:    return "PARAMETRIC";
        default:                          return "UNKNOWN";
    }
}

void printRegister(const RegisterDef& reg, int indentLvl) {
    std::cout << std::string(indentLvl, ' ')
              << "Register(.kind = " << toString(reg.kind)
              << ", .size = " << reg.size
              << ", \"" << reg.name << "\")";
}

void printRegisterTable(const IR& ir) {
    std::cout << ".registers = {\n";

    const auto& regs = ir.getAllRegisters();
    for (size_t i = 0; i < regs.size(); ++i) {
        printRegister(regs[i], 2);
        if (i + 1 < regs.size())
            std::cout << ",";
        std::cout << "\n";
    }

    std::cout << "}\n";
}

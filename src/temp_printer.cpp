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

void printLoopValues(const LoopApplication& loop, int indent) {
    auto pad = std::string(indent, ' ');

    if (std::holds_alternative<Interval>(loop.values)) {
        const auto& interval = std::get<Interval>(loop.values);

        std::cout << pad << ".form = \"interval\",\n";
        std::cout << pad << ".start = " << interval.start << ",\n";
        std::cout << pad << ".end   = " << interval.end << ",\n";

        if (!interval.step.empty())
            std::cout << pad << ".step  = " << interval.step << ",\n";
        else
            std::cout << pad << ".step  = 1,\n";
    }
    else {
        const auto& collection =
            std::get<std::vector<std::string>>(loop.values);

        std::cout << pad << ".form = \"collection\",\n";
        std::cout << pad << ".values = { ";

        for (size_t i = 0; i < collection.size(); ++i) {
            std::cout << collection[i];
            if (i + 1 < collection.size()) std::cout << ", ";
        }
        std::cout << " },\n";
    }
}

void printProgram(const IR& ir) {
    std::cout << ".program = {\n";
    for (std::size_t i = 0; i < ir.getProgram().size(); ++i) {
        std::cout << "  TransducerApplication(.transducer_id = " << std::to_string(i) << " ),\n";
    }
    std::cout << "}\n";
}

void printTransducerDefs(const IR& ir) {
    std::cout << ".transducer_defs = {\n";

    const auto& program = ir.getProgram();
    for (const auto& p : program) {

        if (auto gateApp = dynamic_cast<GateApplication*>(p.get())) {
            std::cout << "  SingleGate(\n";
            std::cout << "    .gate_id = " << gateApp->gate_id << ",\n";
            std::cout << "    .inputs = {\n";

            for (size_t i = 0; i < gateApp->operands.size(); ++i) {
                const auto& op = gateApp->operands[i];
                std::cout << "      RegisterRef(.reg_id = "
                          << op.reg_id << ", .qubit_id = "
                          << op.qubit_index << ")";
                if (i + 1 < gateApp->operands.size()) std::cout << ",";
                std::cout << "\n";
            }

            std::cout << "    }\n";
            std::cout << "  ),\n";
        }

        else if (auto loop = dynamic_cast<LoopApplication*>(p.get())) {
            std::cout << "  FromLoop(\n";

            for (const auto& stmt : loop->body) {
                if (auto gate = dynamic_cast<GateApplication*>(stmt.get())) {
                    std::cout << "    .gate_id = " << gate->gate_id << ",\n";
                    std::cout << "    .inputs = {\n";

                    for (const auto& op : gate->operands) {
                        std::cout << "      RegisterRef(.reg_id = "
                                  << op.reg_id
                                  << ", .qubit_id = "
                                  << op.qubit_index << "),\n";
                    }

                    std::cout << "    },\n";
                }
            }

            std::cout << "    .variable = " << loop->variable << ",\n";
            printLoopValues(*loop, 4);

            std::cout << "  ),\n";
        }

        else {
            std::cout << "  <Unknown ProgramNode>,\n";
        }
    }

    std::cout << "}\n";
}

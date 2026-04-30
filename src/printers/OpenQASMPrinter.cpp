/**
 * @file OpenQASMPrinter.cpp
 * @author Filip Novak
 * @date 2026-04-10
 */

#include "../../inc/ir.hpp"
#include "../../inc/printers/OpenQASMPrinter.hpp"
#include <iomanip> // for std::setprecision

void OpenQASMPrinter::print(const IR& ir, std::ostream& out) {
    printHeader(out);
    printRegisters(ir, out);
    printProgram(ir, out);
}

void OpenQASMPrinter::printHeader(std::ostream& out) {
    out << "OPENQASM "
    << std::fixed << std::setprecision(1)
    << version
    << ";\n";
    out << "include \"qelib1.inc\";\n\n"; // TODO: include based on loaded program
}

void OpenQASMPrinter::printRegisters(const IR& ir, std::ostream& out) {
    for (const auto& reg : ir.getAllRegisters()) {
        if (reg.size == "0") {
            // skip zero-size registers (e.g. removed by optimizations)
            continue;
        }
        switch (reg.type) {
            case RegisterType::Qubit:
                if (version >= 3) {
                    out << "qubit " << reg.name << "[" << reg.size << "];\n";
                } else {
                    out << "qreg " << reg.name << "[" << reg.size << "];\n";
                }
                break;
            case RegisterType::Int:
                if (version >= 3) {
                    out << "int " << reg.name << "[" << reg.size << "];\n";
                } else {
                    out << "creg " << reg.name << "[" << reg.size << "];\n";
                }
                break;
        }
    }
    out << "\n";
}

void OpenQASMPrinter::printProgram(const IR& ir, std::ostream& out) {
    const Block& blk = ir.getGlobalBlock();
    printBlock(blk, ir, out, 0);
}

static std::string indent(int depth) {
    return std::string(depth * 4, ' ');
}

static std::string registerRefStr(const RegisterRef& ref, const IR& ir) {
    const RegisterDef& reg = ir.getRegister(ref.reg_id);
    return reg.name + "[" + ref.qubit_index + "]";
}

void OpenQASMPrinter::printBlock(const Block& block, const IR& ir,
                                  std::ostream& out, int depth) {
    for (const auto& node : block.body) {
        printNode(*node, ir, out, depth);
    }
}

void OpenQASMPrinter::printNode(const ProgramNodeBase& node, const IR& ir,
                                 std::ostream& out, int depth) {
    if (const auto* gate = dynamic_cast<const GateApplication*>(&node)) {
        printGateApplication(*gate, ir, out, depth);
    }
    else if (const auto* loop = dynamic_cast<const LoopApplication*>(&node)) {
        printLoopApplication(*loop, ir, out, depth);
    }
    else if (const auto* cond = dynamic_cast<const ConditionalApplication*>(&node)) {
        printConditionalApplication(*cond, ir, out, depth);
    }
}

void OpenQASMPrinter::printGateApplication(const GateApplication& app,
                                            const IR& ir,
                                            std::ostream& out, int depth) {
    const GateDef& gate = ir.getGate(app.gate_id);
    out << indent(depth) << gate.name;

    // parameters such as rotation angles for parametric gates (e.g. RZ(phi))
    if (!app.params.empty()) {
    out << "(";
    for (size_t i = 0; i < app.params.size(); ++i) {
        out << (i == 0 ? "" : ",") << app.params[i];
    }
    out << ")";
    }

    // operands (qubit arguments)
    for (size_t i = 0; i < app.operands.size(); ++i) {
        out << (i == 0 ? " " : ",") << registerRefStr(app.operands[i], ir);
    }
    out << ";\n";
}

void OpenQASMPrinter::printLoopApplication(const LoopApplication& loop,
                                            const IR& ir,
                                            std::ostream& out, int depth) {
    if (version >= 3) {
        out << indent(depth) << "for (int " << loop.variable << " in ";
        std::visit([&](auto&& values) {
            using T = std::decay_t<decltype(values)>;
            if constexpr (std::is_same_v<T, Interval>) {
                out << "[" << values.start << ":" << values.step << ":" << values.end << "]";
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                out << "{";
                for (size_t i = 0; i < values.size(); ++i) {
                    out << values[i];
                    if (i + 1 < values.size()) out << ", ";
                }
                out << "}";
            } else if constexpr (std::is_same_v<T, std::string>) {
                out << values;
            }
        }, loop.values);
        out << ") {\n";
        printBlock(loop.body, ir, out, depth + 1);
        out << indent(depth) << "}\n";
    } else {
        // OpenQASM 2.0 does not support loops, error and suggest unrolling or using a higher version
        throw std::runtime_error("OpenQASM 2.0 does not support loops. Suggesting: unroll with '--unroll-loops' or use OpenQASM 3.0.");
    }
}

void OpenQASMPrinter::printConditionalApplication(const ConditionalApplication& cond,
                                                   const IR& ir,
                                                   std::ostream& out, int depth) {
    out << indent(depth) << "if (" << cond.condition_expr << ") {\n";
    for (const auto& node : cond.then_body)
        printNode(*node, ir, out, depth + 1);

    if (!cond.else_body.empty()) {
        out << indent(depth) << "} else {\n";
        for (const auto& node : cond.else_body)
            printNode(*node, ir, out, depth + 1);
    }
    out << indent(depth) << "}\n";
}

/* EOF OpenQASMPrinter.cpp */
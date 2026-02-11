/**
 * @file AutoQParaPrinter.cpp
 * @author Filip Novak
 * @date 2026-02-11
 */

#include "../../inc/ir.hpp"
#include "../../inc/printers/AutoQParaPrinter.hpp"

static std::string indent(int n) {
    return std::string(n, ' ');
}

static const char* toString(RegisterKind kind) {
    switch (kind) {
        case RegisterKind::Nonparametric: return "NONPARAMETRIC";
        case RegisterKind::Parametric:    return "PARAMETRIC";
        default:                          return "UNKNOWN";
    }
}

idGate AutoQParaPrinter::getLocalGateId(idGate irGateId) const {
    auto it = local_gates.find(irGateId);
    
    if (it == local_gates.end()) {
        throw std::out_of_range(
            "Gate ID " + std::to_string(irGateId) + 
            " not found in local gate mapping"
        );
    }
    
    return it->second;
}


idGate AutoQParaPrinter::addLocalGateId(idGate irGateId) {
    auto it = local_gates.find(irGateId);
    if (it != local_gates.end()) {
        return it->second;  // return existing
    }
    
    idGate localGateId = static_cast<idGate>(local_gates.size());
    
    local_gates[irGateId] = localGateId;
    
    return localGateId;
}

void AutoQParaPrinter::printHeader(std::ostream& out) const {
    out << "ParametricProgram(\n";
}

void AutoQParaPrinter::printFooter(std::ostream& out) const {
    out << ")\n";
}


void AutoQParaPrinter::printRegister(const RegisterDef& reg, int indentLvl, std::ostream& out) const {
    out << indent(indentLvl)
        << "Register(.kind = " << toString(reg.kind)
        << ", .type = " << (reg.type == RegisterType::Qubit ? "Qubit" : "Int")
        << ", .size = " << reg.size
        << ", .name = \"" << reg.name << "\")";
}

void AutoQParaPrinter::printRegisterTable(const IR& ir, std::ostream& out) const {
    out << indent(1) << ".registers = {\n";

    const auto& regs = ir.getAllRegisters();
    for (size_t i = 0; i < regs.size(); ++i) {
        printRegister(regs[i], 2, out);
        if (i + 1 < regs.size()) out << ",";
        out << "\n";
    }

    out << indent(1) << "},\n";
}

void AutoQParaPrinter::printGateStmt(const GateStmt& stmt, int indentLvl, const IR& ir, std::ostream& out) const {
    std::visit(
        [&](auto&& s) {
            using T = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<T, GatePlacement>) {
                printGatePlacement(s, indentLvl, ir, out);
            } else if constexpr (std::is_same_v<T, RepeatBlock>) {
                printRepeatBlock(s, indentLvl, ir, out);
            }
        }, stmt);
}

void AutoQParaPrinter::printGatePlacement(const GatePlacement& p, int indentLvl, const IR& ir, std::ostream& out) const {
    const auto& gate = ir.getGate(p.gate_id);
    out << indent(indentLvl)
        << "GatePlacement(.gate = \"" << gate.name << "\""
        << ", .gate_id = " << p.gate_id
        << ", .inputs = {";
    for (size_t i = 0; i < p.relativeInputs.size(); ++i) {
        out << p.relativeInputs[i];
        if (i + 1 < p.relativeInputs.size())
            out << ", ";
    }
    out << "})\n";
}

void AutoQParaPrinter::printRepeatBlock(const RepeatBlock& r, int indentLvl, const IR& ir, std::ostream& out) const {
    out << indent(indentLvl)
        << "RepeatBlock(.count = " << r.count
        << ", .body = [\n";

    for (const auto& stmt : r.body) {
        printGateStmt(stmt, indentLvl + 1, ir, out);
    }

    out << indent(indentLvl) << "])\n";
}

void AutoQParaPrinter::printGate(const GateDef& gate, const IR& ir, int indentLvl, std::ostream& out) {
    out << indent(indentLvl) << "Gate(\n";

    out << indent(indentLvl + 2)
        << ".name = \"" << gate.name << "\",\n";

    out << indent(indentLvl + 2)
        << ".num_qubits = " << gate.argument_qubits.size() << ",\n";

    if (gate.kind == GateKind::Atomic) {
        const auto& sem = std::get<AtomicGateSemantics>(gate.semantics);
        out << indent(indentLvl + 2)
            << "Matrix(" << sem.matrix << ")\n";
    } else {
        const auto& body = std::get<CompositeGateBody>(gate.semantics).body;

        out << indent(indentLvl + 2)
            << "Composition({\n";

        for (const auto& stmt : body) {
            printGateStmt(stmt, indentLvl + 4, ir, out);
            out << indent(indentLvl + 4) << ",\n";
        }

        out << indent(indentLvl + 2) << "})\n";
    }

    out << indent(indentLvl) << ")";

    addLocalGateId(ir.getGateId(gate.name));
}

void AutoQParaPrinter::printGateTable(const IR& ir, std::ostream& out) {
    out << indent(1) << ".gates = {\n";

    const auto& gates = ir.getAllGates();
    for (size_t i = 0; i < gates.size(); ++i) {
        if (!gates[i].used) continue;
        printGate(gates[i], ir, 2, out);
        out << ",\n";
    }

    out << indent(1) << "},\n";
}

void AutoQParaPrinter::printVariables(const std::vector<VariableDef>& vars, std::ostream& out, int indentLvl = 2) const {
    out << indent(indentLvl) << ".variables = {\n";
    for (const auto& var : vars) {
        out << indent(indentLvl + 2)
            << "Variable(.name = \"" << var.name << "\", "
            << ".type = \"" << var.type.base << "\", "
            << ".is_const = " << (var.is_const ? "true" : "false") << ", "
            << ".initializer = \"" << var.initializer << "\"),\n";
    }
    out << indent(indentLvl) << "},\n";
}

void AutoQParaPrinter::printGlobalVariables(const IR& ir, std::ostream& out) const {
    out << indent(1) << ".global_variables = {\n";
    printVariables(ir.getGlobalBlock().variables, out, 2);
    out << indent(1) << "},\n";
}

void AutoQParaPrinter::printLoopValues(const LoopApplication& loop, int indentLvl, std::ostream& out) const {
    auto pad = indent(indentLvl);

    if (std::holds_alternative<Interval>(loop.values)) {
        const auto& interval = std::get<Interval>(loop.values);
        out << pad << ".form = \"interval\",\n";
        out << pad << ".start = " << interval.start << ",\n";
        out << pad << ".end   = " << interval.end << ",\n";
        out << pad << ".step  = " 
            << (interval.step.empty() ? "1" : interval.step) << ",\n";
    } else if (std::holds_alternative<std::vector<std::string>>(loop.values)) {
        const auto& collection = std::get<std::vector<std::string>>(loop.values);
        out << pad << ".form = \"collection\",\n";
        out << pad << ".values = { ";
        for (size_t i = 0; i < collection.size(); ++i) {
            out << collection[i];
            if (i + 1 < collection.size()) out << ", ";
        }
        out << " },\n";
    } else {
        const auto& expr = std::get<std::string>(loop.values);
        out << pad << ".form = \"expression\",\n";
        out << pad << ".expr = \"" << expr << "\",\n";
    }
}

// TODO: merge transducer definitions for same semantics
void AutoQParaPrinter::printTransducerDefs(const IR& ir, std::ostream& out) const {
    out << indent(1) << ".transducer_defs = {\n";

    const auto& program = ir.getGlobalBlock();

    for (const auto& p : program.body) {
        if (auto gateApp = dynamic_cast<GateApplication*>(p.get())) {
            out << indent(2) << "SingleGate(\n";
            out << indent(4) << ".gate_id = " << getLocalGateId(gateApp->gate_id) << ",\n";
            out << indent(4) << ".inputs = {\n";

            for (size_t i = 0; i < gateApp->operands.size(); ++i) {
                const auto& op = gateApp->operands[i];
                out << indent(6) << "RegisterRef(.reg_id = "
                    << op.reg_id << ", .qubit_id = "
                    << op.qubit_index << ")";
                if (i + 1 < gateApp->operands.size()) out << ",";
                out << "\n";
            }

            out << indent(4) << "}\n";
            out << indent(2) << "),\n";
        } else if (auto loop = dynamic_cast<LoopApplication*>(p.get())) {
            out << indent(2) << "FromLoop(\n";
            printVariables(loop->body.variables, out, 4);

            for (const auto& stmt : loop->body.body) {
                if (auto gate = dynamic_cast<GateApplication*>(stmt.get())) {
                    out << indent(4) << ".gate_id = " << gate->gate_id << ",\n";
                    out << indent(4) << ".inputs = {\n";

                    for (const auto& op : gate->operands) {
                        out << indent(6) << "RegisterRef(.reg_id = "
                            << op.reg_id
                            << ", .qubit_id = "
                            << op.qubit_index << "),\n";
                    }

                    out << indent(4) << "},\n";
                }
            }

            out << indent(4) << ".variable = " << loop->variable << ",\n";
            printLoopValues(*loop, 4, out);
            out << indent(2) << "),\n";
        } else {
            out << indent(2) << "<Unknown ProgramNode>,\n";
        }
    }

    out << indent(1) << "},\n";
}

void AutoQParaPrinter::printProgram(const IR& ir, std::ostream& out) const {
    out << indent(1) << ".program = {\n";
    const auto& prog = ir.getGlobalBlock().body;
    for (std::size_t i = 0; i < prog.size(); ++i) {
        out << indent(2) << "TransducerApplication(.transducer_id = "
            << std::to_string(i) << " ),\n";
    }
    out << indent(1) << "}\n";
}

void AutoQParaPrinter::printSubroutineTable(const IR& ir, std::ostream& out) const {
    const auto& subroutines = ir.getAllSubroutines();

    if (subroutines.size() == 0) {
        return;
    }

    out << indent(1) << ".subroutines = {\n";

    for (size_t i = 0; i < subroutines.size(); ++i) {
        if (!subroutines[i].used) continue;
        out << indent(2) << "Subroutine(.name = \"" << subroutines[i].name
            << "\", .num_params = " << subroutines[i].parameters.size() << "),\n";
    }

    out << indent(1) << "},\n";
}

void AutoQParaPrinter::print(const IR& ir, std::ostream& out) {
    printHeader(out);
    printRegisterTable(ir, out);
    printGlobalVariables(ir, out);
    printGateTable(ir, out);
    printSubroutineTable(ir, out);
    printTransducerDefs(ir, out);
    printProgram(ir, out);
    printFooter(out);
}
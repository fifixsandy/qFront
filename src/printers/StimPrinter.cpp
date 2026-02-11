/**
 * @file StimPrinter.cpp
 * @author Filip Novak
 * @date 2026-01-29
 */
#include "../../inc/printers/StimPrinter.hpp"
#include <stdexcept>
#include <sstream>

void StimPrinter::print(const IR& ir, std::ostream& out) {

    _qubit_base.clear();
    size_t base = 0;
    for (const auto& reg : ir.getAllRegisters()) {
        if (reg.type == RegisterType::Qubit) {
            _qubit_base[reg.name] = base;
            base += std::stoi(reg.size);  // compile time constant
        }
    }
    
    out << "# Stim circuit from OpenQASM IR (n_qubits=" << base << ")\n";
    
    // Printing program
    for (const auto& node_ptr : ir.getGlobalBlock().body) {
        printProgramNode(*node_ptr, ir, out);
    }
}

size_t StimPrinter::resolveQubit(const RegisterRef& ref, const IR& ir) const {
    const auto& reg = ir.getRegister(ref.reg_id);
    if (reg.type != RegisterType::Qubit) {
        throw std::runtime_error("Non-qubit register in gate");
    }
    
    // TODO: evaluate ref.qubit_index as constant expression
    // for now, just integer index
    size_t idx = std::stoi(ref.qubit_index);
    auto it = _qubit_base.find(reg.name);
    if (it == _qubit_base.end()) {
        throw std::runtime_error("Unknown qubit register: " + reg.name);
    }
    return it->second + idx;
}

void StimPrinter::printAtomicGate(const GateApplication& app, 
                                  const GateDef &gdef, 
                                  const IR& ir, 
                                  std::ostream& out) {
    auto it = _gate_map.find(gdef.name);
    if (it != _gate_map.end()) {
        // print the gate identificator
        out << it->second;
        // print operand qubits
        for (const auto& op : app.operands) {
            out << " " << resolveQubit(op, ir);
        }
        out << "\n";
    } else {
        throw std::runtime_error("Unsupported atomic gate: " + gdef.name);
    }
}

void StimPrinter::printCompositeGate(const GateApplication& app, 
                                  const GateDef &gdef, 
                                  const IR& ir, 
                                  std::ostream& out) { 
    // TODO: printing composite gates
}



void StimPrinter::printGate(const GateApplication& app, const IR& ir, std::ostream& out) {
    const auto& gdef = ir.getGate(app.gate_id);
    
    if (gdef.kind == GateKind::Atomic) {
        printAtomicGate(app, gdef, ir, out);
    } else {
        printCompositeGate(app, gdef, ir, out);
    }

}


void StimPrinter::printBlock(const Block& block, const IR& ir, std::ostream& out) {
    out << "{\n";
    for (const auto& node_ptr : block.body) {
        printProgramNode(*node_ptr, ir, out);
    }
    out << "}\n";
}

void StimPrinter::printProgramNode(const ProgramNodeBase& node, 
                                      const IR& ir, std::ostream& out) {
    if (auto* app = dynamic_cast<const GateApplication*>(&node)) {
        printGate(*app, ir, out);
    } else if (auto* loop = dynamic_cast<const LoopApplication*>(&node)) {
        if (auto* interval = std::get_if<Interval>(&loop->values)) {
            // (end - start)/step + 1
            size_t start = std::stoi(interval->start);
            size_t end = std::stoi(interval->end);
            size_t step = std::stoi(interval->step);
            size_t reps = (end - start) / step + 1;
            out << "REPEAT " << reps << " ";
            printBlock(loop->body, ir, out);
        }
    } else if (auto* cond = dynamic_cast<const ConditionalApplication*>(&node)) {
        throw std::runtime_error("Stim does not support Conditionals");
    } else {
        throw std::runtime_error("Unsupported program node by Stim (only REPEAT blocks supported).");
    }
}

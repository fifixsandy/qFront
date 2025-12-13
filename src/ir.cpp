/**
 * @file ir.cpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#include "ir.hpp"
#include <stdexcept>

std::size_t IR::addRegister(const RegisterDef& def) {
    if (hasRegister(def.name)) {
        throw std::runtime_error("Register already exists: " + def.name);
    }
    std::size_t id = registers.size();
    registers.push_back(def);
    register_table[def.name] = id;
    return id;
}

const RegisterDef& IR::getRegister(std::size_t id) const {
    if (id >= registers.size()) {
        throw std::out_of_range("Invalid register id");
    }
    return registers[id];
}

const RegisterDef& IR::getRegister(const std::string& name) const {
    auto it = register_table.find(name);
    if (it == register_table.end()) {
        throw std::runtime_error("Unknown register: " + name);
    }
    return registers[it->second];
}
    
const std::vector<RegisterDef> IR::getAllRegisters() const {
    return this->registers;
}

bool IR::hasRegister(const std::string& name) const {
    return register_table.find(name) != register_table.end();
}

std::size_t IR::addGate(const GateDef& def) {
    if (hasGate(def.name)) {
        throw std::runtime_error("Gate already exists: " + def.name);
    }
    std::size_t id = gates.size();
    gates.push_back(def);
    gate_table[def.name] = id;
    return id;
}

const GateDef& IR::getGate(std::size_t id) const {
    if (id >= gates.size()) {
        throw std::out_of_range("Invalid gate id");
    }
    return gates[id];
}

const GateDef& IR::getGate(const std::string& name) const {
    auto it = gate_table.find(name);
    if (it == gate_table.end()) {
        throw std::runtime_error("Unknown gate: " + name);
    }
    return gates[it->second];
}

bool IR::hasGate(const std::string& name) const {
    return gate_table.find(name) != gate_table.end();
}

/* EOF ir.cpp */
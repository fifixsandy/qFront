/**
 * @file ir.cpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#include "ir.hpp"
#include <stdexcept>
#include <iostream>

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

const idRegister IR::getRegisterId(std::string name) const {
    auto it = register_table.find(name);
    if (it == register_table.end()) {
        throw std::runtime_error("Register not found: " + name);
    }
    return it->second;
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

    // register canonical name
    gate_table[def.name] = id;

    // register aliases
    for (const auto& alias : def.aliases) {
        if (hasGate(alias)) {
            throw std::runtime_error("Alias already exists: " + alias);
        }
        gate_table[alias] = id;
    }

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

GateDef& IR::getGate(std::size_t id) { return gates.at(id); }
GateDef& IR::getGate(const std::string& name) { return gates.at(gate_table.at(name)); }

void IR::markGateUsed(std::size_t id) {
    if (id >= gates.size()) {
        throw std::out_of_range("Invalid gate id");
    }
    gates[id].used = true;
}

void IR::markGateUsed(const std::string& name) {
    auto it = gate_table.find(name);
    if (it == gate_table.end()) {
        throw std::runtime_error("Unknown gate: " + name);
    }
    gates[it->second].used = true;
}


const std::vector<GateDef> IR::getAllGates() const {
    return this->gates;
}

const idGate IR::getGateId(std::string name) const {
    auto it = gate_table.find(name);
    if (it == gate_table.end()) {
        throw std::runtime_error("Gate not found: " + name);
    }
    return it->second;
}

bool IR::hasGate(const std::string& name) const {
    return gate_table.find(name) != gate_table.end();
}

Block& IR::getGlobalBlock() { return global_block; }
const Block& IR::getGlobalBlock() const { return global_block; }

const VariableDef& IR::getGlobalVariable(const std::string& name) const {
    for (const auto& var : global_block.variables) {
        if (var.name == name) {
            return var;
        }
    }
    throw std::runtime_error("Global variable not found: " + name);
}

std::size_t IR::addSubroutine(SubroutineDef&& def) {
    if (hasSubroutine(def.name)) {
        throw std::runtime_error("Subroutine already exists: " + def.name);
    }

    std::size_t id = subroutines.size();
    subroutine_table[def.name] = id;
    subroutines.push_back(std::move(def));

    return id;
}

const SubroutineDef& IR::getSubroutine(std::size_t id) const {
    if (id >= subroutines.size()) {
        throw std::out_of_range("Invalid subroutine id");
    }
    return subroutines[id];
}

const SubroutineDef& IR::getSubroutine(const std::string& name) const {
    auto it = subroutine_table.find(name);
    if (it == subroutine_table.end()) {
        throw std::runtime_error("Unknown subroutine: " + name);
    }
    return subroutines[it->second];
}

SubroutineDef& IR::getSubroutine(const std::string& name) {
    auto it = subroutine_table.find(name);
    if (it == subroutine_table.end()) {
        throw std::runtime_error("Unknown subroutine: " + name);
    }
    return subroutines.at(it->second);
}

SubroutineDef& IR::getSubroutine(std::size_t id) {
    if (id >= subroutines.size()) {
        throw std::out_of_range("Invalid subroutine id");
    }
    return subroutines[id];
}

const std::vector<SubroutineDef>& IR::getAllSubroutines() const {
    return subroutines;
}

const idGate IR::getSubroutineId(std::string name) const {
    auto it = subroutine_table.find(name);
    if (it == subroutine_table.end()) {
        throw std::runtime_error("Subroutine not found: " + name);
    }
    return it->second;
}

bool IR::hasSubroutine(const std::string& name) const {
    return subroutine_table.find(name) != subroutine_table.end();
}

void IR::markSubroutineUsed(std::size_t id) {
    if (id >= subroutines.size()) {
        throw std::out_of_range("Invalid subroutine id");
    }
    subroutines[id].used = true;
}

void IR::markSubroutineUsed(const std::string& name) {
    auto it = subroutine_table.find(name);
    if (it == subroutine_table.end()) {
        throw std::runtime_error("Unknown subroutine: " + name);
    }
    subroutines[it->second].used = true;
}

/* EOF ir.cpp */
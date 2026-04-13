/**
 * @file merge.cpp
 * @author Filip Novak
 */
#include "merge.hpp"

#include <stdexcept>


std::vector<std::pair<idRegister, std::size_t>>
collectMergeableRegisters(const IR& ir) {
    std::vector<std::pair<idRegister, std::size_t>> mergeable;
    
    for (const auto& reg : ir.getAllRegisters()) {
        if (reg.kind == RegisterKind::Nonparametric &&
            reg.type == RegisterType::Qubit) {
            try {
                std::size_t size = std::stoul(reg.size);
                mergeable.emplace_back(ir.getRegisterId(reg.name), size);
            } catch (const std::invalid_argument&) {
                // skip registers with non-integer sizes for now
            }
        }
    }
    return mergeable;
}

std::unordered_map<idRegister, std::size_t>
computeOffsets(const std::vector<std::pair<idRegister, std::size_t>>& regs) {
    std::unordered_map<idRegister, std::size_t> offset_map;
    std::size_t current_offset = 0;
    for (const auto& [reg_id, size] : regs) {
        offset_map[reg_id] = current_offset;
        current_offset += size;
    }
    return offset_map;
}

void rewriteRef(
    RegisterRef& ref,
    const std::unordered_map<idRegister, std::size_t>& offset_map,
    idRegister merged_id
) {
    auto it = offset_map.find(ref.reg_id);
    if (it != offset_map.end()) {
        std::size_t offset = it->second;
        ref.reg_id = merged_id;

        try {
            std::size_t index = std::stoul(ref.qubit_index);
            ref.qubit_index = std::to_string(index + offset);
        } catch (const std::invalid_argument&) {
            // symbolic index: append offset as an expression
            ref.qubit_index += " + " + std::to_string(offset);
        }
    }
}
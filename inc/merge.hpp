/**
 * @file merge.hpp
 * @author Filip Novak
 *
 * Register merging pass - collapses all Nonparametric qubit registers
 * into a single contiguous register, rewriting all RegisterRefs in the IR.
 */
#pragma once

#include "ir.hpp"
#include <unordered_map>


/**
 * Collects all Nonparametric qubit registers whose size is a valid integer.
 * Registers with symbolic sizes (e.g. "n") are excluded.
 *
 * @return  Vector of (register_id, parsed_size) in order of id.
 */
std::vector<std::pair<idRegister, std::size_t>>
collectMergeableRegisters(const IR& ir);

/**
 * Builds a map from old register_id - offset in the merged register.
 * Offsets are assigned in order of the input vector.
 *
 * @param regs  Output of collectMergeableRegisters.
 * @return      Map: reg_id - start offset.
 */
std::unordered_map<idRegister, std::size_t>
computeOffsets(const std::vector<std::pair<idRegister, std::size_t>>& regs);

/**
 * Rewrites a single RegisterRef in-place.
 * If the ref's reg_id is in the offset map, updates reg_id to merged_id
 * and adjusts qubit_index by the offset.
 * Constant indices are resolved numerically.
 * Symbolic indices get "+ offset" appended as a string expression.
 *
 * @param ref           RegisterRef to rewrite.
 * @param offset_map    Map from old reg_id to offset.
 * @param merged_id     The id of the new merged register.
 */
void rewriteRef(
    RegisterRef& ref,
    const std::unordered_map<idRegister, std::size_t>& offset_map,
    idRegister merged_id
);


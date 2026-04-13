#pragma once
#include "ir.hpp"

namespace passes {

// Unrolls all LoopApplication nodes with compile-time constant iteration count
void unrollLoops(IR& ir);

// Inlines all CompositeGate bodies at their call sites
void inlineCompositeGates(IR& ir);

/**
 * @brief Decomposes all MCX gates in the IR into chains of X, CX, and CCX gates using the standard V-chain decomposition.
 * 
 * @warning This pass may add new ancilla qubit registers to the IR.
 * 
 * @param ir The IR context to modify
 */
void decomposeMCX(IR& ir);


/**
 * @brief Merges registers into one register when possible to reduce the total number of registers used.
 * 
 * @param ir The IR context to modify
 * 
 * @warning This pass will merge all Nonparametric qubit registers with constant integer sizes into a 
 *          single register named "__merged_qubits" and rename the merged registers "__unused" with size "0".
 *          Should be skipped during emitting.
 */
void mergeRegisters(IR& ir);

} // namespace passes
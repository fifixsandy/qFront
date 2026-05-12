#pragma once
#include "ir.hpp"

namespace passes {

/**
 * @brief Unrolls all LoopApplication nodes in the IR whose iteration count is a compile-time constant.
 * @param ir The IR context to modify
 */
void unrollLoops(IR& ir);

/**
 * @brief Inlines all CompositeGate bodies at their call sites, replacing composite gate applications
 *        with the sequence of atomic gate applications they are defined as.
 * @param ir The IR context to modify
 */
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

/**
 * @brief Evaluates all gate parameter expressions to their double-precision floating point values.
 *
 * Traverses all gate applications in the IR, including those inside loops and conditionals,
 * and replaces any symbolic or arithmetic parameter expressions (e.g. "pi/4", "3*pi/2")
 * with their evaluated double literal string representations.
 *
 * @note This pass should be run before any emitter that requires concrete numeric angle values,
 *       such as MOSFPrinter. It has no effect on gates with no parameters.
 *
 * @param ir The IR context to modify
 */
void evaluateAngles(IR& ir);

} // namespace passes
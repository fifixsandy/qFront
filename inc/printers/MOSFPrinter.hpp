/**
 * @file MOSFPrinter.hpp
 * @author Filip Novak
 * @date 2026-04-28
 *
 * Prints the IR as a MOSF (MTBDD Operation Serialization Format) JSON file.
 * Each gate application is translated to a MOSF op tree based on the
 * gate name, operand count, and BDD level ordering of the qubits.
 *
 * Requires nlohmann/json (header-only, https://github.com/nlohmann/json).
 * Based on https://github.com/VeriFIT/MoToBuddy/blob/main/doc/mosf.mosf.
 */
#pragma once

#include "ir.hpp"
#include "Printer.hpp"
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>


class MOSFPrinter : public Printer {
public:
    std::string mosf_version = "1.0";
    std::string tree_id      = "tree_id";

    MOSFPrinter() = default;
    ~MOSFPrinter() = default;

    void print(const IR& ir, std::ostream& out) override;

    std::string name()        const override { return "MOSF"; }
    std::string extension()   const override { return "mosf"; }
    std::string description() const override { return "MTBDD Operation Serialization Format"; }

private:
    //  Per-print mutable state (reset on each call to print()) 
    std::unordered_map<std::string, int> qubit_levels; // varName --> BDD level
    bool needs_high_swap = false; // set when CX with t-above-c is encountered
    int next_group_id = 0; // for generating unique group names when loop variable name is unavailable

    //  Level assignment 
    void        assignLevels(const IR& ir);
    std::string qubitVarName(const RegisterRef& ref, const IR& ir) const;

    //  Top-level section builders 
    nlohmann::ordered_json buildVars() const;
    nlohmann::ordered_json buildDefs() const;
    nlohmann::ordered_json buildOps(const IR& ir);

    /**
     * @brief Dispatch a loop application to a MOSF op group.
     * @param loop The LoopApplication to dispatch.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op group for the loop.
     */
    nlohmann::ordered_json dispatchLoop(const LoopApplication& loop, const IR& ir);

    /**
     * @brief Dispatch a GateApplication to the appropriate MOSF op generator based on gate name and operand count.
     * @param app The GateApplication to dispatch.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the given gate application
     */
    nlohmann::ordered_json dispatchGate(GateApplication app, const IR& ir);

    nlohmann::ordered_json dispatchCond(ConditionalApplication cond, const IR& ir);

    /**
     * @brief Emit a [(multi-)controlled] X gate as a MOSF op tree.
     * Uses "high_swap" definition if needed to handle t-above-c cases.
     * Handles "x", "cx", "ccx", and "mcx" gates based on operand count.
     * 
     * @param app The GateApplication representing the MCX gate.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the MCX gate.
     */
    nlohmann::ordered_json emitMCX(GateApplication app, const IR& ir);

    /**
     * @brief Emit an H gate as a MOSF op tree.
     * 
     * @param app The GateApplication representing the H gate.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the H gate.
     */
    nlohmann::ordered_json emitH(GateApplication app, const IR& ir);

    /**
     * @brief Emit an RX gate as a MOSF op tree.
     * 
     * @param app The GateApplication representing the RX gate.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the RX gate.
     */
    nlohmann::ordered_json emitRX(GateApplication app, const IR& ir);

    /**
     * @brief Emit an RY gate as a MOSF op tree.
     * 
     * @param app The GateApplication representing the RY gate.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the RY gate
     */
    nlohmann::ordered_json emitRY(GateApplication app, const IR& ir);

    /**
     * @brief Emit an RZ gate as a MOSF op tree.
     * 
     * @param app The GateApplication representing the RZ gate.
     * @param ir The IR for looking up gate and register details.
     * @return An ordered_json object representing the MOSF op tree for the RZ gate
     */
    nlohmann::ordered_json emitRZ(GateApplication app, const IR& ir);

    nlohmann::ordered_json emitZ(GateApplication app, const IR& ir);
    nlohmann::ordered_json emitS(GateApplication app, const IR& ir);
    nlohmann::ordered_json emitY(GateApplication app, const IR& ir);
    nlohmann::ordered_json emitT(GateApplication app, const IR& ir);
    nlohmann::ordered_json emitCZ(GateApplication app, const IR& ir);
    nlohmann::ordered_json emitTdg(GateApplication app, const IR& ir);

};

/* EOF MOSFPrinter.hpp */
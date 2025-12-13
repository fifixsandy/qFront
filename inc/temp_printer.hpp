/**
 * This file will be replaced by proper output (i hope lol)
 */


#pragma once
#include "ir.hpp"
void printGatePlacement(const GatePlacement& p, int indentLvl);
void printGate(const GateDef& gate, const IR& ir, int indentLvl);
void printGateTable(const IR& ir);
void printRegisterTable(const IR& ir);
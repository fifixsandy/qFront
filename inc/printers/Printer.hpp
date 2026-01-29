/**
 * @file Printer.hpp
 * @author Filip Novak
 * @date 2026-01-29
 */
#pragma once
#include "ir.hpp"
#include <memory>
#include <string>
#include <ostream>

class Printer {
public:
    virtual ~Printer() = default;

    virtual void print(const IR& ir, std::ostream& out) = 0;
    
    virtual std::string name() const = 0;
    virtual std::string extension() const = 0;
    virtual std::string description() const = 0;
};

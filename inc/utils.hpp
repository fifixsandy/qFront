/**
 * @file utils.hpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#pragma once
#include <string>
#include "qasm3Parser.h"
#include <optional>

namespace parse_utils {
    std::optional<int> tryExtractIntConst(qasm3Parser::ExpressionContext* expr);
}

/**
 * @file utils.cpp
 * @author Filip Novak
 * @date 2025-12-13
 */

#include "../inc/utils.hpp"
#include <cstdlib>
#include <stdexcept>
#include <optional>

namespace parse_utils {
  std::optional<int> tryExtractIntConst(qasm3Parser::ExpressionContext* expr) {
    if (!expr) return std::nullopt;

    auto lit = dynamic_cast<qasm3Parser::LiteralExpressionContext*>(expr);
    if (!lit) return std::nullopt;

    if (auto dec = lit->DecimalIntegerLiteral()) {
        return std::stoi(dec->getText());
    }

    // TODO later: binary, hex, constant expression folding...

    return std::nullopt;
}

}
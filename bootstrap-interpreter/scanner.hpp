#ifndef SYSMEL_BOOTSTRAP_SCANNER
#define SYSMEL_BOOTSTRAP_SCANNER

#include "source.hpp"
#include <vector>
#include <string>

#pragma once

namespace Sysmel
{
/**
 * TokenKind. The different kinds of token used in Sysmel.
 */
enum class TokenKind
{
#define TokenKindName(name) name,
#include "token_kind.inc"
#undef TokenKindName
};

const char *getTokenKindName(TokenKind kind);

struct Token
{
    SourcePositionPtr position;
    TokenKind kind;
    std::string errorMessage;
};
typedef std::shared_ptr<Token> TokenPtr;

std::vector<TokenPtr> scanSourceCode(const SourceCodePtr &sourceCode);

} // End of namespace Sysmel
#endif
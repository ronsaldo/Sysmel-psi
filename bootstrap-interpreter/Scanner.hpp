#ifndef SYSMEL_BOOTSTRAP_SCANNER
#define SYSMEL_BOOTSTRAP_SCANNER

#include "Source.hpp"
#include "Value.hpp"
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
#include "TokenKind.inc"
#undef TokenKindName
};

const char *getTokenKindName(TokenKind kind);

struct Token : public Object
{
    SourcePositionPtr position;
    TokenKind kind;
    std::string errorMessage;

    std::string getValue() const
    {
        return position->getValue();
    }
};
typedef std::shared_ptr<Token> TokenPtr;

std::vector<TokenPtr> scanSourceCode(const SourceCodePtr &sourceCode);

} // End of namespace Sysmel
#endif
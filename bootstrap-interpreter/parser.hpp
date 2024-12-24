#ifndef SYSMEL_PARSER_HPP
#define SYSMEL_PARSER_HPP

#pragma once

#include "scanner.hpp"

namespace Sysmel
{

ValuePtr parseTokens(const SourceCodePtr &sourceCode, const std::vector<TokenPtr> &tokens);

} // End of namespace Sysmel

#endif //SYSMEL_PARSER_HPP
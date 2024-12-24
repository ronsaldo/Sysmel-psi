#ifndef SYSMEL_SOURCE_CODE_HPP
#define SYSMEL_SOURCE_CODE_HPP

#pragma once

#include <stdint.h>
#include <string>
#include <memory>

namespace Sysmel
{
/**
 * Source code. Text plus additional metadata.
 */
struct SourceCode
{
    std::string directory;
    std::string name;
    std::string language;
    std::string text;
};

typedef std::shared_ptr<SourceCode> SourceCodePtr;

struct SourcePosition
{
    SourceCodePtr sourceCode;
    size_t startIndex;
    size_t endIndex;
    size_t startLine;
    size_t startColumn;
    size_t endLine;
    size_t endColumn;
};

typedef std::shared_ptr<SourcePosition> SourcePositionPtr;

} // end of namespace Sysmel

#endif //SYSMEL_SOURCE_CODE_HPP
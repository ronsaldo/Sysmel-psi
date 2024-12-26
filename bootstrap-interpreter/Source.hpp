#ifndef SYSMEL_SOURCE_CODE_HPP
#define SYSMEL_SOURCE_CODE_HPP

#pragma once

#include "Object.hpp"

#include <stdint.h>
#include <string>
#include <memory>
#include <ostream>

namespace Sysmel
{
/**
 * Source code. Text plus additional metadata.
 */
struct SourceCode : public Object
{
    std::string directory;
    std::string name;
    std::string language;
    std::string text;
};

typedef std::shared_ptr<SourceCode> SourceCodePtr;

struct SourcePosition : public Object
{
    SourceCodePtr sourceCode;
    size_t startIndex;
    size_t endIndex;
    size_t startLine;
    size_t startColumn;
    size_t endLine;
    size_t endColumn;

    std::string getValue() const
    {
        return sourceCode->text.substr(startIndex, endIndex - startIndex);
    }

    void formatIn(std::ostream &out)
    {
        out << sourceCode->directory << sourceCode->name
            << ':' << startLine << '.' << startColumn
            << '-' << startLine << '.' << endColumn;
    }

    SourcePositionPtr until(const SourcePositionPtr &endSourcePosition) const
    {
        auto merged = std::make_shared<SourcePosition> ();
        merged->sourceCode = sourceCode;
        
        merged->startIndex  = startIndex;
        merged->startLine   = startLine;
        merged->startColumn = startColumn;

        merged->endIndex  = endSourcePosition->startIndex;
        merged->endLine   = endSourcePosition->startLine;
        merged->endColumn = endSourcePosition->startColumn;
        return merged;
    }

    SourcePositionPtr to(const SourcePositionPtr &endSourcePosition) const
    {
        auto merged = std::make_shared<SourcePosition> ();
        merged->sourceCode = sourceCode;
        
        merged->startIndex  = startIndex;
        merged->startLine   = startLine;
        merged->startColumn = startColumn;

        merged->endIndex  = endSourcePosition->endIndex;
        merged->endLine   = endSourcePosition->endLine;
        merged->endColumn = endSourcePosition->endColumn;
        return merged;
    }
};

typedef std::shared_ptr<SourcePosition> SourcePositionPtr;

} // end of namespace Sysmel

#endif //SYSMEL_SOURCE_CODE_HPP
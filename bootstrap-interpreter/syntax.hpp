#ifndef SYSMEL_SYNTAX_HPP
#define SYSMEL_SYNTAX_HPP

#pragma once

#include "value.hpp"
#include "source.hpp"
#include <vector>
#include <sstream>

namespace Sysmel
{

class SyntacticValue : public Object
{
public:
    virtual bool isSyntacticValue() const override {return true;}
    virtual std::string printString() const override {return "SyntacticValue";}
    virtual SourcePositionPtr getSourcePosition() const override {return sourcePosition;}

    SourcePositionPtr sourcePosition;
};

class SyntaxValueSequence : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxValueSequence(";
        for(size_t i = 0; i < elements.size(); ++i)
        {
            if(i > 0)
                out << ". ";
            out << elements[i]->printString();
        }
        out << ")";
        return out.str();
    }

    std::vector<ValuePtr> elements;
};


class SyntaxError : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxError(" << errorMessage << ")";
        return out.str();
    }

    std::string errorMessage;
};

class SyntaxLiteral : public SyntacticValue
{
public:
};

class SyntaxLiteralFloat : public SyntaxLiteral
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxLiteralFloat(" << value << ")";
        return out.str();
    }

    double value;
};

class SyntaxLiteralInteger : public SyntaxLiteral
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxLiteralInteger(" << value << ")";
        return out.str();
    }

    int64_t value;    
};

} // end of namespace Sysmel

#endif //SYSMEL_SYNTAX_HPP
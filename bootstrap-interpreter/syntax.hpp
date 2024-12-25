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

class SyntaxAssociation : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxAssociation(" << key->printString() << " : " << value->printString() << ")";
        return out.str();
    }

    ValuePtr key;
    ValuePtr value;
};

class SyntaxTuple : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxTuple(";
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
        out << "SyntaxError(" << errorMessage;
        if (innerNode)
            out << ": " << innerNode->printString();
        out << ")";
        return out.str();
    }

    std::string errorMessage;
    ValuePtr innerNode;
};

class SyntaxIdentifierReference : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxIdentifierReference(" << value << ")";
        return out.str();
    }

    std::string value;    
};

class SyntaxFunctionalDependentType : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxFunctionalDependentType(";
        if (argumentPattern)
            out << argumentPattern->printString();
        if (resultType)
            out << " :: " <<  resultType->printString();
        return out.str();
    }

    ValuePtr argumentPattern;
    ValuePtr resultType;
};

typedef std::shared_ptr<SyntaxFunctionalDependentType> SyntaxFunctionalDependentTypePtr;

class SyntaxAssignment : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxAssignment(" << store->printString() << " := " << value->printString() << ")";
        return out.str();
    }

    ValuePtr store;
    ValuePtr value;
};

class SyntaxBindPattern : public SyntacticValue
{
public:
    ValuePtr pattern;
    ValuePtr value;
};

class SyntaxBlock : public SyntacticValue
{
public:
    ValuePtr functionType;
    ValuePtr body;
};

class SyntaxLexicalBlock : public SyntacticValue
{
public:
    ValuePtr body;
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

class SyntaxLiteralCharacter : public SyntaxLiteral
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxLiteralCharacter(" << value << ")";
        return out.str();
    }

    char32_t value;
};

class SyntaxLiteralString : public SyntaxLiteral
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxLiteralString(" << value << ")";
        return out.str();
    }

    std::string value;    
};

class SyntaxLiteralSymbol : public SyntaxLiteral
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxLiteralSymbol(" << value << ")";
        return out.str();
    }

    std::string value;    
};

class SyntaxBinaryExpressionSequence : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxBinaryExpressionSequence(";
        bool isFirst = true;
        for(auto &element : elements)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            out << element->printString();
        }
        out << ")";
        return out.str();
    }

    std::vector<ValuePtr> elements;
};

class SyntaxMessageSend : public SyntacticValue
{
public:
    virtual std::string printString() const override
    {
        std::ostringstream out;
        out << "SyntaxMessageSend(";
        if(receiver)
            out << receiver->printString() << ",";
        out << selector->printString();
        for(auto &argument : arguments)
        {
            out << ", " << argument->printString();
        }
        out << ")";
        return out.str();
    }

    ValuePtr receiver;
    ValuePtr selector;
    std::vector<ValuePtr> arguments;
};

} // end of namespace Sysmel

#endif //SYSMEL_SYNTAX_HPP
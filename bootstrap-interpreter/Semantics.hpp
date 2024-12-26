#ifndef SYSMEL_SEMANTICS_HPP
#define SYSMEL_SEMANTICS_HPP

#pragma once

#include "Object.hpp"

namespace Sysmel
{
class SemanticValue : public Object
{
public:
    virtual bool isSemanticValue() const override { return true;}
    virtual void printStringOn(std::ostream &out) const override {out << "SemanticValue";}
    virtual SourcePositionPtr getSourcePosition() const override {return sourcePosition;}
    virtual ValuePtr getType() const override {return type;}

    SourcePositionPtr sourcePosition;
    ValuePtr type;
};

class SemanticMessageSend : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "MessageSend(";
        if(receiver)
        {
            receiver->printStringOn(out);
            out << ",";
        }
            
        selector->printStringOn(out);
        for(auto &argument : arguments)
        {
            out << ", ";
            argument->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(receiver)
        {
            function(receiver);
            receiver->traverseChildren(function);
        }
        if(selector)
        {
            function(selector);
            selector->traverseChildren(function);
        }
        for(const auto &argument : arguments)
        {
            function(argument);
            argument->traverseChildren(function);
        }
    }

    ValuePtr receiver;
    ValuePtr selector;
    std::vector<ValuePtr> arguments;
};

class SemanticLiteralValue : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override {
        out << "SemanticLiteralValue(";
        value->printStringOn(out);
        out << ")";
    }

    virtual SymbolPtr asAnalyzedSymbolValue() override 
    {
        return value->asAnalyzedSymbolValue(); 
    }

    ValuePtr value;
};

} // End of namespace Sysmel

#endif //SYSMEL_SEMANTICS_HPP
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
    virtual ValuePtr getType() const override {return type;}

    ValuePtr type;
};

class SemanticValueSequence : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticValueSequence(";
        for(size_t i = 0; i < elements.size(); ++i)
        {
            if(i > 0)
                out << ". ";
            elements[i]->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        for(auto &element : elements)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        ValuePtr result = UndefinedObject::uniqueInstance();
        for(auto &element : elements)
            result = element->evaluateInEnvironment(environment);
        return result;
    }

    std::vector<ValuePtr> elements;
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

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto receiverValue = receiver->evaluateInEnvironment(environment);
        auto selectorValue = selector->evaluateInEnvironment(environment);
        std::vector<ValuePtr> argumentValues;
        argumentValues.reserve(arguments.size());
        for (auto& arg: arguments)
        {
            auto evaluatedArgument = arg->evaluateInEnvironment(environment);
            argumentValues.push_back(evaluatedArgument);
        }

        return receiverValue->performWithArguments(selectorValue, argumentValues);
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

    virtual ValuePtr getType() const override
    {
        return value->getType();
    }

    virtual ValuePtr getClass() const override
    {
        return value->getClass();
    }

    virtual ValuePtr getTypeOrClass() const override
    {
        return value->getTypeOrClass();
    }

    virtual SymbolPtr asAnalyzedSymbolValue() override 
    {
        return value->asAnalyzedSymbolValue(); 
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        return value;
    }

    ValuePtr value;
};

} // End of namespace Sysmel

#endif //SYSMEL_SEMANTICS_HPP
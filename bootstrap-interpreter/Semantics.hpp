#ifndef SYSMEL_SEMANTICS_HPP
#define SYSMEL_SEMANTICS_HPP

#pragma once

#include "Object.hpp"
#include "Environment.hpp"

namespace Sysmel
{
class SemanticValue : public Object
{
public:
    virtual const char *getClassName() const { return "SemanticValue"; }
    
    virtual bool isSemanticValue() const override { return true;}
    virtual void printStringOn(std::ostream &out) const override {out << "SemanticValue";}
    virtual ValuePtr getType() const override {return type;}
    virtual ValuePtr getTypeOrClass() const override
    {
        if(type)
            return type;
        return getClass();
    }
    
    ValuePtr type;
};

class SemanticValueSequence : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticValueSequence"; }

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
    virtual const char *getClassName() const { return "SemanticMessageSend"; }

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

class SemanticArgumentNode : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticArgumentNode"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticArgumentNode(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        abort();
    }

    SymbolArgumentBindingPtr binding;
    bool isImplicit = false;
    bool isExistential = false;
};

class SemanticFunctionalValue : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticFunctionalValue"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticFunctionalValue(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        abort();
    }

    SymbolPtr name;
    EnvironmentPtr closure;
    std::vector<SymbolArgumentBindingPtr> argumentBindings;
    bool isVariadic = false;
    std::vector<SymbolCaptureBindingPtr> captureBindings;
    ValuePtr body;
};

class SemanticLambda : public SemanticFunctionalValue
{
public:
    virtual const char *getClassName() const { return "SemanticLambda"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticLambda(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        abort();
    }
};

class SemanticPi : public SemanticFunctionalValue
{
public:
    virtual const char *getClassName() const { return "SemanticPi"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticPiValue(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        auto pi = std::make_shared<PiType> ();
        pi->nameExpression = name;
        pi->arguments = argumentBindings;
        pi->resultType = body;
        return pi;
    }
};

class SemanticSigma : public SemanticFunctionalValue
{
public:
    virtual const char *getClassName() const { return "SemanticSigma"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticSigma(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        // TODO: Implement this
        (void)environment;
        abort();
    }
};

class SemanticLiteralValue : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticLiteralValue"; }

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

    virtual ValuePtr getClassOrType() const override
    {
        return value->getClassOrType();
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

class SemanticArray : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticArray"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticArray(";
        bool isFirst = true;
        for(auto &expression : expressions)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ". ";
            expression->printStringOn(out);
        }
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto arrayObject = std::make_shared<Array> ();
        arrayObject->values.reserve(expressions.size());

        for (const auto &expression: expressions)
        {
            auto expressionValue = expression->evaluateInEnvironment(environment);
            arrayObject->values.push_back(expressionValue);
        }

        return arrayObject;
    }

    std::vector<ValuePtr> expressions;
};

class SemanticTuple : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticTuple"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticTuple(";
        bool isFirst = true;
        for(auto &expression : expressions)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ". ";
            expression->printStringOn(out);
        }
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto tupleObject = std::make_shared<ProductTypeValue> ();
        tupleObject->type = std::static_pointer_cast<ProductType> (type);
        tupleObject->elements.reserve(expressions.size());

        for (const auto &expression: expressions)
        {
            auto expressionValue = expression->evaluateInEnvironment(environment);
            tupleObject->elements.push_back(expressionValue);
        }

        return tupleObject;
    }

    std::vector<ValuePtr> expressions;
};

class SemanticByteArray : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticByteArray"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticByteArray(";
        bool isFirst = true;
        for(auto &byte : byteExpressions)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ". ";
            byte->printStringOn(out);
        }
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto byteArrayObject = std::make_shared<ByteArray> ();
        byteArrayObject->values.reserve(byteExpressions.size());

        for (const auto &byteExpression: byteExpressions)
        {
            auto byteExpressionValue = byteExpression->evaluateInEnvironment(environment);
            byteArrayObject->values.push_back(byteExpressionValue->evaluateAsSingleByte());
        }

        return byteArrayObject;
    }

    std::vector<ValuePtr> byteExpressions;
};

class SemanticIdentifierReference : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticIdentifierReference"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticIdentifierReference(";
        identifierBinding->printStringOn(out);
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        abort();
    }

    ValuePtr identifierBinding;
};
} // End of namespace Sysmel

#endif //SYSMEL_SEMANTICS_HPP
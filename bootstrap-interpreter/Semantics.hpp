#ifndef SYSMEL_SEMANTICS_HPP
#define SYSMEL_SEMANTICS_HPP

#pragma once

#include "Object.hpp"
#include "Environment.hpp"
#include "Assert.hpp"
#include "Type.hpp"

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

class SemanticApplication : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticApplication"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "MessageSend(";
        if(functional)
        {
            functional->printStringOn(out);
        }
        for(auto &argument : arguments)
        {
            out << ", ";
            argument->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(functional)
        {
            function(functional);
            functional->traverseChildren(function);
        }

        for(const auto &argument : arguments)
        {
            function(argument);
            argument->traverseChildren(function);
        }
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto functionalValue = functional->evaluateInEnvironment(environment);
        if(functionalValue->isMacro())
            throwExceptionWithMessage("Macro methods have to evaluated during syntactic translation.");
        std::vector<ValuePtr> argumentValues;
        argumentValues.reserve(arguments.size());
        for(auto &argument : arguments)
        {
            auto argumentValue = argument->evaluateInEnvironment(environment);
            argumentValues.push_back(argumentValue);
        }

        auto result = functionalValue->applyWithArguments(argumentValues);
        return result;
    }

    ValuePtr functional;
    std::vector<ValuePtr> arguments;
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

class SemanticSimpleFunctionType : public SemanticValue
{
public:
    virtual const char *getClassName() const { return "SemanticSimpleFunctionType"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticSimpleFunctionType(";
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        if(simpleFunctionTypeCache)
            return simpleFunctionTypeCache;

        auto simpleFunctionType = std::make_shared<SimpleFunctionType> ();
        simpleFunctionType->argumentTypes = argumentTypes;
        simpleFunctionType->argumentNames = argumentNames;
        simpleFunctionType->resultType = resultType;
        return simpleFunctionTypeCache = simpleFunctionType;
    }

    virtual ArgumentTypeAnalysisContextPtr createArgumentTypeAnalysisContext()
    {
        return evaluateInEnvironment(nullptr)->createArgumentTypeAnalysisContext();
    }

    std::vector<ValuePtr> argumentTypes;
    std::vector<SymbolPtr> argumentNames;
    ValuePtr resultType;
    SimpleFunctionTypePtr simpleFunctionTypeCache;
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
    SymbolFixpointBindingPtr fixpointBinding;
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
        (void)environment;
        auto lambdaValue = std::make_shared<LambdaValue> ();
        lambdaValue->name = name;
        lambdaValue->type = type;
        lambdaValue->closure = closure;
        lambdaValue->body = body;
        lambdaValue->argumentBindings = argumentBindings;
        lambdaValue->fixpointBinding = fixpointBinding;
    
        return lambdaValue;
    }
};

class SemanticPi : public SemanticFunctionalValue
{
public:
    virtual const char *getClassName() const { return "SemanticPi"; }

    virtual ArgumentTypeAnalysisContextPtr createArgumentTypeAnalysisContext() override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticPiValue(";
        if(name)
        {
            name->printStringOn(out);
            out << ' ';  
        }
        out << '(';
        bool isFirst = true;
        for(auto &binding : argumentBindings)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            binding->printStringOn(out);
        }
        out << ") => ";
        body->printStringOn(out);
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

    virtual ValuePtr reduce()
    {
        std::vector<ValuePtr> argumentTypes;
        std::vector<SymbolPtr> argumentNames;
        argumentTypes.reserve(argumentBindings.size());
        for(auto &binding : argumentBindings)
        {
            auto argType = binding->getType()->asTypeValue();
            if(!argType)
                return shared_from_this();

            argumentTypes.push_back(argType);
            argumentNames.push_back(binding->name);
        }

        ValuePtr resultType = body->asTypeValue();
        if(!resultType)
            return shared_from_this();

        auto reducedType = std::make_shared<SemanticSimpleFunctionType> ();
        reducedType->argumentTypes = argumentTypes;
        reducedType->argumentNames = argumentNames;
        reducedType->resultType = resultType;
        return reducedType;
    }
};

typedef std::shared_ptr<SemanticPi> SemanticPiPtr;

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

    virtual ValuePtr asTypeValue() override { return value->asTypeValue(); }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        (void)environment;
        return value;
    }

    virtual bool isMacro() const override
    {
        return value->isMacro();
    }

    virtual ValuePtr applyMacroWithContextAndArguments(const MacroContextPtr &context, const std::vector<ValuePtr> &arguments) override
    {
        return value->applyMacroWithContextAndArguments(context, arguments);
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
        auto foundValue = environment->lookupValueForBinding(identifierBinding);
        if(!foundValue)
            throwExceptionWithMessage("Failed to find value for binding");
        
        return foundValue;
    }

    ValuePtr identifierBinding;
};

class MutableValueBox : public Value
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "MutableValueBox(";
        if(value)
        {
            value->printStringOn(out);
            out << ", ";
        }
        valueType->printStringOn(out);
        out << ", ";
        type->printStringOn(out);
        out << ")";
    }

    virtual void mutableStoreValue(const ValuePtr &valueToAssign) override
    {
        value = valueToAssign;
    }
    
    virtual ValuePtr mutableLoadValue() override
    {
        return value;
    }

    ValuePtr value;
    ValuePtr valueType;
    ValuePtr type;
};

class SemanticAlloca : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticAlloca([";
        valueType->printStringOn(out);
        out << "]";
        type->printStringOn(out);
        out << ")";
        if(initialValueExpression)
        {
            out << " := ";
            initialValueExpression->printStringOn(out);
        }
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto box = std::make_shared<MutableValueBox> ();
        box->valueType = valueType->evaluateInEnvironment(environment);
        box->type = type->evaluateInEnvironment(environment);
        if(initialValueExpression)
            box->value = initialValueExpression->evaluateInEnvironment(environment);
        return box;
    }

    ValuePtr initialValueExpression;
    ValuePtr valueType;
    //ValuePtr type;
};

class SemanticLoadValue : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticLoadValue([";
        pointer->printStringOn(out);
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto evalPointer = pointer->evaluateInEnvironment(environment);
        auto evalValue = evalPointer->mutableLoadValue();
        return evalValue;
    }

    ValuePtr pointer;
};

class SemanticStoreValue : public SemanticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SemanticStoreValue([";
        pointer->printStringOn(out);
        out << " := ";
        value->printStringOn(out);
        out << ")";
    }

    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto evalValue = value->evaluateInEnvironment(environment);
        auto evalReference = pointer->evaluateInEnvironment(environment);
        evalReference->mutableStoreValue(evalValue);
        return evalValue;
    }

    ValuePtr pointer;
    ValuePtr value;
};

class SemanticIf : public SemanticValue
{
public:
    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        ValuePtr resultValue = VoidValue::uniqueInstance();
        auto conditionValue = condition->evaluateInEnvironment(environment);
        
        ValuePtr caseToEvaluate;
        if(conditionValue->isTrue())
            caseToEvaluate = trueCase;
        else if(conditionValue->isFalse())
            caseToEvaluate = falseCase;

        if(caseToEvaluate)
        {
            auto caseResult = caseToEvaluate->evaluateInEnvironment(environment);
            resultValue = caseResult;
        }
        else
        {
            sysmelAssert(!returnsValue);
        }
        
        return resultValue;
    }

    bool returnsValue = false;
    ValuePtr condition;
    ValuePtr trueCase;
    ValuePtr falseCase;
};

class SemanticWhile : public SemanticValue
{
public:
    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment) override
    {
        auto conditionValue = condition->evaluateInEnvironment(environment);
        while(conditionValue->isTrue())
        {
            if(body)
                body->evaluateInEnvironment(environment);
            if(continueAction)
                continueAction->evaluateInEnvironment(environment);
        }

        return VoidValue::uniqueInstance();
    }

    ValuePtr condition;
    ValuePtr body;
    ValuePtr continueAction;
};

} // End of namespace Sysmel

#endif //SYSMEL_SEMANTICS_HPP
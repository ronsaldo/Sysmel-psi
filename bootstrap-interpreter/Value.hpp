#ifndef SYSMEL_VALUE_HPP
#define SYSMEL_VALUE_HPP

#pragma once

#include "Source.hpp"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <sstream>

namespace Sysmel
{

typedef std::shared_ptr<class Value> ValuePtr;
typedef std::shared_ptr<class Class> ClassPtr;
typedef std::weak_ptr<class Class> ClassWeakPtr;
typedef std::shared_ptr<class Type> TypePtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;
typedef std::shared_ptr<class Environment> EnvironmentPtr;
typedef std::shared_ptr<class SyntacticValue> SyntacticValuePtr;
typedef std::shared_ptr<class SourcePosition> SourcePositionPtr; 
typedef std::shared_ptr<class SyntaxError> SyntaxErrorPtr; 
typedef std::shared_ptr<class SyntaxMessageCascade> SyntaxMessageCascadePtr;

class Value : public std::enable_shared_from_this<Value>
{
public:
    virtual ValuePtr getType() const;
    virtual ValuePtr getClass() const;
    virtual ValuePtr getTypeOrClass() const;

    virtual ValuePtr performWithArguments(const ValuePtr &selector, const std::vector<ValuePtr> &arguments);
    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments);
    virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments);
    virtual ValuePtr lookupSelector(const ValuePtr &selector);

    virtual bool isType() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isTypeOrClass() const { return isType() || isClass(); }

    virtual bool isObject() const { return false; }
    virtual bool isEnvironment() const { return false; }
    virtual bool isSemanticValue() const { return false; }
    virtual bool isSyntacticValue() const { return false; }
    virtual bool isSyntaxError() const { return false; }
    virtual bool isBindableName() const { return false; }
    virtual bool isNil() const { return false; };

    virtual SymbolPtr asAnalyzedSymbolValue() { return nullptr; } 

    virtual ValuePtr analyzeInEnvironment(const EnvironmentPtr &environment);
    virtual ValuePtr analyzeInEnvironmentForMacroExpansionOnly(const EnvironmentPtr &environment);
    virtual ValuePtr evaluateInEnvironment(const EnvironmentPtr &environment);
    virtual ValuePtr analyzeAndEvaluateInEnvironment(const EnvironmentPtr &environment);
    virtual ValuePtr analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment);

    virtual bool isSatisfiedByType(const ValuePtr &sourceType);
    virtual ValuePtr coerceIntoExpectedTypeAt(const ValuePtr &targetType, const SourcePositionPtr &coercionLocation);
    virtual bool isSubclassOf(const ValuePtr &targetSuperclass);
    virtual bool isSubtypeOf(const ValuePtr &targetSupertype);
    
    virtual uint8_t evaluateAsSingleByte() 
    {
        throwExceptionWithMessage("Invalid expression for evaluating a single byte.");
    }

    virtual size_t evaluateAsIndex()
    {
        throwExceptionWithMessage("Invalid expression for evaluating an index.");
    }
    
    virtual ValuePtr getElementAtIndex(size_t index)
    {
        (void)index;
        throwExceptionWithMessage("Object is not a container of indexed values.");
    }

    virtual ValuePtr setElementAtIndex(size_t index, const ValuePtr &value)
    {
        (void)index;
        (void)value;
        throwExceptionWithMessage("Object is not a container of indexed values.");
    }

    virtual std::pair<size_t, const uint8_t*> getBinaryContentsData() const
    {
        return std::make_pair(0, nullptr);
    }

    virtual void printStringOn(std::ostream &out) const
    {
        out << "a Value";
    }

    virtual std::string printString() const 
    {
        std::ostringstream out;
        printStringOn(out);
        return out.str();
    }

    virtual SourcePositionPtr getSourcePosition() const
    {
        return sourcePosition;
    }

    [[noreturn]] void throwExceptionWithMessage(const char *message);
    [[noreturn]] void throwExceptionWithMessageAt(const char *message, const SourcePositionPtr &position);
    
    virtual SyntaxMessageCascadePtr asMessageCascade() const { return nullptr; }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const
    {
        (void)function;
    }

    std::vector<SyntaxErrorPtr> collectSyntaxErrors()
    {
        std::vector<SyntaxErrorPtr> errors;
        if(isSyntaxError())
            errors.push_back(std::static_pointer_cast<SyntaxError> (shared_from_this()));

        traverseChildren([&](ValuePtr value){
            if(value->isSyntaxError())
                errors.push_back(std::static_pointer_cast<SyntaxError> (value));
        });
        return errors;
    }

    SourcePositionPtr sourcePosition;
};




}
#endif //SYSMEL_VALUE
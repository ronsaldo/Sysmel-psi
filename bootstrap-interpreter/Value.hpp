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
typedef std::shared_ptr<class Type> TypePtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class SyntacticValue> SyntacticValuePtr;
typedef std::shared_ptr<class SourcePosition> SourcePositionPtr; 
typedef std::shared_ptr<class SyntaxError> SyntaxErrorPtr; 
typedef std::shared_ptr<class SyntaxMessageCascade> SyntaxMessageCascadePtr;

class Value : public std::enable_shared_from_this<Value>
{
public:
    virtual ValuePtr getType() const;
    virtual ValuePtr getClass() const;

    virtual ValuePtr performWithArguments(const ValuePtr &selector, const std::vector<ValuePtr> &arguments);
    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments);
    virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments);
    virtual ValuePtr lookupSelector(const ValuePtr &selector);

    virtual bool isType() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isObject() const { return false; }
    virtual bool isEnvironment() const { return false; }
    virtual bool isSyntacticValue() const { return false; }
    virtual bool isSyntaxError() const { return false; }

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

    virtual SourcePositionPtr getSourcePosition() const {return nullptr;}
    [[noreturn]] void throwExceptionWithMessage(const char *message);
    
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
};




}
#endif //SYSMEL_VALUE
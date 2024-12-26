#ifndef SYSMEL_VALUE_HPP
#define SYSMEL_VALUE_HPP

#pragma once

#include "Source.hpp"
#include <memory>
#include <string>
#include <functional>

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
    virtual ValuePtr getType();
    virtual ValuePtr getClass() const;

    virtual bool isType() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isObject() const { return false; }
    virtual bool isSyntacticValue() const { return false; }
    virtual bool isSyntaxError() const { return false; }

    virtual std::string printString() const {return "a Value";}
    virtual SourcePositionPtr getSourcePosition() const {return nullptr;}
    
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
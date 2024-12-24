#ifndef SYSMEL_VALUE_HPP
#define SYSMEL_VALUE_HPP

#pragma once

#include "source.hpp"
#include <memory>
#include <string>

namespace Sysmel
{

typedef std::shared_ptr<class Value> ValuePtr;
typedef std::shared_ptr<class Type> TypePtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class SyntacticValue> SyntacticValuePtr;
typedef std::shared_ptr<class SourcePosition> SourcePositionPtr; 

class Value
{
public:
    virtual bool isType() const { return false; }
    virtual bool isObject() const { return false; }
    virtual bool isSyntacticValue() const { return false; }

    virtual std::string printString() const {return "SomeValue";}
    virtual SourcePositionPtr getSourcePosition() const {return nullptr;}
};

class Type : public Value
{
public:
};

class Object : public Value
{
public:
};


}
#endif //SYSMEL_VALUE
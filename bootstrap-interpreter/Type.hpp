#ifndef SYSMEL_TYPE_HPP
#define SYSMEL_TYPE_HPP

#include "Value.hpp"
#include <sstream>
#include <vector>

namespace Sysmel
{

typedef std::shared_ptr<class TypeUniverse> TypeUniversePtr;
typedef std::shared_ptr<class GradualType> GradualTypePtr;
typedef std::shared_ptr<class UnitType> UnitTypePtr;
typedef std::shared_ptr<class BottomType> BottomTypePtr;
typedef std::shared_ptr<class ProductType> ProductTypePtr;
typedef std::shared_ptr<class SumType> SumTypePtr;
typedef std::shared_ptr<class FunctionType> FunctionTypePtr;
typedef std::shared_ptr<class ObjectType> ObjectTypePtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;

class TypeBehavior : public Value
{
public:
    virtual ValuePtr lookupSelector(const ValuePtr &selector) override
    {
        auto it = methodDict.find(selector);
        return it != methodDict.end() ? it->second : nullptr;
    }

    std::map<ValuePtr, ValuePtr> methodDict;
};

class TypeUniverse : public TypeBehavior
{
public:
    TypeUniverse(int index = 0)
        : universeIndex(index) {}

    virtual bool isType() const override { return true; }

    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Type(" << universeIndex << ")";
    }

    int universeIndex = 0;

    static TypeUniversePtr uniqueInstanceForIndex(int index);

private:
    static std::vector<TypeUniversePtr> uniqueInstances;
};

class Type : public TypeBehavior
{
public:
    virtual bool isType() const override { return true; }
    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Type";
    }

    static TypePtr uniqueInstance();
};

class BasicType : public TypeBehavior
{
public:
    virtual ValuePtr getType() const override;

};

class UnitType : public BasicType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Unit";
    }

    static UnitTypePtr uniqueInstance();
private:
    static UnitTypePtr singletonValue;
};

class BottomType : public BasicType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Bottom";
    }

    static BottomTypePtr uniqueInstance();
private:
    static BottomTypePtr singletonValue;
};

class GradualType : public BasicType
{
public:
    virtual bool isType() const override { return true; }
    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "?";
    }
    
    static GradualTypePtr uniqueInstance();
private:
    static GradualTypePtr singletonValue;
};


}// End of namespace Sysmel


#endif
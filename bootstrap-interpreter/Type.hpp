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
typedef std::shared_ptr<class VoidType> VoidTypePtr;
typedef std::shared_ptr<class ProductType> ProductTypePtr;
typedef std::shared_ptr<class SumType> SumTypePtr;
typedef std::shared_ptr<class FunctionType> FunctionTypePtr;
typedef std::shared_ptr<class ObjectType> ObjectTypePtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;

class TypeBehavior : public Value
{
public:
    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments) override
    {
        auto method = lookupSelector(selector);
        if (!method)
        {
            receiver->throwExceptionWithMessage(("Failed to find method " + selector->printString() + " in " + receiver->getType()->printString()).c_str());
        }

        std::vector<ValuePtr> allArguments;
        allArguments.reserve(1 + arguments.size());
        allArguments.push_back(receiver);
        for (auto &arg : arguments)
            allArguments.push_back(arg);

        return method->applyWithArguments(allArguments);
    }

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

private:
    static TypePtr singletonValue;
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
        out << "UnitType";
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
        out << "BottomType";
    }

    static BottomTypePtr uniqueInstance();
private:
    static BottomTypePtr singletonValue;
};

class VoidType : public BasicType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Void";
    }

    static VoidTypePtr uniqueInstance();
private:
    static VoidTypePtr singletonValue;
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

class ProductTypeValue : public Value
{
public:
    virtual ValuePtr getType() const override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "(";
        bool isFirst = true;
        for (const auto &element : elements)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            element->printStringOn(out);
        }
        out << ")";
    }
    
    ProductTypePtr type;
    std::vector<ValuePtr> elements;
};

class ProductType : public TypeBehavior
{
public:
    static ProductTypePtr getOrCreateWithElementTypes(const std::vector<ValuePtr> &elements);

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '(';
        bool isFirst = true;
        for(auto &element : elementTypes)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            element->printStringOn(out);
        }
        out << ')';
    }

    std::vector<ValuePtr> elementTypes;
private:
    static std::map<std::vector<ValuePtr>, ProductTypePtr> ProductTypeCache;
};

class SumTypeValue : public Value
{
public:
    virtual ValuePtr getType() const override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "(" << caseIndex << ": ";
        element->printStringOn(out);
        out << ")";
    }
    
    SumTypePtr type;
    uint32_t caseIndex;
    ValuePtr element;
};

class SumType : public TypeBehavior
{
public:
    static SumTypePtr getOrCreateWithAlternativeTypes(const std::vector<ValuePtr> &elements);

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '(';
        bool isFirst = true;
        for(auto &alternative : alternativeTypes)
        {
            if(isFirst)
                isFirst = false;
            else
                out << " | ";
            alternative->printStringOn(out);
        }
        out << ')';
    }

    std::vector<ValuePtr> alternativeTypes;
private:
    static std::map<std::vector<ValuePtr>, SumTypePtr> SumTypeCache;
};

class PiType : public TypeBehavior
{
public:
    virtual void printStringOn(std::ostream &out) const override;

    ValuePtr nameExpression;
    std::vector<SymbolArgumentBindingPtr> arguments;
    ValuePtr resultType;
};

}// End of namespace Sysmel


#endif
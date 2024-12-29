#include "Type.hpp"
#include "Environment.hpp"

namespace Sysmel
{

// Type Universe
ValuePtr TypeUniverse::getType()
{
    return uniqueInstanceForIndex(universeIndex + 1);
}

std::vector<TypeUniversePtr> TypeUniverse::uniqueInstances;

TypeUniversePtr TypeUniverse::uniqueInstanceForIndex(int index)
{
    for(int currentIndex = 0; currentIndex <= index; ++currentIndex)
        uniqueInstances.push_back(std::make_shared<TypeUniverse> (currentIndex));
    return uniqueInstances[index];
}

// Generic type that ignores the universe index
ValuePtr Type::getType()
{
    return std::static_pointer_cast<Type> (shared_from_this());
}

// Gradual type
ValuePtr GradualType::getType()
{
    return uniqueInstance();
}

GradualTypePtr GradualType::singletonValue;
GradualTypePtr GradualType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<GradualType> ();
    return singletonValue;
}

// BasicType
ValuePtr BasicType::getType() const
{
    return TypeUniverse::uniqueInstanceForIndex(0);
}

// Type
TypePtr Type::singletonValue;
TypePtr Type::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<Type> ();
    return singletonValue;
}

// Unit type
UnitTypePtr UnitType::singletonValue;
UnitTypePtr UnitType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<UnitType> ();
    return singletonValue;
}

// Bottom type
BottomTypePtr BottomType::singletonValue;
BottomTypePtr BottomType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<BottomType> ();
    return singletonValue;
}

// Void type
VoidTypePtr VoidType::singletonValue;
VoidTypePtr VoidType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<VoidType> ();
    return singletonValue;
}

// Void value
ValuePtr VoidValue::getType() const
{
    return VoidType::uniqueInstance();
}

VoidValuePtr VoidValue::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<VoidValue> ();
    return singletonValue;
}

VoidValuePtr VoidValue::singletonValue;

// Product type
ValuePtr ProductTypeValue::getType() const
{
    return type;
}

ProductTypePtr ProductType::getOrCreateWithElementTypes(const std::vector<ValuePtr> &elements)
{
    auto it = ProductTypeCache.find(elements);
    if(it != ProductTypeCache.end())
        return it->second;

    auto newProductType = std::make_shared<ProductType> ();
    newProductType->elementTypes = elements;
    ProductTypeCache.insert(std::make_pair(elements, newProductType));
    return newProductType;
}

std::map<std::vector<ValuePtr>, ProductTypePtr> ProductType::ProductTypeCache;

// Sum type
ValuePtr SumTypeValue::getType() const
{
    return type;
}

SumTypePtr SumType::getOrCreateWithAlternativeTypes(const std::vector<ValuePtr> &alternatives)
{
    auto it = SumTypeCache.find(alternatives);
    if(it != SumTypeCache.end())
        return it->second;

    auto newSumType = std::make_shared<SumType> ();
    newSumType->alternativeTypes = alternatives;
    SumTypeCache.insert(std::make_pair(alternatives, newSumType));
    return newSumType;
}

std::map<std::vector<ValuePtr>, SumTypePtr> SumType::SumTypeCache;

void PiType::printStringOn(std::ostream &out) const
{
    out << "Pi(";
    bool isFirst = true;
    for(auto &argument : arguments)
    {
        if(isFirst)
            isFirst = false;
        else
            out << ", ";
        argument->printStringOn(out);
    }
    out << "): ";
    if(resultType)
        resultType->printStringOn(out);
}

ValuePtr PiType::reduce()
{
    return shared_from_this();
}

void SimpleFunctionType::printStringOn(std::ostream &out) const
{
    out << '(';
    for(size_t i = 0; i < argumentTypes.size(); ++i)
    {
        if(i > 0)
            out << ", ";
        out << ":(";
        argumentTypes[i]->printStringOn(out);
        out << ")";
        argumentNames[i]->printStringOn(out);
    }

    out << ") =>";
    resultType->printStringOn(out);
}

ArgumentTypeAnalysisContextPtr SimpleFunctionType::createArgumentTypeAnalysisContext()
{
    auto context = std::make_shared<SimpleFunctionArgumentTypeAnalysisContext> ();
    context->simpleFunctionalType = std::static_pointer_cast<SimpleFunctionType> (shared_from_this());
    return context;
}
ValuePtr SimpleFunctionArgumentTypeAnalysisContext::coerceArgumentWithIndex(size_t index, ValuePtr argument)
{
    return argument->coerceIntoExpectedTypeAt(simpleFunctionalType->argumentTypes[index], argument->getSourcePosition());
}

ValuePtr SimpleFunctionArgumentTypeAnalysisContext::getResultType()
{
    return simpleFunctionalType->resultType;
}

} //end of namespace Sysmel

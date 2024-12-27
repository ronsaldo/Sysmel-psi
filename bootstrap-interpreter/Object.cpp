#include "Object.hpp"
#include "Environment.hpp"
namespace Sysmel
{

ValuePtr ProtoObject::getClass() const
{
    if(!clazz)
        clazz = IntrinsicsEnvironment::uniqueInstance()->lookupValidClass(getClassName());
    return clazz;
}

UndefinedObjectPtr UndefinedObject::uniqueInstance()
{
    if(!singleton)
        singleton = std::make_shared<UndefinedObject> ();
    return singleton;
}

UndefinedObjectPtr UndefinedObject::singleton;

SymbolPtr Symbol::internString(const std::string &string)
{
    auto it = internedSymbols.find(string);
    if (it != internedSymbols.end())
        return it->second;

    auto newSymbol = std::make_shared<Symbol> ();
    newSymbol->clazz = IntrinsicsEnvironment::uniqueInstance()->lookupValidClass("Symbol");
    newSymbol->value = string;
    internedSymbols.insert(std::make_pair(string, newSymbol));
    return newSymbol;
}

std::map<std::string, SymbolPtr> Symbol::internedSymbols;

ValuePtr PrimitiveMethod::applyWithArguments(const std::vector<ValuePtr> &arguments)
{
    return implementation(arguments);
}

} // End of namespace

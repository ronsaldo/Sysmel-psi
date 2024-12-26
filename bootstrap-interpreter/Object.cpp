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

SymbolPtr Symbol::internString(const std::string &string)
{
    auto it = internedSymbols.find(string);
    if (it != internedSymbols.end())
        return it->second;

    auto newSymbol = std::make_shared<Symbol> ();
    newSymbol->clazz = IntrinsicsEnvironment::uniqueInstance()->lookupValidClass("Symbol");
    newSymbol->value = string;
    return newSymbol;
}

std::map<std::string, SymbolPtr> Symbol::internedSymbols;

} // End of namespace

#include "Object.hpp"
#include "Environment.hpp"
#include "stdio.h"

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

ValuePtr Boolean::encode(bool value)
{
    if (value)
        return True::uniqueInstance();
    else
        return False::uniqueInstance();
}

TruePtr True::uniqueInstance()
{
    if(!singleton)
        singleton = std::make_shared<True> ();
    return singleton;
}

TruePtr True::singleton;

FalsePtr False::uniqueInstance()
{
    if(!singleton)
        singleton = std::make_shared<False> ();
    return singleton;
}

FalsePtr False::singleton;

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

ValuePtr PrimitiveMacroMethod::applyMacroWithContextAndArguments(const MacroContextPtr &context, const std::vector<ValuePtr> &arguments)
{
    return implementation(context, arguments);
}

void Class::addSubclass(const ValuePtr &subclass)
{
    subclasses = subclasses->copyWith(subclass);
}

BinaryFileStreamPtr Stdio::getValidStdinStream()
{
    if (!stdinStream)
    {
        stdinStream = std::make_shared<BinaryFileStream> ();
        stdinStream->file = stdin;
        stdinStream->ownsFile = false;
    }

    return stdinStream;
}
BinaryFileStreamPtr Stdio::getValidStdoutStream()
{
    if (!stdoutStream)
    {
        stdoutStream = std::make_shared<BinaryFileStream> ();
        stdoutStream->file = stdout;
        stdoutStream->ownsFile = false;
    }

    return stdoutStream;
}
BinaryFileStreamPtr Stdio::getValidStderrStream()
{
    if (!stderrStream)
    {
        stderrStream = std::make_shared<BinaryFileStream> ();
        stderrStream->file = stderr;
        stderrStream->ownsFile = false;
    }

    return stderrStream;
}
BinaryFileStreamPtr Stdio::stdinStream;
BinaryFileStreamPtr Stdio::stdoutStream;
BinaryFileStreamPtr Stdio::stderrStream;

} // End of namespace

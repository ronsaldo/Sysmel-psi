#include "Value.hpp"
#include "Type.hpp"
#include "Semantics.hpp"
#include <exception>
#include <sstream>

namespace Sysmel
{
    
ValuePtr Value::getType() const
{
    return GradualType::uniqueInstance();
}

ValuePtr Value::getTypeOrClass() const
{
    return getType();
}

ValuePtr Value::performWithArguments(const ValuePtr &selector, const std::vector<ValuePtr> &arguments)
{
    auto typeOrClass = getClass();
    if (!typeOrClass)
    {
        typeOrClass = getType();
        if(!typeOrClass)
            throwExceptionWithMessage("Cannot send a message to something without a type or a class.");
    }

    return typeOrClass->performWithArgumentsOnInstance(shared_from_this(), selector, arguments);
}

ValuePtr Value::performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments)
{
    (void)receiver;
    (void)selector;
    (void)arguments;
    throwExceptionWithMessage("Cannot perform instance message for not being a type or a class.");
}

ValuePtr Value::applyWithArguments(const std::vector<ValuePtr> &arguments)
{
    (void)arguments;
    throwExceptionWithMessage("Cannot apply over non-functional value.");
}

ValuePtr Value::lookupSelector(const ValuePtr &selector)
{
    (void)selector;
    return nullptr;
}

[[noreturn]] void Value::throwExceptionWithMessage(const char *message)
{
    throwExceptionWithMessageAt(message, getSourcePosition());
}

[[noreturn]] void Value::throwExceptionWithMessageAt(const char *message, const SourcePositionPtr &position)
{
    if (position)
    {
        std::ostringstream out;
        position->formatIn(out);
        out << ": " << message;
        throw std::runtime_error(out.str());
    }
    else
    {
        throw std::runtime_error(message);
    }
}

ValuePtr Value::analyzeInEnvironment(const EnvironmentPtr &environment)
{
    (void)environment;
    return shared_from_this();
}

ValuePtr Value::analyzeInEnvironmentForMacroExpansionOnly(const EnvironmentPtr &environment)
{
    (void)environment;
    return shared_from_this();
}

ValuePtr Value::evaluateInEnvironment(const EnvironmentPtr &environment)
{
    (void)environment;
    return shared_from_this();
}

ValuePtr Value::analyzeAndEvaluateInEnvironment(const EnvironmentPtr &environment)
{
    return analyzeInEnvironment(environment)->evaluateInEnvironment(environment);
}

ValuePtr Value::analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment)
{
    (void)environment;
    if (isSemanticValue())
        return shared_from_this();

    auto literal = std::make_shared<SemanticLiteralValue> ();
    literal->sourcePosition = syntaxNode->getSourcePosition();
    literal->value = shared_from_this();
    return literal;
}

bool Value::parseAndUnpackArgumentsPattern(std::vector<ValuePtr> &argumentNodes, bool &isExistential, bool &isVariadic)
{
    (void)argumentNodes;
    (void)isExistential;
    (void)isVariadic;

    throwExceptionWithMessage((printString() + "is not a valid argument pattern spec.").c_str());
}

bool Value::isSatisfiedByType(const ValuePtr &sourceType)
{
    auto self = shared_from_this();
    return sourceType->isSubclassOf(self) || sourceType->isSubtypeOf(self);
}

bool Value::isSubclassOf(const ValuePtr &targetSuperclass)
{
    return this == targetSuperclass.get();
}

bool Value::isSubtypeOf(const ValuePtr &targetSupertype)
{
    return this == targetSupertype.get();
}

ValuePtr Value::coerceIntoExpectedTypeAt(const ValuePtr &targetType, const SourcePositionPtr &coercionLocation)
{
    auto myType = getTypeOrClass();
    if(myType == targetType)
        return shared_from_this();

    if(!targetType->isSatisfiedByType(myType))
    {
        throwExceptionWithMessageAt(("Cannot coerce value of type " + myType->printString() + " into " + targetType->printString()).c_str(), coercionLocation);
    }

    return shared_from_this();
}

ValuePtr Value::getClass() const
{
    return nullptr;
}

} // End of namespace Sysmel

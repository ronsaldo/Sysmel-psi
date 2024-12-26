#include "Value.hpp"
#include "Type.hpp"
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
        auto typeOrClass = getType();
        if (!typeOrClass)
        {
            typeOrClass = getClass();
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
        auto sourcePosition = getSourcePosition();
        if (sourcePosition)
        {
            std::ostringstream out;
            sourcePosition->formatIn(out);
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
    
    ValuePtr Value::evaluateInEnvironment(const EnvironmentPtr &environment)
    {
        (void)environment;
        return shared_from_this();
    }

    ValuePtr Value::analyzeAndEvaluateInEnvironment(const EnvironmentPtr &environment)
    {
        return analyzeInEnvironment(environment)->evaluateInEnvironment(environment);
    }

    ValuePtr Value::getClass() const
    {
        return nullptr;
    }

} // End of namespace Sysmel

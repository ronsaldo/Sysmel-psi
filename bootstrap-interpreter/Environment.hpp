#ifndef SYSMEL_ENVIRONMENT_HPP
#define SYSMEL_ENVIRONMENT_HPP

#pragma once

#include "Object.hpp"
#include <map>

namespace Sysmel
{
typedef std::shared_ptr<Environment> EnvironmentPtr;
typedef std::shared_ptr<class IntrinsicsEnvironment> IntrinsicsEnvironmentPtr;
typedef std::shared_ptr<class LexicalEnvironment> LexicalEnvironmentPtr;
typedef std::shared_ptr<class Module> ModulePtr;
typedef std::shared_ptr<class Namespace> NamespacePtr;

class Environment : public Object
{
public:
    virtual EnvironmentPtr getParent() const = 0;

    virtual ModulePtr getModule() const
    {
        auto parent = getParent();
        return parent->getModule();
    }

    virtual NamespacePtr getNamespace() const
    {
        auto parent = getParent();
        return parent->getNamespace();
    }

    virtual ValuePtr lookupSymbolRecursively(SymbolPtr symbol)
    {
        return getParent()->lookupSymbolRecursively(symbol);
    }

};

class EmptyEnvironment : public Environment
{
public:
    virtual EnvironmentPtr getParent() const
    {
        return nullptr;
    }

    virtual ModulePtr getModule() const
    {
        return nullptr;
    }

    virtual NamespacePtr getNamespace() const
    {
        auto parent = getParent();
        return parent->getNamespace();
    }

    virtual ValuePtr lookupSymbolRecursively(SymbolPtr symbol)
    {
        (void)symbol;
        return nullptr;
    }
};

class NonEmptyEnvironment : public Environment
{
public:
    virtual EnvironmentPtr getParent() const
    {
        return parent;
    }

    virtual ValuePtr lookupLocalSymbol(SymbolPtr symbol)
    {
        auto it = symbolTable.find(symbol);
        return it != symbolTable.end() ? it->second : nullptr;
    }

    virtual ValuePtr lookupSymbolRecursively(SymbolPtr symbol)
    {
        auto localLookup = lookupLocalSymbol(symbol);
        if (localLookup)
            return localLookup;
        else
            return nullptr;
    }

    void addLocalSymbolBinding(SymbolPtr symbol, ValuePtr binding)
    {
        symbolTable[symbol] = binding;
    }

    EnvironmentPtr parent;
    std::map<SymbolPtr, ValuePtr> symbolTable;
};

class IntrinsicsEnvironment : public NonEmptyEnvironment
{
public:
    static IntrinsicsEnvironmentPtr uniqueInstance();

    ClassPtr lookupValidClass(const std::string &s);
    void addIntrinsicClass(const ClassPtr &intrinsicClass);

private:
    void buildIntrinsicsState();
    void buildBasicTypes();
    void buildMetaHierarchy();
    void buildObjectPrimitives();
    void buildValuePrimitives();

    void addPrimitiveToClass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature);
    void addPrimitiveToMetaclass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature);

    static IntrinsicsEnvironmentPtr singleton;
    std::map<std::string, ClassPtr> intrinsicClasses;
    std::map<std::string, MetaclassPtr> intrinsicMetaclasses;
};

class ModuleEnvironment : public NonEmptyEnvironment
{
public:
    ModuleEnvironment(const ModulePtr &cmodule, const EnvironmentPtr &cparent)
    {
        module = cmodule;
        parent = cparent;
    }

    virtual ModulePtr getModule() const
    {
        return module;
    }

    ModulePtr module;
};

class NamespaceEnvironment : public NonEmptyEnvironment
{
public:
    NamespaceEnvironment(const NamespacePtr cnamespce, const EnvironmentPtr &cparent)
    {
        namespce = cnamespce;
        parent = cparent;
    }

    virtual NamespacePtr getNamespace() const
    {
        return namespce;
    }

    NamespacePtr namespce;
};

class LexicalEnvironment : public NonEmptyEnvironment
{
public:
    LexicalEnvironment(const EnvironmentPtr &cparent, const SourcePositionPtr &csourcePosition)
    {
        parent = cparent;
        sourcePosition = csourcePosition;
    }

    SourcePositionPtr sourcePosition;
};

}
#endif //SYSMEL_ENVIRONMENT_HPP
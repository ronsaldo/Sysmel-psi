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
    typedef std::shared_ptr<class FunctionalAnalysisEnvironment> FunctionalAnalysisEnvironmentPtr;
    typedef std::shared_ptr<class FunctionalActivationEnvironment> FunctionalActivationEnvironmentPtr;
    typedef std::shared_ptr<class Module> ModulePtr;
    typedef std::shared_ptr<class Namespace> NamespacePtr;
    typedef std::shared_ptr<class SymbolValueBinding> SymbolValueBindingPtr;
    typedef std::shared_ptr<class SymbolCaptureBinding> SymbolCaptureBindingPtr;
    typedef std::shared_ptr<class SymbolArgumentBinding> SymbolArgumentBindingPtr;
    typedef std::shared_ptr<class SymbolFixpointBinding> SymbolFixpointBindingPtr;

    class SymbolValueBinding : public Value
    {
    public:
        SymbolPtr name;
        ValuePtr analyzedValue;

        virtual ValuePtr analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment);
    };

    class SymbolArgumentBinding : public Value
    {
    public:
        SymbolPtr name;
        ValuePtr type;
        bool isImplicit = false;
        bool isExistential = false;

        virtual void printStringOn(std::ostream &out) const override
        {
            if(name)
                name->printStringOn(out);

            if(type && name)
                out << " : ";
            if(type)
                type->printStringOn(out);
        }

        virtual ValuePtr analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment);
    };

    class SymbolFixpointBinding : public Value
    {
    public:
        SymbolPtr name;
        ValuePtr typeExpression;

        virtual void printStringOn(std::ostream &out) const override
        {
            if(name)
                name->printStringOn(out);

            if(typeExpression && name)
                out << " : ";
            if(typeExpression)
                typeExpression->printStringOn(out);
        }

        virtual ValuePtr analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment);
    };

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

        virtual FunctionalAnalysisEnvironmentPtr getFunctionalAnalysisEnvironment()
        {
            auto parent = getParent();
            return parent->getFunctionalAnalysisEnvironment();
        }

        virtual FunctionalActivationEnvironmentPtr getFunctionalActivationEnvironment()
        {
            auto parent = getParent();
            return parent->getFunctionalActivationEnvironment();
        }

        virtual ValuePtr lookupValueForBinding(ValuePtr binding)
        {
            auto parent = getParent();
            return parent->lookupValueForBinding(binding);
        }

        virtual ValuePtr lookupSymbolRecursively(SymbolPtr symbol)
        {
            return getParent()->lookupSymbolRecursively(symbol);
        }

        virtual void addLocalSymbolBinding(SymbolPtr symbol, ValuePtr binding) = 0;
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

        virtual FunctionalAnalysisEnvironmentPtr getFunctionalAnalysisEnvironment() override
        {
            return nullptr;
        }

        virtual FunctionalActivationEnvironmentPtr getFunctionalActivationEnvironment() override
        {
            return nullptr;
        }

        virtual ValuePtr lookupValueForBinding(ValuePtr binding) override
        {
            (void)binding;
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

        virtual void addLocalSymbolBinding(SymbolPtr symbol, ValuePtr binding)
        {
            (void)symbol;
            (void)binding;
            fprintf(stderr, "Cannot add local symbol binding to empty environments.\n");
            abort();
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

        virtual ValuePtr lookupSymbolRecursively(SymbolPtr symbol) override
        {
            auto localLookup = lookupLocalSymbol(symbol);
            if (localLookup)
                return localLookup;
            else if (parent)
                return parent->lookupSymbolRecursively(symbol);
            else
                return nullptr;
        }

        virtual void addLocalSymbolBinding(SymbolPtr symbol, ValuePtr binding) override
        {
            symbolTable.insert(std::make_pair(symbol, binding));
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
        void buildBasicMacros();
        void addPrimitiveToClass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature);
        void addPrimitiveToMetaclass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature);
        void addPrimitiveGlobalMacro(const std::string &name, PrimitiveMacroImplementationSignature);

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

    class FunctionalAnalysisEnvironment : public NonEmptyEnvironment
    {
    public:
        FunctionalAnalysisEnvironment(const EnvironmentPtr &cparent, const SourcePositionPtr &csourcePosition)
        {
            parent = cparent;
            sourcePosition = csourcePosition;
        }

        virtual FunctionalAnalysisEnvironmentPtr getFunctionalAnalysisEnvironment() override
        {
            return std::static_pointer_cast<FunctionalAnalysisEnvironment> (shared_from_this());
        }

        void addArgumentBinding(const SymbolArgumentBindingPtr &analyzedArgument)
        {
            auto name = analyzedArgument->name;
            if(name)
                addLocalSymbolBinding(name, analyzedArgument);
            argumentBindings.push_back(analyzedArgument);
        }

        void addFixpointBinding(const SymbolFixpointBindingPtr &fixpointBinding)
        {
            sysmelAssert(!this->fixpointBinding);
            if(fixpointBinding->name)
                addLocalSymbolBinding(fixpointBinding->name, fixpointBinding);
            this->fixpointBinding = fixpointBinding;
        }

        SourcePositionPtr sourcePosition;
        SymbolFixpointBindingPtr fixpointBinding;
        std::vector<ValuePtr> argumentBindings;
    };

    class FunctionalActivationEnvironment : public NonEmptyEnvironment
    {
    public:
        FunctionalActivationEnvironment(const EnvironmentPtr &cparent, const SourcePositionPtr &csourcePosition)
        {
            parent = cparent;
            sourcePosition = csourcePosition;
        }

        virtual FunctionalActivationEnvironmentPtr getFunctionalActivationEnvironment() override
        {
            return std::static_pointer_cast<FunctionalActivationEnvironment> (shared_from_this());
        }

        void forArgumentBindingSetValue(const SymbolArgumentBindingPtr &binding, ValuePtr value)
        {
            argumentBindings.insert(std::make_pair(binding, value));
        }

        virtual ValuePtr lookupValueForBinding(ValuePtr binding) override
        {
            auto it = argumentBindings.find(std::static_pointer_cast<SymbolArgumentBinding> (binding));
            if(it != argumentBindings.end())
                return it->second;
            return nullptr;
        }

        SourcePositionPtr sourcePosition;
        std::map<SymbolArgumentBindingPtr, ValuePtr> argumentBindings;
    };
}
#endif // SYSMEL_ENVIRONMENT_HPP
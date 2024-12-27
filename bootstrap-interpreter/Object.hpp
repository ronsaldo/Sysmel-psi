#ifndef SYSMEL_OBJECT_HPP
#define SYSMEL_OBJECT_HPP

#include "Value.hpp"
#include "LargeInteger.hpp"
#include <map>
#include <functional>

namespace Sysmel
{

typedef std::shared_ptr<class ProtoObject> ProtoObjectPtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class UndefinedObject> UndefinedObjectPtr;
typedef std::shared_ptr<class Class> ClassPtr;
typedef std::shared_ptr<class Metaclass> MetaclassPtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;

typedef std::function<ValuePtr (const std::vector<ValuePtr> &arguments)> PrimitiveImplementationSignature;

class ProtoObject : public Value
{
public:
    virtual const char *getClassName() const {return "ProtoObject";}
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "a ";
        getClass()->printStringOn(out);
    }

    virtual ValuePtr getClass() const override;

    ValuePtr getTypeOrClass() const override
    {
        return getClass();
    }

    mutable ValuePtr clazz;
    size_t identityHash;
};

class Object : public ProtoObject
{
public:
    virtual const char *getClassName() const override {return "Object";}
};

class UndefinedObject : public Object
{
public:
    virtual const char *getClassName() const override {return "UndefinedObject";}

    static UndefinedObjectPtr uniqueInstance();
private:
    static UndefinedObjectPtr singleton;
};

class Behavior : public Object
{
public:
    virtual const char *getClassName() const override {return "Behavior";}
    
    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments) override
    {
        auto method = receiver->getClass()->lookupSelector(selector);
        if(!method)
        {
            receiver->throwExceptionWithMessage(("Failed to find method " + selector->printString() + " in " + receiver->getClass()->printString()).c_str());
        }
        
        std::vector<ValuePtr> allArguments;
        allArguments.reserve(1 + arguments.size());
        allArguments.push_back(receiver);
        for(auto &arg : arguments)
            allArguments.push_back(arg);

        return method->applyWithArguments(allArguments);
    }

    virtual ValuePtr lookupSelector(const ValuePtr &selector) override
    {
        auto it = methodDict.find(std::static_pointer_cast<Symbol> (selector));
        if(it != methodDict.end())
            return it->second;
        if(superclass)
            return superclass->lookupSelector(selector);
        return nullptr;
    }

    ValuePtr superclass;
    std::map<SymbolPtr, ValuePtr> methodDict;
};

class ClassDescription : public Behavior
{
public:

    virtual const char *getClassName() const override {return "ClassDescription";}
};

class Class : public ClassDescription
{
public:
    virtual const char *getClassName() const override {return "Class";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << name;
    }

    std::string name;
};

class Metaclass : public ClassDescription
{
public:
    virtual const char *getClassName() const override {return "Metaclass";}

    virtual void printStringOn(std::ostream &out) const override
    {
        auto clazz = thisClass.lock();
        if (clazz)
        {
            clazz->printStringOn(out);
            out << " class";
        }
        else
        {
            out << "a Metaclass";
        }
    }

    ClassWeakPtr thisClass;
};

class PrimitiveMethod : public Object
{
public:
    PrimitiveMethod(PrimitiveImplementationSignature cimplementation)
        : implementation(cimplementation) {}

    virtual const char *getClassName() const override {return "PrimitiveMethod";}
    virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments) override;

    PrimitiveImplementationSignature implementation;
};

class CompiledMethod : public Object
{
public:
    virtual const char *getClassName() const override {return "CompiledMethod";}
    virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments) override;
};

class Magnitude : public Object
{
public:
    virtual const char *getClassName() const override {return "Magnitude";}
};

class Character : public Magnitude
{
public:
    virtual const char *getClassName() const override {return "Character";}
    
    char32_t value;
};

class Number : public Magnitude
{
public:
    virtual const char *getClassName() const override {return "Number";}
};

class Integer : public Number
{
public:
    virtual const char *getClassName() const override {return "Integer";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value;
    }

    LargeInteger value;
};

class Float : public Number
{
public:
    virtual const char *getClassName() const override {return "Float";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value;
    }

    double value;
};

class Collection : public Object
{
public:
    virtual const char *getClassName() const override {return "Collection";}
};

class SequenceableCollection : public Collection
{
public:
    virtual const char *getClassName() const override {return "SequenceableCollection";}
};

class ArrayedCollection : public Collection
{
public:
    virtual const char *getClassName() const override {return "ArrayedCollection";}
};

class Array : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override {return "Array";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '[';
        bool first = true;
        for (auto &value : values)
        {
            if(first)
                first = false;
            else
                out << ". ";
            value->printStringOn(out);
        }
        out << ']';
    }

    std::vector<ValuePtr> values;
};

class ByteArray : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override {return "ByteArray";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "#[";
        bool first = true;
        for (auto &value : values)
        {
            if(first)
                first = false;
            else
                out << ". ";
            out << value;
        }
        out << ']';
    }

    std::vector<uint8_t> values;
};

class String : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override {return "String";}

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '"' << value << '"';
    }

    std::string value;
};

class Symbol : public String
{
public:
    virtual const char *getClassName() const override {return "Symbol";}
    static SymbolPtr internString(const std::string &string);

    virtual SymbolPtr asAnalyzedSymbolValue() override
    {
        return std::static_pointer_cast<Symbol> (shared_from_this());
    }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "#\"" << value << '"';
    }

private:
    static std::map<std::string, SymbolPtr> internedSymbols;
};

} // End of namespace Sysmel

#endif //SYSMEL_OBJECT_HPP
#ifndef SYSMEL_OBJECT_HPP
#define SYSMEL_OBJECT_HPP

#include "Value.hpp"
#include "LargeInteger.hpp"
#include <map>

namespace Sysmel
{

typedef std::shared_ptr<class ProtoObject> ProtoObjectPtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class Class> ClassPtr;
typedef std::shared_ptr<class Metaclass> MetaclassPtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;

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
};

class Behavior : public Object
{
public:
    virtual const char *getClassName() const override {return "Behavior";}

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

    ClassWeakPtr thisClass;
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

class String : public Object
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
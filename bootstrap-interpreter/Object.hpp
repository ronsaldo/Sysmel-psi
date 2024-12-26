#ifndef SYSMEL_OBJECT_HPP
#define SYSMEL_OBJECT_HPP

#include "Value.hpp"
#include "LargeInteger.hpp"

namespace Sysmel
{

class ProtoObject : public Value
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "a ";
        getClass()->printStringOn(out);
    }

    virtual ValuePtr getClass() const override;

    ClassPtr clazz;
    size_t identityHash;
};

class Object : public ProtoObject
{
public:
};

class Behavior : public Object
{
public:

};

class ClassDescription : public Behavior
{
public:

};

class Class : public ClassDescription
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << name;
    }

    std::string name;
};

class Metaclass : public ClassDescription
{
public:
    ClassPtr thisClass;
};

class String : public Object
{
public:
    std::string data;
};

class Magnitude : public Object
{
public:
};

class Character : public Magnitude
{
public:
};

class Number : public Magnitude
{
public:
};

class Integer : public Number
{
public:
    LargeInteger value;
};

class Float : public Number
{
public:
    double value;
};

} // End of namespace Sysmel

#endif //SYSMEL_OBJECT_HPP
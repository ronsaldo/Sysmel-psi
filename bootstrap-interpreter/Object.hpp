#ifndef SYSMEL_OBJECT_HPP
#define SYSMEL_OBJECT_HPP

#include "Value.hpp"

namespace Sysmel
{

class Object : public Value
{
public:
    virtual std::string printString() const {return "an Object";}
};

class Class : public Object
{
public:
    virtual std::string printString() const {return name;}

    std::string name;
};

} // End of namespace Sysmel

#endif //SYSMEL_OBJECT_HPP
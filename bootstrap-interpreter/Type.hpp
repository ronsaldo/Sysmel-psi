#ifndef SYSMEL_TYPE_HPP
#define SYSMEL_TYPE_HPP

#include "Value.hpp"
#include <sstream>
#include <vector>

namespace Sysmel
{

typedef std::shared_ptr<class TypeUniverse> TypeUniversePtr;
typedef std::shared_ptr<class GradualType> GradualTypePtr;

class TypeUniverse : public Value
{
public:
    TypeUniverse(int index = 0)
        : universeIndex(index) {}

    virtual ValuePtr getType() const;

    virtual std::string printString() const {
        std::ostringstream out;
        out << "Type(" << universeIndex << ")";
        return out.str();
    }

    int universeIndex = 0;

    static TypeUniversePtr uniqueInstanceForIndex(int index);

private:
    static std::vector<TypeUniversePtr> uniqueInstances;
};

class Type : public Value
{
public:
    virtual std::string printString() const {return "a Type";}

    static TypePtr uniqueInstance();
};

class GradualType : public Value
{
public:
    virtual std::string printString() const {return "?";}
    
    static GradualTypePtr uniqueInstance();
private:
    static GradualTypePtr singletonValue;
};


}// End of namespace Sysmel


#endif
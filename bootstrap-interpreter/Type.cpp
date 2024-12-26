#include "Type.hpp"

namespace Sysmel
{

// Type Universe
ValuePtr TypeUniverse::getType()
{
    return uniqueInstanceForIndex(universeIndex + 1);
}

std::vector<TypeUniversePtr> TypeUniverse::uniqueInstances;

TypeUniversePtr TypeUniverse::uniqueInstanceForIndex(int index)
{
    for(int currentIndex = 0; currentIndex <= index; ++currentIndex)
        uniqueInstances.push_back(std::make_shared<TypeUniverse> (currentIndex));
    return uniqueInstances[index];
}

// Generic type that ignores the universe index
ValuePtr Type::getType()
{
    return std::static_pointer_cast<Type> (shared_from_this());
}

// Gradual type
ValuePtr GradualType::getType()
{
    return uniqueInstance();
}

GradualTypePtr GradualType::singletonValue;
GradualTypePtr GradualType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<GradualType> ();
    return singletonValue;
}

} //end of namespace Sysmel

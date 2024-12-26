#include "Type.hpp"

namespace Sysmel
{

// Type Universe
ValuePtr TypeUniverse::getType() const
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

GradualTypePtr GradualType::singletonValue;
GradualTypePtr GradualType::uniqueInstance()
{
    if(!singletonValue)
        singletonValue = std::make_shared<GradualType> ();
    return singletonValue;
}

} //end of namespace Sysmel

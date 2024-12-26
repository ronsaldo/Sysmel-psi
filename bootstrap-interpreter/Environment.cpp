#include "Environment.hpp"
#include "Assert.hpp"

namespace Sysmel
{

ClassPtr IntrinsicsEnvironment::lookupValidClass(const std::string &name)
{
    auto it = intrinsicClasses.find(name);
    sysmelAssert(it != intrinsicClasses.end());
    return it->second;
}

void IntrinsicsEnvironment::addIntrinsicClass(const ClassPtr &intrinsicClass)
{
    intrinsicClasses[intrinsicClass->name] = intrinsicClass;
}

template<typename BaseClass> std::pair<ClassPtr, MetaclassPtr> makeClassAndMetaclass(const std::string &name)
{
    auto meta = std::make_shared<Metaclass> ();
    auto clazz = std::make_shared<Class> ();
    clazz->clazz = meta;
    clazz->name = name;

    meta->thisClass = clazz;

    return std::make_pair(clazz, meta);
}

void IntrinsicsEnvironment::buildIntrinsicsState()
{
    parent = std::make_shared<EmptyEnvironment> ();

    std::vector<std::pair<ClassPtr, MetaclassPtr> > intrinsicClassesAndMetaclasses{
#define AddClass(cls) makeClassAndMetaclass<cls> (#cls),
#define AddClassWithSuperclass(cls, superclass) makeClassAndMetaclass<cls> (#cls),

#include "IntrinsicClasses.inc"

#undef AddClass
#undef AddClassWithSuperclass
    };

    for (auto &pair : intrinsicClassesAndMetaclasses)
    {
        intrinsicClasses[pair.first->name] = pair.first;
        intrinsicMetaclasses[pair.first->name] = pair.second;
    }
    
}

IntrinsicsEnvironmentPtr IntrinsicsEnvironment::uniqueInstance()
{
    if(!singleton)
    {
        singleton = std::make_shared<IntrinsicsEnvironment> ();
        singleton->buildIntrinsicsState();
    }
    return singleton;
}
IntrinsicsEnvironmentPtr IntrinsicsEnvironment::singleton;
} // End of namespace Sysmel

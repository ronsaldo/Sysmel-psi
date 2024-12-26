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
    buildMetaHierarchy();
    buildPrimitives();
}

void IntrinsicsEnvironment::buildMetaHierarchy()
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

    for (auto &pair : intrinsicClassesAndMetaclasses)
    {
        addLocalSymbolBinding(Symbol::internString(pair.first->name), pair.second);
    }

    // Connect the superclasses
#define AddClass(cls)
#define AddClassWithSuperclass(cls, super) \
    intrinsicClasses[#cls]->superclass     =  intrinsicClasses[#super]; \
    intrinsicMetaclasses[#cls]->superclass =  intrinsicMetaclasses[#super];

#include "IntrinsicClasses.inc"

#undef AddClass
#undef AddClassWithSuperclass

    // Metaclasses are instances of Metaclass
    auto MetaclassObj = intrinsicClasses["Metaclass"];
#define AddClass(cls) \
    intrinsicMetaclasses[#cls]->clazz = MetaclassObj;
#define AddClassWithSuperclass(cls, super) \
    intrinsicMetaclasses[#cls]->clazz = MetaclassObj;

#include "IntrinsicClasses.inc"

#undef AddClass
#undef AddClassWithSuperclass

    // Meta hiearchy short-circuit.
    intrinsicMetaclasses["ProtoObject"]->superclass = intrinsicClasses["Class"];
}

void IntrinsicsEnvironment::buildPrimitives()
{
    addPrimitiveToClass("Integer", "+", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value + right->value;
        return result;
    });
}

void IntrinsicsEnvironment::addPrimitiveToClass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature impl)
{
    auto primitive = std::make_shared<PrimitiveMethod> (impl);
    auto &clazz = intrinsicClasses[className];
    clazz->methodDict[Symbol::internString(selector)] = primitive;
}

void IntrinsicsEnvironment::addPrimitiveToMetaclass(const std::string &className, const std::string &selector, PrimitiveImplementationSignature impl)
{
    auto primitive = std::make_shared<PrimitiveMethod> (impl);
    auto &clazz = intrinsicClasses[className];
    clazz->methodDict[Symbol::internString(selector)] = primitive;
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

#include "Environment.hpp"
#include "Assert.hpp"
#include "Type.hpp"
#include "Semantics.hpp"
#include "Syntax.hpp"

namespace Sysmel
{

ValuePtr SymbolValueBinding::analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment)
{
    (void)syntaxNode;
    (void)environment;
    return analyzedValue;
}

ValuePtr SymbolArgumentBinding::analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment)
{
    (void)syntaxNode;
    (void)environment;
    auto semanticReference = std::make_shared<SemanticIdentifierReference> ();
    semanticReference->sourcePosition =  sourcePosition;
    semanticReference->type = type;
    semanticReference->identifierBinding = shared_from_this();
    return semanticReference;
}

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
    buildBasicTypes();
    buildObjectPrimitives();
    buildValuePrimitives();
    buildBasicMacros();
}

void IntrinsicsEnvironment::buildBasicTypes()
{
    addLocalSymbolBinding(Symbol::internString("Type"),       Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("UnitType"),   UnitType::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("BottomType"), BottomType::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Void"),       VoidType::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("true"),       True::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("false"),      False::uniqueInstance());
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
        addLocalSymbolBinding(Symbol::internString(pair.first->name), pair.first);
    }
    addLocalSymbolBinding(Symbol::internString("nil"), UndefinedObject::uniqueInstance());

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
    intrinsicClasses["ProtoObject"]->superclass = UndefinedObject::uniqueInstance();
    intrinsicMetaclasses["ProtoObject"]->superclass = intrinsicClasses["Class"];
}

void IntrinsicsEnvironment::buildObjectPrimitives()
{
    // ProtoObject
    addPrimitiveToClass("ProtoObject", "class", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        auto self = std::static_pointer_cast<ProtoObject> (arguments[0]);
        return self->clazz;
    });
    addPrimitiveToClass("ProtoObject", "identityHash", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        auto self = std::static_pointer_cast<ProtoObject> (arguments[0]);
        auto result = std::make_shared<Integer> ();
        result->value = LargeInteger(self->identityHash);
        return result;
    });

    // Behavior
    addPrimitiveToClass("Behavior", "withSelector:addMethod:", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 3);
        auto behavior = std::static_pointer_cast<Behavior> (arguments[0]);
        auto selector = arguments[1];
        auto method = arguments[0];
        behavior->methodDict.insert(std::make_pair(selector->asAnalyzedSymbolValue(), method));
        return behavior;
    });

    // Object
    addPrimitiveToClass("Object", "printString", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        auto string = arguments[0]->printString();
        auto stringObject = std::make_shared<String> ();
        stringObject->value = string;
        return stringObject;
    });
    addPrimitiveToClass("Object", "yourself", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        return arguments[0];
    });
    
    addPrimitiveToClass("Object", "at:", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto index = arguments[1]->evaluateAsIndex();
        return arguments[0]->getElementAtIndex(index);
    });
    addPrimitiveToClass("Object", "at:put:", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 3);
        auto index = arguments[1]->evaluateAsIndex();
        return arguments[0]->setElementAtIndex(index, arguments[2]);
    });

    // Collection
    addPrimitiveToClass("Collection", "size", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        auto collection = std::static_pointer_cast<Collection> (arguments[0]);
        auto size = collection->getSize();
        auto sizeInteger = std::make_shared<Integer> ();
        sizeInteger->value = LargeInteger(size);
        return sizeInteger;
    });

    // Integer
    addPrimitiveToClass("Integer", "+", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value + right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "-", [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value - right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "*", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value * right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "//", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value / right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "\\\\", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value % right->value;
        return result;
    });
    

    // Stream
    addPrimitiveToClass("Stream", "nextPut:", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto stream = std::static_pointer_cast<Stream> (arguments[0]);
        stream->nextPut(arguments[1]);
        return stream;
    });
    addPrimitiveToClass("Stream", "nextPutAll:", [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto stream = std::static_pointer_cast<Stream> (arguments[0]);
        stream->nextPutAll(arguments[1]);
        return stream;
    });

    // Stdio
    addPrimitiveToMetaclass("Stdio", "stdin", [](const std::vector<ValuePtr> &arguments) {
        (void)arguments;
        return Stdio::getValidStdinStream();
    });
    addPrimitiveToMetaclass("Stdio", "stdout", [](const std::vector<ValuePtr> &arguments) {
        (void)arguments;
        return Stdio::getValidStdoutStream();
    });
    addPrimitiveToMetaclass("Stdio", "stderr", [](const std::vector<ValuePtr> &arguments) {
        (void)arguments;
        return Stdio::getValidStderrStream();
    });
}

void IntrinsicsEnvironment::buildValuePrimitives()
{
}

void IntrinsicsEnvironment::buildBasicMacros()
{
    addPrimitiveGlobalMacro("if:then:else:", [](const MacroContextPtr &context, const std::vector<ValuePtr> &arguments){
        auto syntaxIf = std::make_shared<SyntaxIf> ();
        syntaxIf->sourcePosition = context->sourcePosition;
        syntaxIf->condition = arguments[0];
        syntaxIf->trueCase = arguments[1];
        syntaxIf->falseCase = arguments[2];
        return syntaxIf;
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
    auto &clazz = intrinsicMetaclasses[className];
    clazz->methodDict[Symbol::internString(selector)] = primitive;
}

void IntrinsicsEnvironment::addPrimitiveGlobalMacro(const std::string &name, PrimitiveMacroImplementationSignature impl)
{
    auto primitiveMacro = std::make_shared<PrimitiveMacroMethod> (impl);
    addLocalSymbolBinding(Symbol::internString(name), primitiveMacro);
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

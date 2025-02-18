#include "Environment.hpp"
#include "Assert.hpp"
#include "Type.hpp"
#include "Semantics.hpp"
#include "Syntax.hpp"
#include <math.h>

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

ValuePtr SymbolFixpointBinding::analyzeIdentifierReferenceInEnvironment(const ValuePtr &syntaxNode, const EnvironmentPtr &environment)
{
    (void)syntaxNode;
    (void)environment;
    auto semanticReference = std::make_shared<SemanticIdentifierReference>();
    semanticReference->sourcePosition =  sourcePosition;
    semanticReference->type = typeExpression->analyzeInEnvironment(environment);
    semanticReference->identifierBinding = shared_from_this();
    return semanticReference;
}

ClassPtr IntrinsicsEnvironment::lookupValidClass(const std::string &name)
{
    auto it = intrinsicClasses.find(name);
    sysmelAssert(it != intrinsicClasses.end());
    return it->second;
}

MetaclassPtr IntrinsicsEnvironment::lookupValidMetaclass(const std::string &name)
{
    auto it = intrinsicMetaclasses.find(name);
    sysmelAssert(it != intrinsicMetaclasses.end());
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
    clazz->subclasses = std::make_shared<Array> ();
    clazz->format = sizeof(BaseClass);
    clazz->interpreterBasicNew = []() {
        return std::make_shared<BaseClass> ();
    };

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
    addLocalSymbolBinding(Symbol::internString("void"),       VoidValue::uniqueInstance());

    addLocalSymbolBinding(Symbol::internString("UInt8"),  PrimitiveUInt8Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("UInt16"), PrimitiveUInt16Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("UInt32"), PrimitiveUInt32Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("UInt64"), PrimitiveUInt64Type::uniqueInstance());

    addLocalSymbolBinding(Symbol::internString("Int8"),   PrimitiveInt8Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Int16"),  PrimitiveInt16Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Int32"),  PrimitiveInt32Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Int64"),  PrimitiveInt64Type::uniqueInstance());

    addLocalSymbolBinding(Symbol::internString("Char8"),  PrimitiveChar8Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Char16"), PrimitiveChar16Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Char32"), PrimitiveChar32Type::uniqueInstance());

    addLocalSymbolBinding(Symbol::internString("Float32"), PrimitiveFloat32Type::uniqueInstance());
    addLocalSymbolBinding(Symbol::internString("Float64"), PrimitiveFloat64Type::uniqueInstance());
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
    intrinsicMetaclasses[#cls]->clazz = MetaclassObj; \
    intrinsicClasses[#cls]->registerInSuperclass();

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
    addPrimitiveToClass("ProtoObject", "initialize",
        SimpleFunctionType::make(lookupValidClass("ProtoObject"), "self", lookupValidClass("ProtoObject")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            return arguments[0];
        });
    addPrimitiveToClass("ProtoObject", "class",
        SimpleFunctionType::make(lookupValidClass("ProtoObject"), "self", lookupValidClass("ProtoObject")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<ProtoObject> (arguments[0]);
            return self->clazz;
        });
    addPrimitiveToClass("ProtoObject", "identityHash",
        SimpleFunctionType::make(lookupValidClass("ProtoObject"), "self", lookupValidClass("Integer")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<ProtoObject> (arguments[0]);
            auto result = std::make_shared<Integer> ();
            result->value = LargeInteger(self->identityHash);
            return result;
        });

    // Behavior
    addPrimitiveToClass("Behavior", "withSelector:addMethod:",
        SimpleFunctionType::make(
            lookupValidClass("Behavior"), "self",
            lookupValidClass("Symbol"), "selector",
            GradualType::uniqueInstance(), "method",
            lookupValidClass("Behavior")),

        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 3);
            auto behavior = std::static_pointer_cast<Behavior> (arguments[0]);
            auto selector = arguments[1];
            auto method = arguments[0];
            behavior->methodDict.insert(std::make_pair(selector->asAnalyzedSymbolValue(), method));
            return behavior;
        });
    addPrimitiveToClass("Behavior", "basicNew",
        SimpleFunctionType::make(
            lookupValidClass("Behavior"), "self",
            lookupValidClass("ProtoObject")),

        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Behavior> (arguments[0]);
            return self->basicNew();
        });
    addPrimitiveToClass("Behavior", "new",
        SimpleFunctionType::make(
            lookupValidClass("Behavior"), "self",
            lookupValidClass("ProtoObject")),

        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Behavior> (arguments[0]);
            auto basicNew = self->basicNew();
            std::vector<ValuePtr> initializeArgs;
            return basicNew->performWithArguments(Symbol::internString("initialize"), initializeArgs);
        });
    addPrimitiveToClass("Behavior", "superclass",
        SimpleFunctionType::make(
            lookupValidClass("Behavior"), "self",
            lookupValidClass("Behavior")),

        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto behavior = std::static_pointer_cast<Behavior> (arguments[0]);
            return behavior->superclass;
        });

    // Class
    addPrimitiveToClass("Class", "subclasses",
        SimpleFunctionType::make(
            lookupValidClass("Class"), "self",
            lookupValidClass("Array")),

        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto clazz = std::static_pointer_cast<Class> (arguments[0]);
            return clazz->subclasses;
        });

    // Object
    addPrimitiveToClass("Object", "printString", 
        SimpleFunctionType::make(
            lookupValidClass("Object"), "self",
            lookupValidClass("String")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            auto string = arguments[0]->printString();
            auto stringObject = std::make_shared<String> ();
            stringObject->value = string;
            return stringObject;
        });
    addPrimitiveToClass("Object", "yourself",
        SimpleFunctionType::make(
            lookupValidClass("Object"), "self",
            lookupValidClass("Object")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            return arguments[0];
        });
    
    addPrimitiveToClass("Object", "at:",
        SimpleFunctionType::make(
            lookupValidClass("Object"), "self",
            lookupValidClass("Integer"), "index",
            lookupValidClass("Object")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 2);
            auto index = arguments[1]->evaluateAsIndex();
            return arguments[0]->getElementAtIndex(index);
        });
    addPrimitiveToClass("Object", "at:put:",
        SimpleFunctionType::make(
            lookupValidClass("Object"), "self",
            lookupValidClass("Integer"), "index",
            lookupValidClass("Object"), "element",
            lookupValidClass("Object")), 
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 3);
            auto index = arguments[1]->evaluateAsIndex();
            return arguments[0]->setElementAtIndex(index, arguments[2]);
        });

    // Collection
    addPrimitiveToClass("Collection", "size",
        SimpleFunctionType::make(
            lookupValidClass("Collection"), "self",
            lookupValidClass("Integer")),
        [](const std::vector<ValuePtr> &arguments){
            sysmelAssert(arguments.size() == 1);
            auto collection = std::static_pointer_cast<Collection> (arguments[0]);
            auto size = collection->getSize();
            auto sizeInteger = std::make_shared<Integer> ();
            sizeInteger->value = LargeInteger(size);
            return sizeInteger;
        });

    // Integer
    auto integerUnaryArithmeticType = SimpleFunctionType::make(
            lookupValidClass("Integer"), "self",
            lookupValidClass("Integer"));
    auto integerBinaryArithmeticType = SimpleFunctionType::make(
            lookupValidClass("Integer"), "self",
            lookupValidClass("Integer"), "other",
            lookupValidClass("Integer"));
    auto integerBinaryComparisonType = SimpleFunctionType::make(
            lookupValidClass("Integer"), "self",
            lookupValidClass("Integer"), "other",
            lookupValidClass("Boolean"));

    addPrimitiveToClass("Integer", "negated", integerUnaryArithmeticType, [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 1);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);

        auto result = std::make_shared<Integer> ();
        result->value = -left->value;
        return result;
    });
    addPrimitiveToClass("Integer", "+", integerBinaryArithmeticType, [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value + right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "-", integerBinaryArithmeticType, [](const std::vector<ValuePtr> &arguments){
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value - right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "*", integerBinaryArithmeticType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value * right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "//", integerBinaryArithmeticType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value / right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "\\\\", integerBinaryArithmeticType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);

        auto result = std::make_shared<Integer> ();
        result->value = left->value % right->value;
        return result;
    });
    addPrimitiveToClass("Integer", "=", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value == right->value);
    });
    addPrimitiveToClass("Integer", "~=", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value != right->value);
    });
    addPrimitiveToClass("Integer", "<", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value < right->value);
    });
    addPrimitiveToClass("Integer", "<=", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value <= right->value);
    });
    addPrimitiveToClass("Integer", ">", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value > right->value);
    });
    addPrimitiveToClass("Integer", ">=", integerBinaryComparisonType, [](const std::vector<ValuePtr> &arguments) {
        sysmelAssert(arguments.size() == 2);
        auto left = std::static_pointer_cast<Integer> (arguments[0]);
        auto right = std::static_pointer_cast<Integer> (arguments[1]);
        return Boolean::encode(left->value >= right->value);
    });
    addPrimitiveToClass("Integer", "asInteger", 
        SimpleFunctionType::make(
            lookupValidClass("Integer"), "self",
            lookupValidClass("Integer")),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Integer> (arguments[0]);
            return self;
        });
    addPrimitiveToClass("Integer", "asFloat", 
        SimpleFunctionType::make(
                lookupValidClass("Integer"), "self",
                lookupValidClass("Float")),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Integer> (arguments[0]);
            auto floatObject = std::make_shared<Float> ();
            floatObject->value = self->value.asDouble();
            return floatObject;
        });


    // Stream
    addPrimitiveToClass("Stream", "nextPut:",
        SimpleFunctionType::make(
                lookupValidClass("Stream"), "self",
                lookupValidClass("Object"), "element",
                lookupValidClass("Stream")),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 2);
            auto stream = std::static_pointer_cast<Stream> (arguments[0]);
            stream->nextPut(arguments[1]);
            return stream;
        });
    addPrimitiveToClass("Stream", "nextPutAll:",
        SimpleFunctionType::make(
                lookupValidClass("Stream"), "self",
                lookupValidClass("Object"), "elements",
                lookupValidClass("Stream")),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 2);
            auto stream = std::static_pointer_cast<Stream> (arguments[0]);
            stream->nextPutAll(arguments[1]);
            return stream;
        });

    // Stdio
    addPrimitiveToMetaclass("Stdio", "stdin",
        SimpleFunctionType::make(
                lookupValidMetaclass("Stdio"), "self",
                lookupValidClass("BinaryFileStream")),
        [](const std::vector<ValuePtr> &arguments) {
            (void)arguments;
            return Stdio::getValidStdinStream();
        });
    addPrimitiveToMetaclass("Stdio", "stdout",
        SimpleFunctionType::make(
            lookupValidMetaclass("Stdio"), "self",
            lookupValidClass("BinaryFileStream")),
        [](const std::vector<ValuePtr> &arguments) {
            (void)arguments;
            return Stdio::getValidStdoutStream();
        });
    addPrimitiveToMetaclass("Stdio", "stderr",
        SimpleFunctionType::make(
                lookupValidMetaclass("Stdio"), "self",
                lookupValidClass("BinaryFileStream")),
        [](const std::vector<ValuePtr> &arguments) {
            (void)arguments;
            return Stdio::getValidStderrStream();
        });
}
template<typename PrimitiveNumberTypeClass, typename PrimitiveNumberValueClass>
void buildPrimitiveNumberTypeMethods(IntrinsicsEnvironment *environment,
    const char *literalSuffix, const char *conversionMethodName)
{
    auto primitiveType = PrimitiveNumberTypeClass::uniqueInstance();
    auto integerType = environment->lookupValidClass("Integer");
    auto floatType = environment->lookupValidClass("Float");
    environment->addPrimitiveToClass("Integer", literalSuffix,
        SimpleFunctionType::make(integerType, "self", primitiveType),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Integer> (arguments[0]);
            auto primitive = std::make_shared<PrimitiveNumberValueClass> ();
            primitive->value = typename PrimitiveNumberValueClass::ValueType(self->value);
            return primitive;
        });

    environment->addPrimitiveToClass("Integer", conversionMethodName,
        SimpleFunctionType::make(integerType, "self", primitiveType),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Integer> (arguments[0]);
            auto primitive = std::make_shared<PrimitiveNumberValueClass> ();
            primitive->value = typename PrimitiveNumberValueClass::ValueType(self->value);
            return primitive;
        });

    environment->addPrimitiveToClass("Float", literalSuffix,
        SimpleFunctionType::make(floatType, "self", primitiveType),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Float> (arguments[0]);
            auto primitive = std::make_shared<PrimitiveNumberValueClass> ();
            primitive->value = typename PrimitiveNumberValueClass::ValueType(self->value);
            return primitive;
        });

    environment->addPrimitiveToClass("Float", conversionMethodName,
        SimpleFunctionType::make(floatType, "self",primitiveType),
        [](const std::vector<ValuePtr> &arguments) {
            sysmelAssert(arguments.size() == 1);
            auto self = std::static_pointer_cast<Float> (arguments[0]);
            auto primitive = std::make_shared<PrimitiveNumberValueClass> ();
            primitive->value = typename PrimitiveNumberValueClass::ValueType(self->value);
            return primitive;
        });

    auto unaryArithmethicType = SimpleFunctionType::make(primitiveType, "self", primitiveType);
    auto binaryArithmethicType = SimpleFunctionType::make(primitiveType, "self", primitiveType, "other", primitiveType);
    auto binaryComparisonType = SimpleFunctionType::make(primitiveType, "self", primitiveType, "other", environment->lookupValidClass("Boolean"));
    environment->addPrimitiveToType(primitiveType, "negated", unaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = -self->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "+", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value + other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "-", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value - other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "*", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value * other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "/", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value / other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "//", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value / other->value;
            return result;
        });

    environment->addPrimitiveToType(primitiveType, "=", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value == other->value);
        });
    environment->addPrimitiveToType(primitiveType, "~=", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value != other->value);
        });
    environment->addPrimitiveToType(primitiveType, "<", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value < other->value);
        });
    environment->addPrimitiveToType(primitiveType, "<=", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value <= other->value);
        });
    environment->addPrimitiveToType(primitiveType, ">", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value > other->value);
        });
    environment->addPrimitiveToType(primitiveType, ">=", binaryComparisonType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            return Boolean::encode(self->value >= other->value);
        });
}

template<typename PrimitiveNumberTypeClass, typename PrimitiveNumberValueClass>
void buildPrimitiveIntegerTypeMethods(IntrinsicsEnvironment *environment,
    const char *literalSuffix, const char *conversionMethodName)
{
    buildPrimitiveNumberTypeMethods<PrimitiveNumberTypeClass, PrimitiveNumberValueClass>  (environment, literalSuffix, conversionMethodName);

    auto primitiveType = PrimitiveNumberTypeClass::uniqueInstance();
    auto unaryArithmethicType = SimpleFunctionType::make(primitiveType, "self", primitiveType);
    auto binaryArithmethicType = SimpleFunctionType::make(primitiveType, "self", primitiveType, "other", primitiveType);

    environment->addPrimitiveToType(primitiveType, "bitInvert", unaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = ~self->value;
            return result;
        });

    environment->addPrimitiveToType(primitiveType, "%", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value % other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "|", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value | other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "&", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value | other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "^", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value ^ other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, "<<", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value << other->value;
            return result;
        });
    environment->addPrimitiveToType(primitiveType, ">>", binaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto other = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[1]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = self->value >> other->value;
            return result;
        });
}

template<typename PrimitiveNumberTypeClass, typename PrimitiveNumberValueClass>
void buildPrimitiveFloatTypeMethods(IntrinsicsEnvironment *environment,
    const char *literalSuffix, const char *conversionMethodName)
{
    buildPrimitiveNumberTypeMethods<PrimitiveNumberTypeClass, PrimitiveNumberValueClass>  (environment, literalSuffix, conversionMethodName);

    auto primitiveType = PrimitiveNumberTypeClass::uniqueInstance();
    auto unaryArithmethicType = SimpleFunctionType::make(primitiveType, "self", primitiveType);
    environment->addPrimitiveToType(primitiveType, "sqrt", unaryArithmethicType,
        [](const std::vector<ValuePtr> &arguments) {
            auto self  = std::static_pointer_cast<PrimitiveNumberValueClass> (arguments[0]);
            auto result = std::make_shared<PrimitiveNumberValueClass> ();
            result->value = sqrt(self->value);
            return result;
        });
}


void IntrinsicsEnvironment::buildValuePrimitives()
{
    buildPrimitiveIntegerTypeMethods<PrimitiveUInt8Type,  PrimitiveUInt8Value>  (this, "u8", "asUInt8");
    buildPrimitiveIntegerTypeMethods<PrimitiveUInt16Type, PrimitiveUInt16Value> (this, "u16", "asUInt16");
    buildPrimitiveIntegerTypeMethods<PrimitiveUInt32Type, PrimitiveUInt32Value> (this, "u32", "asUInt32");
    buildPrimitiveIntegerTypeMethods<PrimitiveUInt64Type, PrimitiveUInt64Value> (this, "u64", "asUInt64");

    buildPrimitiveIntegerTypeMethods<PrimitiveInt8Type,  PrimitiveInt8Value>  (this, "i8",  "asInt8");
    buildPrimitiveIntegerTypeMethods<PrimitiveInt16Type, PrimitiveInt16Value> (this, "i16", "asInt16");
    buildPrimitiveIntegerTypeMethods<PrimitiveInt32Type, PrimitiveInt32Value> (this, "i32", "asInt32");
    buildPrimitiveIntegerTypeMethods<PrimitiveInt64Type, PrimitiveInt64Value> (this, "i64", "asInt64");

    buildPrimitiveIntegerTypeMethods<PrimitiveChar8Type,  PrimitiveChar8Value>  (this, "c8",  "asChar8");
    buildPrimitiveIntegerTypeMethods<PrimitiveChar16Type, PrimitiveChar16Value> (this, "c16", "asChar16");
    buildPrimitiveIntegerTypeMethods<PrimitiveChar32Type, PrimitiveChar32Value> (this, "c32", "asChar32");

    buildPrimitiveFloatTypeMethods<PrimitiveFloat32Type, PrimitiveFloat32Value> (this, "f32", "asFloat32");
    buildPrimitiveFloatTypeMethods<PrimitiveFloat64Type, PrimitiveFloat64Value> (this, "f64", "asFloat64");

}

void IntrinsicsEnvironment::buildBasicMacros()
{
    addPrimitiveGlobalMacro("if:then:",
        SimpleFunctionType::make(
            lookupValidClass("MacroContext"), "context",
            lookupValidClass("SyntacticValue"), "condition",
            lookupValidClass("SyntacticValue"), "trueCase",
            lookupValidClass("SyntacticValue")),
        [](const MacroContextPtr &context, const std::vector<ValuePtr> &arguments){
            auto syntaxIf = std::make_shared<SyntaxIf> ();
            syntaxIf->sourcePosition = context->sourcePosition;
            syntaxIf->condition = arguments[0];
            syntaxIf->trueCase = arguments[1];
            return syntaxIf;
        });
    addPrimitiveGlobalMacro("if:then:else:",
        SimpleFunctionType::make(
            lookupValidClass("MacroContext"), "context",
            lookupValidClass("SyntacticValue"), "condition",
            lookupValidClass("SyntacticValue"), "trueCase",
            lookupValidClass("SyntacticValue"), "falseCase",
            lookupValidClass("SyntacticValue")),
        [](const MacroContextPtr &context, const std::vector<ValuePtr> &arguments){
            auto syntaxIf = std::make_shared<SyntaxIf> ();
            syntaxIf->sourcePosition = context->sourcePosition;
            syntaxIf->condition = arguments[0];
            syntaxIf->trueCase = arguments[1];
            syntaxIf->falseCase = arguments[2];
            return syntaxIf;
        });
    
    addPrimitiveGlobalMacro("while:do:",
        SimpleFunctionType::make(
            lookupValidClass("MacroContext"), "context",
            lookupValidClass("SyntacticValue"), "condition",
            lookupValidClass("SyntacticValue"), "body",
            lookupValidClass("SyntacticValue")),
        [](const MacroContextPtr &context, const std::vector<ValuePtr> &arguments){
            auto syntaxWhile = std::make_shared<SyntaxWhile> ();
            syntaxWhile->sourcePosition = context->sourcePosition;
            syntaxWhile->condition = arguments[0];
            syntaxWhile->body = arguments[1];
            return syntaxWhile;
        });
    addPrimitiveGlobalMacro("while:do:continueWith:",
        SimpleFunctionType::make(
            lookupValidClass("MacroContext"), "context",
            lookupValidClass("SyntacticValue"), "condition",
            lookupValidClass("SyntacticValue"), "body",
            lookupValidClass("SyntacticValue"), "continueAction",
            lookupValidClass("SyntacticValue")),
        [](const MacroContextPtr &context, const std::vector<ValuePtr> &arguments){
            auto syntaxWhile = std::make_shared<SyntaxWhile> ();
            syntaxWhile->sourcePosition = context->sourcePosition;
            syntaxWhile->condition = arguments[0];
            syntaxWhile->body = arguments[1];
            syntaxWhile->continueAction = arguments[2];
            return syntaxWhile;
        });
}

void IntrinsicsEnvironment::addPrimitiveToType(const TypeBehaviorPtr &behavior, const std::string &selector, ValuePtr functionalType, PrimitiveImplementationSignature impl)
{
    auto primitive = std::make_shared<PrimitiveMethod> (functionalType, impl);
    behavior->methodDict[Symbol::internString(selector)] = primitive;
}

void IntrinsicsEnvironment::addPrimitiveToClass(const std::string &className, const std::string &selector, ValuePtr functionalType, PrimitiveImplementationSignature impl)
{
    auto primitive = std::make_shared<PrimitiveMethod> (functionalType, impl);
    auto &clazz = intrinsicClasses[className];
    clazz->methodDict[Symbol::internString(selector)] = primitive;
}

void IntrinsicsEnvironment::addPrimitiveToMetaclass(const std::string &className, const std::string &selector, ValuePtr functionalType, PrimitiveImplementationSignature impl)
{
    auto primitive = std::make_shared<PrimitiveMethod> (functionalType, impl);
    auto &clazz = intrinsicMetaclasses[className];
    clazz->methodDict[Symbol::internString(selector)] = primitive;
}

void IntrinsicsEnvironment::addPrimitiveGlobalMacro(const std::string &name, ValuePtr functionalType, PrimitiveMacroImplementationSignature impl)
{
    auto primitiveMacro = std::make_shared<PrimitiveMacroMethod> (functionalType, impl);
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

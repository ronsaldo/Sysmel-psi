#ifndef SYSMEL_TYPE_HPP
#define SYSMEL_TYPE_HPP

#include "Value.hpp"
#include <sstream>
#include <vector>

namespace Sysmel
{

typedef std::shared_ptr<class TypeUniverse> TypeUniversePtr;
typedef std::shared_ptr<class GradualType>  GradualTypePtr;
typedef std::shared_ptr<class UnitType>     UnitTypePtr;
typedef std::shared_ptr<class BottomType>   BottomTypePtr;
typedef std::shared_ptr<class VoidValue>    VoidValuePtr;
typedef std::shared_ptr<class VoidType>     VoidTypePtr;
typedef std::shared_ptr<class ProductType>  ProductTypePtr;
typedef std::shared_ptr<class SumType>      SumTypePtr;
typedef std::shared_ptr<class FunctionType> FunctionTypePtr;
typedef std::shared_ptr<class ObjectType>   ObjectTypePtr;
typedef std::shared_ptr<class Symbol>       SymbolPtr;

typedef std::shared_ptr<class PointerLikeType> PointerLikeTypePtr;
typedef std::shared_ptr<class PointerType>     PointerTypePtr;
typedef std::shared_ptr<class ReferenceType>   ReferenceTypePtr;

typedef std::shared_ptr<class PrimitiveUInt8Type>  PrimitiveUInt8TypePtr;
typedef std::shared_ptr<class PrimitiveUInt16Type> PrimitiveUInt16TypePtr;
typedef std::shared_ptr<class PrimitiveUInt32Type> PrimitiveUInt32TypePtr;
typedef std::shared_ptr<class PrimitiveUInt64Type> PrimitiveUInt64TypePtr;

typedef std::shared_ptr<class PrimitiveInt8Type>  PrimitiveInt8TypePtr;
typedef std::shared_ptr<class PrimitiveInt16Type> PrimitiveInt16TypePtr;
typedef std::shared_ptr<class PrimitiveInt32Type> PrimitiveInt32TypePtr;
typedef std::shared_ptr<class PrimitiveInt64Type> PrimitiveInt64TypePtr;

typedef std::shared_ptr<class PrimitiveChar8Type>  PrimitiveChar8TypePtr;
typedef std::shared_ptr<class PrimitiveChar16Type> PrimitiveChar16TypePtr;
typedef std::shared_ptr<class PrimitiveChar32Type> PrimitiveChar32TypePtr;

typedef std::shared_ptr<class PrimitiveFloat32Type> PrimitiveFloat32TypePtr;
typedef std::shared_ptr<class PrimitiveFloat64Type> PrimitiveFloat64TypePtr;


class TypeBehavior : public Value
{
public:
    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments) override
    {
        auto method = lookupSelector(selector);
        if (!method)
        {
            receiver->throwExceptionWithMessage(("Failed to find method " + selector->printString() + " in " + receiver->getType()->printString()).c_str());
        }

        std::vector<ValuePtr> allArguments;
        allArguments.reserve(1 + arguments.size());
        allArguments.push_back(receiver);
        for (auto &arg : arguments)
            allArguments.push_back(arg);

        return method->applyWithArguments(allArguments);
    }

    virtual ValuePtr lookupSelector(const ValuePtr &selector) override
    {
        auto it = methodDict.find(selector);
        return it != methodDict.end() ? it->second : nullptr;
    }

    virtual ValuePtr asTypeValue() { return shared_from_this(); }

    std::map<ValuePtr, ValuePtr> methodDict;
};

class TypeUniverse : public TypeBehavior
{
public:
    TypeUniverse(int index = 0)
        : universeIndex(index) {}

    virtual bool isType() const override { return true; }

    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Type(" << universeIndex << ")";
    }

    int universeIndex = 0;

    static TypeUniversePtr uniqueInstanceForIndex(int index);

private:
    static std::vector<TypeUniversePtr> uniqueInstances;
};

class Type : public TypeBehavior
{
public:
    virtual bool isType() const override { return true; }
    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Type";
    }

    static TypePtr uniqueInstance();

private:
    static TypePtr singletonValue;
};

class BasicType : public TypeBehavior
{
public:
    virtual ValuePtr getType() const override;
};

class UnitType : public BasicType
{
public:
    virtual ValuePtr asTypeValue() { return shared_from_this(); }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "UnitType";
    }

    static UnitTypePtr uniqueInstance();
private:
    static UnitTypePtr singletonValue;
};

class BottomType : public BasicType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "BottomType";
    }

    static BottomTypePtr uniqueInstance();
private:
    static BottomTypePtr singletonValue;
};

class VoidType : public BasicType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Void";
    }

    static VoidTypePtr uniqueInstance();
private:
    static VoidTypePtr singletonValue;
};

class VoidValue : public Value
{
public:
    virtual ValuePtr getType() const override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "void";
    }

    static VoidValuePtr uniqueInstance();
private:
    static VoidValuePtr singletonValue;
};

class GradualType : public BasicType
{
public:
    virtual ValuePtr asTypeValue() { return shared_from_this(); }

    virtual bool isType() const override { return true; }
    virtual bool isGradualType() const {return true;}
    virtual ValuePtr getType();

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "?";
    }
    
    static GradualTypePtr uniqueInstance();
private:
    static GradualTypePtr singletonValue;
};

class ProductTypeValue : public Value
{
public:
    virtual ValuePtr getType() const override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "(";
        bool isFirst = true;
        for (const auto &element : elements)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            element->printStringOn(out);
        }
        out << ")";
    }
    
    ProductTypePtr type;
    std::vector<ValuePtr> elements;
};

class ProductType : public TypeBehavior
{
public:
    static ProductTypePtr getOrCreateWithElementTypes(const std::vector<ValuePtr> &elements);

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '(';
        bool isFirst = true;
        for(auto &element : elementTypes)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            element->printStringOn(out);
        }
        out << ')';
    }

    std::vector<ValuePtr> elementTypes;
private:
    static std::map<std::vector<ValuePtr>, ProductTypePtr> ProductTypeCache;
};

class SumTypeValue : public Value
{
public:
    virtual ValuePtr getType() const override;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "(" << caseIndex << ": ";
        element->printStringOn(out);
        out << ")";
    }
    
    SumTypePtr type;
    uint32_t caseIndex;
    ValuePtr element;
};

class SumType : public TypeBehavior
{
public:
    static SumTypePtr getOrCreateWithAlternativeTypes(const std::vector<ValuePtr> &elements);

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '(';
        bool isFirst = true;
        for(auto &alternative : alternativeTypes)
        {
            if(isFirst)
                isFirst = false;
            else
                out << " | ";
            alternative->printStringOn(out);
        }
        out << ')';
    }

    std::vector<ValuePtr> alternativeTypes;
private:
    static std::map<std::vector<ValuePtr>, SumTypePtr> SumTypeCache;
};

class PiType : public TypeBehavior
{
public:
    virtual void printStringOn(std::ostream &out) const override;

    ValuePtr reduce();

    ValuePtr nameExpression;
    std::vector<SymbolArgumentBindingPtr> arguments;
    ValuePtr resultType;
};


typedef std::shared_ptr<class SimpleFunctionType> SimpleFunctionTypePtr;
class SimpleFunctionType : public TypeBehavior
{
public:
    virtual void printStringOn(std::ostream &out) const override;
    virtual ArgumentTypeAnalysisContextPtr createArgumentTypeAnalysisContext();

    std::vector<ValuePtr> argumentTypes;
    std::vector<SymbolPtr> argumentNames;
    ValuePtr resultType;

    static SimpleFunctionTypePtr make(const ValuePtr &resultType)
    {
        auto functionalType = std::make_shared<SimpleFunctionType> ();
        functionalType->resultType = resultType;
        return functionalType;
    }

    static SimpleFunctionTypePtr make(const ValuePtr &arg0Type, const std::string &arg0Name, const ValuePtr &resultType)
    {
        auto functionalType = std::make_shared<SimpleFunctionType> ();
        functionalType->argumentTypes.push_back(arg0Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg0Name));
        functionalType->resultType = resultType;
        return functionalType;
    }

    static SimpleFunctionTypePtr make(
        const ValuePtr &arg0Type, const std::string &arg0Name,
        const ValuePtr &arg1Type, const std::string &arg1Name,
        const ValuePtr &resultType)
    {
        auto functionalType = std::make_shared<SimpleFunctionType> ();
        functionalType->argumentTypes.push_back(arg0Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg0Name));
        functionalType->argumentTypes.push_back(arg1Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg1Name));
        functionalType->resultType = resultType;
        return functionalType;
    }

    static SimpleFunctionTypePtr make(
        const ValuePtr &arg0Type, const std::string &arg0Name,
        const ValuePtr &arg1Type, const std::string &arg1Name,
        const ValuePtr &arg2Type, const std::string &arg2Name,
        const ValuePtr &resultType)
    {
        auto functionalType = std::make_shared<SimpleFunctionType> ();
        functionalType->argumentTypes.push_back(arg0Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg0Name));
        functionalType->argumentTypes.push_back(arg1Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg1Name));
        functionalType->argumentTypes.push_back(arg2Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg2Name));
        functionalType->resultType = resultType;
        return functionalType;
    }

    static SimpleFunctionTypePtr make(
        const ValuePtr &arg0Type, const std::string &arg0Name,
        const ValuePtr &arg1Type, const std::string &arg1Name,
        const ValuePtr &arg2Type, const std::string &arg2Name,
        const ValuePtr &arg3Type, const std::string &arg3Name,
        const ValuePtr &resultType)
    {
        auto functionalType = std::make_shared<SimpleFunctionType> ();
        functionalType->argumentTypes.push_back(arg0Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg0Name));
        functionalType->argumentTypes.push_back(arg1Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg1Name));
        functionalType->argumentTypes.push_back(arg2Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg2Name));
        functionalType->argumentTypes.push_back(arg3Type);
        functionalType->argumentNames.push_back(Symbol::internString(arg3Name));
        functionalType->resultType = resultType;
        return functionalType;
    }
};

typedef std::shared_ptr<class SimpleFunctionType> SimpleFunctionTypePtr;
class PrimitiveType : public TypeBehavior
{
public:
};

class PointerLikeType : public PrimitiveType
{
public:
    virtual bool isPointerLikeType() const override
    {
        return true;
    }

    ValuePtr baseType;
};

class PointerType : public PointerLikeType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        baseType->printStringOn(out);
        out << " pointer";
    }

    static PointerTypePtr make(ValuePtr baseType);

private:
    static std::map<ValuePtr, PointerTypePtr> PointerTypeCache;
};

class ReferenceLikeType : public PointerLikeType
{
public:
    virtual ValuePtr getDecayedType() override
    {
        return baseType;
    }

    virtual bool isReferenceLikeType() const override
    {
        return true;
    }

    virtual ValuePtr analyzeSyntaxMessageSendOfInstance(const SyntaxMessageSendPtr &messageSend, const EnvironmentPtr &environment, const ValuePtr &analyzedReceiver, const ValuePtr &analyzedSelector);
};

class ReferenceType : public ReferenceLikeType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        baseType->printStringOn(out);
        out << " ref";
    }

    static ReferenceTypePtr make(ValuePtr baseType);

private:
    static std::map<ValuePtr, ReferenceTypePtr> ReferenceTypeCache;
};

class PrimitiveNumberType : public PrimitiveType
{
public:
    virtual bool isSigned() const = 0;
    virtual bool isCharacter() const = 0;
    virtual bool isFloatingPoint() const = 0;
    virtual size_t alignment() const = 0;
    virtual size_t size() const = 0;

};

class PrimitiveValue : public Value 
{
public:
};

class PrimitiveUInt8Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "UInt8";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 1; };
    virtual size_t size() const override          { return 1; };

    static PrimitiveUInt8TypePtr uniqueInstance();
private:
    static PrimitiveUInt8TypePtr singleton;
};

class PrimitiveUInt8Value : public PrimitiveValue 
{
public:
    typedef uint8_t ValueType;
    virtual void printStringOn(std::ostream &out) const override
    {
        out << uint32_t(value) << "u8";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveUInt8Type::uniqueInstance();
    }

    uint8_t value;
};

class PrimitiveUInt16Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "UInt16";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 2; };
    virtual size_t size() const override          { return 2; };

    static PrimitiveUInt16TypePtr uniqueInstance();
private:
    static PrimitiveUInt16TypePtr singleton;
};

class PrimitiveUInt16Value : public PrimitiveValue 
{
public:
    typedef uint16_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "u16";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveUInt16Type::uniqueInstance();
    }

    uint16_t value;
};

class PrimitiveUInt32Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "UInt32";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 4; };
    virtual size_t size() const override          { return 4; };

    static PrimitiveUInt32TypePtr uniqueInstance();
private:
    static PrimitiveUInt32TypePtr singleton;
};

class PrimitiveUInt32Value : public PrimitiveValue 
{
public:
    typedef uint32_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "u32";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveUInt32Type::uniqueInstance();
    }

    uint32_t value;
};

class PrimitiveUInt64Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "UInt64";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 8; };
    virtual size_t size() const override          { return 8; };

    static PrimitiveUInt64TypePtr uniqueInstance();
private:
    static PrimitiveUInt64TypePtr singleton;
};

class PrimitiveUInt64Value : public PrimitiveValue 
{
public:
    typedef uint64_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "u64";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveUInt64Type::uniqueInstance();
    }

    uint64_t value;
};

class PrimitiveInt8Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Int8";
    }

    virtual bool isSigned() const override        { return true; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 1; };
    virtual size_t size() const override          { return 1; };

    static PrimitiveInt8TypePtr uniqueInstance();
private:
    static PrimitiveInt8TypePtr singleton;
};

class PrimitiveInt8Value : public PrimitiveValue 
{
public:
    typedef int8_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << int32_t(value) << "i8";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveInt8Type::uniqueInstance();
    }

    int8_t value;
};

class PrimitiveInt16Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Int16";
    }

    virtual bool isSigned() const override        { return true; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 2; };
    virtual size_t size() const override          { return 2; };

    static PrimitiveInt16TypePtr uniqueInstance();
private:
    static PrimitiveInt16TypePtr singleton;
};

class PrimitiveInt16Value : public PrimitiveValue 
{
public:
    typedef int16_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "i16";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveInt16Type::uniqueInstance();
    }

    int16_t value;
};

class PrimitiveInt32Type : public PrimitiveNumberType
{
public:

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Int32";
    }

    virtual bool isSigned() const override        { return true; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 4; };
    virtual size_t size() const override          { return 4; };

    static PrimitiveInt32TypePtr uniqueInstance();
private:
    static PrimitiveInt32TypePtr singleton;
};

class PrimitiveInt32Value : public PrimitiveValue 
{
public:
    typedef int32_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "i32";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveInt32Type::uniqueInstance();
    }

    int32_t value;
};

class PrimitiveInt64Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Int64";
    }

    virtual bool isSigned() const override        { return true; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 8; };
    virtual size_t size() const override          { return 8; };

    static PrimitiveInt64TypePtr uniqueInstance();
private:
    static PrimitiveInt64TypePtr singleton;
};

class PrimitiveInt64Value : public PrimitiveValue 
{
public:
    typedef int64_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "i64";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveInt64Type::uniqueInstance();
    }

    int64_t value;
};

class PrimitiveChar8Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Char8";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return true; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 1; };
    virtual size_t size() const override          { return 1; };

    static PrimitiveChar8TypePtr uniqueInstance();
private:
    static PrimitiveChar8TypePtr singleton;
};

class PrimitiveChar8Value : public PrimitiveValue 
{
public:
    typedef char ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "c8";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveChar8Type::uniqueInstance();
    }

    char value;
};

class PrimitiveChar16Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Char16";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return true; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 2; };
    virtual size_t size() const override          { return 2; };

    static PrimitiveChar16TypePtr uniqueInstance();
private:
    static PrimitiveChar16TypePtr singleton;
};

class PrimitiveChar16Value : public PrimitiveValue 
{
public:
    typedef char16_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "c16";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveChar16Type::uniqueInstance();
    }

    char16_t value;
};

class PrimitiveChar32Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Char32";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return true; };
    virtual bool isFloatingPoint() const override { return false; };
    virtual size_t alignment() const override     { return 4; };
    virtual size_t size() const override          { return 4; };

    static PrimitiveChar32TypePtr uniqueInstance();
private:
    static PrimitiveChar32TypePtr singleton;
};

class PrimitiveChar32Value : public PrimitiveValue 
{
public:
    typedef char32_t ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "c32";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveChar32Type::uniqueInstance();
    }

    char32_t value;
};


class PrimitiveFloat32Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Float32";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return true; };
    virtual size_t alignment() const override     { return 4; };
    virtual size_t size() const override          { return 4; };

    static PrimitiveFloat32TypePtr uniqueInstance();
private:
    static PrimitiveFloat32TypePtr singleton;
};

class PrimitiveFloat32Value : public PrimitiveValue 
{
public:
    typedef float ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "f32";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveFloat32Type::uniqueInstance();
    }

    float value;
};

class PrimitiveFloat64Type : public PrimitiveNumberType
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "Float64";
    }

    virtual bool isSigned() const override        { return false; };
    virtual bool isCharacter() const override     { return false; };
    virtual bool isFloatingPoint() const override { return true; };
    virtual size_t alignment() const override     { return 8; };
    virtual size_t size() const override          { return 8; };

    static PrimitiveFloat64TypePtr uniqueInstance();
private:
    static PrimitiveFloat64TypePtr singleton;
};

class PrimitiveFloat64Value : public PrimitiveValue 
{
public:
    typedef double ValueType;

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value << "f64";
    }

    virtual ValuePtr getType() const override
    {
        return PrimitiveFloat64Type::uniqueInstance();
    }

    double value;
};

}// End of namespace Sysmel


#endif
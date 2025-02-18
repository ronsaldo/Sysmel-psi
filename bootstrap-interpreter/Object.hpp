#ifndef SYSMEL_OBJECT_HPP
#define SYSMEL_OBJECT_HPP

#include "Value.hpp"
#include "LargeInteger.hpp"
#include "stdio.h"
#include <map>
#include <functional>

namespace Sysmel
{

typedef std::shared_ptr<class ProtoObject> ProtoObjectPtr;
typedef std::shared_ptr<class Object> ObjectPtr;
typedef std::shared_ptr<class UndefinedObject> UndefinedObjectPtr;
typedef std::shared_ptr<class True> TruePtr;
typedef std::shared_ptr<class False> FalsePtr;
typedef std::shared_ptr<class Class> ClassPtr;
typedef std::shared_ptr<class Metaclass> MetaclassPtr;
typedef std::shared_ptr<class Symbol> SymbolPtr;
typedef std::shared_ptr<class BinaryStream> BinaryStreamPtr;
typedef std::shared_ptr<class BinaryFileStream> BinaryFileStreamPtr;
typedef std::shared_ptr<class MacroContext> MacroContextPtr;
typedef std::shared_ptr<class Array> ArrayPtr;
typedef std::shared_ptr<class OrderedCollection> OrderedCollectionPtr;
typedef std::function<ValuePtr(const std::vector<ValuePtr> &arguments)> PrimitiveImplementationSignature;
typedef std::function<ValuePtr(const MacroContextPtr &context, const std::vector<ValuePtr> &arguments)> PrimitiveMacroImplementationSignature;

class ProtoObject : public Value
{
public:
    virtual const char *getClassName() const { return "ProtoObject"; }
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "a ";
        getClass()->printStringOn(out);
    }

    virtual ValuePtr getClass() const override;

    ValuePtr getClassOrType() const override
    {
        return getClass();
    }

    ValuePtr getTypeOrClass() const override
    {
        return getClass();
    }

    virtual ValuePtr asTypeValue() { return shared_from_this(); }

    virtual bool isSatisfiedByType(const ValuePtr &sourceType)
    {
        auto myClass = getClass();
        // TODO: Add gradual check here
        auto otherClass = sourceType->getClass();
        //printf("%s class: %s. sourceType %s\n", myClass->printString().c_str(), printString().c_str(), sourceType->printString().c_str());
        if (!otherClass)
            return false;

        //return otherClass->isSubclassOf(myClass);
        return otherClass->isSubclassOf(myClass);
    }

    mutable ValuePtr clazz;
    size_t identityHash;
};

class Object : public ProtoObject
{
public:
    virtual const char *getClassName() const override { return "Object"; }
};

class UndefinedObject : public Object
{
public:
    virtual const char *getClassName() const override { return "UndefinedObject"; }
    virtual bool isNil() const { return true; };
    static UndefinedObjectPtr uniqueInstance();

private:
    static UndefinedObjectPtr singleton;
};

class Behavior : public Object
{
public:
    virtual const char *getClassName() const override { return "Behavior"; }

    virtual ValuePtr basicNew()
    {
        if(interpreterBasicNew)
            return interpreterBasicNew();
        abort();
    }

    virtual bool isSatisfiedByType(const ValuePtr &sourceType) override
    {
        auto myClass = shared_from_this();
        // TODO: Add gradual check here
        auto otherClass = sourceType->asTypeValue();
        //printf("%s class: %s. sourceType %s\n", myClass->printString().c_str(), printString().c_str(), sourceType->printString().c_str());
        if (!otherClass)
            return false;

        //return otherClass->isSubclassOf(myClass);
        return otherClass->isSubclassOf(myClass);
    }

    virtual ValuePtr performWithArgumentsOnInstance(const ValuePtr &receiver, const ValuePtr &selector, const std::vector<ValuePtr> &arguments) override
    {
        auto method = receiver->getClass()->lookupSelector(selector);
        if (!method)
        {
            receiver->throwExceptionWithMessage(("Failed to find method " + selector->printString() + " in " + receiver->getClass()->printString()).c_str());
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
        auto it = methodDict.find(std::static_pointer_cast<Symbol>(selector));
        if (it != methodDict.end())
            return it->second;
        if (superclass)
            return superclass->lookupSelector(selector);
        return nullptr;
    }

    virtual bool isSubclassOf(const ValuePtr &targetSuperclass) override
    {
        //printf("target %s\n", targetSuperclass->printString().c_str());
        auto currentBehavior = std::static_pointer_cast<Behavior>(shared_from_this());
        while (currentBehavior && !currentBehavior->isNil())
        {
            //printf("cur %s\n", currentBehavior->printString().c_str());
            if (currentBehavior == targetSuperclass)
                return true;

            currentBehavior = std::static_pointer_cast<Behavior>(currentBehavior->superclass);
        }

        return false;
    }

    ValuePtr superclass;
    std::map<SymbolPtr, ValuePtr> methodDict;
    uint32_t format = 0;

    std::function<ValuePtr ()> interpreterBasicNew;
};

class ClassDescription : public Behavior
{
public:
    virtual const char *getClassName() const override { return "ClassDescription"; }
};

class Class : public ClassDescription
{
public:
    Class()
    {
        subclasses = std::make_shared<Array> ();
    }

    virtual const char *getClassName() const override { return "Class"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << name;
    }

    void registerInSuperclass()
    {
        if(superclass)
            superclass->addSubclass(shared_from_this());
    }

    virtual void addSubclass(const ValuePtr &subclass) override;

    ArrayPtr subclasses;
    std::string name;
};

class Metaclass : public ClassDescription
{
public:
    virtual const char *getClassName() const override { return "Metaclass"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        auto clazz = thisClass.lock();
        if (clazz)
        {
            clazz->printStringOn(out);
            out << " class";
        }
        else
        {
            out << "a Metaclass";
        }
    }

    ClassWeakPtr thisClass;
};

class Boolean : public Object
{
public:
    virtual const char *getClassName() const override { return "Boolean"; }

    static ValuePtr encode(bool value);
};

class True : public Object
{
public:
    virtual const char *getClassName() const override { return "True"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "true";
    }

    virtual bool isTrue() const { return true; };

    static TruePtr uniqueInstance();
private:
    static TruePtr singleton;
};

class False : public Object
{
public:
    virtual const char *getClassName() const override { return "False"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "false";
    }

    virtual bool isFalse() const { return true; };

    static FalsePtr uniqueInstance();
private:
    static FalsePtr singleton;
};

class PrimitiveMethod : public Object
{
public:
    PrimitiveMethod() = default;
    PrimitiveMethod(ValuePtr ctype, PrimitiveImplementationSignature cimplementation)
        : type(ctype), implementation(cimplementation) {}

    virtual const char *getClassName() const override { return "PrimitiveMethod"; }
    virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments) override;
    virtual ValuePtr getType() const override
    {
        return type;
    }

    ValuePtr type;
    PrimitiveImplementationSignature implementation;
};

class MacroContext : public Object
{
public:
    EnvironmentPtr environment;
};

class PrimitiveMacroMethod : public Object
{
public:
    PrimitiveMacroMethod() = default;
    PrimitiveMacroMethod(ValuePtr ctype, PrimitiveMacroImplementationSignature cimplementation)
        : type(ctype), implementation(cimplementation) {}

    virtual const char *getClassName() const override { return "PrimitiveMacroMethod"; }
    virtual bool isMacro() const override { return true;};
    virtual ValuePtr applyMacroWithContextAndArguments(const MacroContextPtr &context, const std::vector<ValuePtr> &arguments) override;

    virtual ValuePtr getType() const override
    {
        return type;
    }

    ValuePtr type;
    PrimitiveMacroImplementationSignature implementation;
};

class Magnitude : public Object
{
public:
    virtual const char *getClassName() const override { return "Magnitude"; }
};

class Character : public Magnitude
{
public:
    virtual const char *getClassName() const override { return "Character"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << char(value);
    }

    virtual std::pair<size_t, const uint8_t *> getBinaryContentsData() const override
    {
        return std::make_pair(4, reinterpret_cast<const uint8_t *>(&value));
    }

    virtual uint8_t evaluateAsSingleByte()
    {
        return uint8_t(value);
    }

    char32_t value;
};

class Number : public Magnitude
{
public:
    virtual const char *getClassName() const override { return "Number"; }
};

class Integer : public Number
{
public:
    virtual const char *getClassName() const override { return "Integer"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value;
    }

    virtual std::pair<size_t, const uint8_t *> getBinaryContentsData() const override
    {
        return std::make_pair(value.words.size() * 4, reinterpret_cast<const uint8_t *>(value.words.data()));
    }

    virtual uint8_t evaluateAsSingleByte()
    {
        auto firstWord = value.wordAt(0);
        if(value.signBit)
            firstWord = -firstWord;

        return firstWord & 0xFF;
    }

    virtual size_t evaluateAsIndex()
    {
        size_t index = size_t(value.wordAt(0)) | (size_t(value.wordAt(1)) << 32);
        if (value.signBit)
            index = -index;
        return index;
    }

    LargeInteger value;
};

class Float : public Number
{
public:
    virtual const char *getClassName() const override { return "Float"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << value;
    }

    double value;
};

class Collection : public Object
{
public:
    virtual const char *getClassName() const override { return "Collection"; }

    virtual size_t getSize() const
    {
        return 0;
    }
};

class SequenceableCollection : public Collection
{
public:
    virtual const char *getClassName() const override { return "SequenceableCollection"; }
};

class ArrayedCollection : public Collection
{
public:
    virtual const char *getClassName() const override { return "ArrayedCollection"; }
};

class Array : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override { return "Array"; }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '[';
        bool first = true;
        for (auto &value : values)
        {
            if (first)
                first = false;
            else
                out << ". ";
            value->printStringOn(out);
        }
        out << ']';
    }

    virtual size_t getSize() const
    {
        return values.size();
    }

    virtual ValuePtr getElementAtIndex(size_t index) override
    {
        if(index >= values.size())
            throwExceptionWithMessage("Index is out of bounds.");
        return values[index];
    }

    virtual ValuePtr setElementAtIndex(size_t index, const ValuePtr &value)override
    {
        if(index >= values.size())
            throwExceptionWithMessage("Index is out of bounds.");
        return values[index] = value;
    }

    ArrayPtr copyWith(const ValuePtr &extraValue)
    {
        for(auto &value : values)
        {
            if(value == extraValue)
                return std::static_pointer_cast<Array> (shared_from_this());
        }

        auto result = std::make_shared<Array> ();
        result->values = values;
        result->values.push_back(extraValue);
        return result;
    }
    
    std::vector<ValuePtr> values;
};

class ByteArray : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override { return "ByteArray"; }

    virtual std::pair<size_t, const uint8_t *> getBinaryContentsData() const override
    {
        return std::make_pair(values.size(), values.data());
    }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "#[";
        bool first = true;
        for (auto &value : values)
        {
            if (first)
                first = false;
            else
                out << ". ";
            out << int(value);
        }
        out << ']';
    }

    virtual size_t getSize() const
    {
        return values.size();
    }

    virtual ValuePtr getElementAtIndex(size_t index) override
    {
        if(index >= values.size())
            throwExceptionWithMessage("Index is out of bounds.");
        
        auto integer = std::make_shared<Integer> ();
        integer->value = LargeInteger(values[index]);
        return integer;
    }

    virtual ValuePtr setElementAtIndex(size_t index, const ValuePtr &value) override
    {
        if(index >= values.size())
            throwExceptionWithMessage("Index is out of bounds.");
        values[index] = value->evaluateAsSingleByte();
        return value;
    }

    std::vector<uint8_t> values;
};

class String : public ArrayedCollection
{
public:
    virtual const char *getClassName() const override { return "String"; }

    virtual std::pair<size_t, const uint8_t *> getBinaryContentsData() const override
    {
        return std::make_pair(value.size(), reinterpret_cast<const uint8_t *>(value.data()));
    }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << '"' << value << '"';
    }

    virtual size_t getSize() const
    {
        return value.size();
    }

    virtual ValuePtr getElementAtIndex(size_t index) override
    {
        if(index >= value.size())
            throwExceptionWithMessage("Index is out of bounds.");
        
        auto character = std::make_shared<Character> ();
        character->value = value[index];
        return character;
    }

    virtual ValuePtr setElementAtIndex(size_t index, const ValuePtr &newValue) override
    {
        if(index >= value.size())
            throwExceptionWithMessage("Index is out of bounds.");
        value[index] = newValue->evaluateAsSingleByte();
        return shared_from_this();
    }

    std::string value;
};

class Symbol : public String
{
public:
    virtual const char *getClassName() const override { return "Symbol"; }
    static SymbolPtr internString(const std::string &string);

    virtual SymbolPtr asAnalyzedSymbolValue() override
    {
        return std::static_pointer_cast<Symbol>(shared_from_this());
    }

    virtual void printStringOn(std::ostream &out) const override
    {
        out << "#\"" << value << '"';
    }

    virtual ValuePtr setElementAtIndex(size_t index, const ValuePtr &newValue) override
    {
        (void)index;
        (void)newValue;
        throwExceptionWithMessage("Symbol mutation is forbidden.");
    }

    virtual bool isSymbolWithValue(const char *expectedValue) override
    {
        return value == expectedValue;
    }

private:
    static std::map<std::string, SymbolPtr> internedSymbols;
};

class Stream : public Object
{
public:
    virtual const char *getClassName() const override { return "Stream"; }

    virtual void nextPut(const ValuePtr &data)
    {
        (void)data;
        // By default do nothing.
    }

    virtual void nextPutAll(const ValuePtr &data)
    {
        (void)data;
        // By default do nothing.
    }
};

class AbstractBinaryStream : public Stream
{
public:
    virtual const char *getClassName() const override { return "AbstractBinaryStream"; }

    virtual void nextPut(const ValuePtr &data)
    {
        auto binaryData = data->getBinaryContentsData();
        if (binaryData.first > 0)
            nextPutBytes(1, binaryData.second);
    }

    virtual void nextPutAll(const ValuePtr &data)
    {
        auto binaryData = data->getBinaryContentsData();
        nextPutBytes(binaryData.first, binaryData.second);
    }

    virtual void nextPutBytes(size_t size, const uint8_t *bytes)
    {
        (void)size;
        (void)bytes;
    }
};

class BinaryStream : public AbstractBinaryStream
{
public:
    virtual const char *getClassName() const override { return "BinaryStream"; }

    virtual void nextPutBytes(size_t size, const uint8_t *bytes)
    {
        data.reserve(size);
        data.insert(data.end(), bytes, bytes + size);
    }

    std::vector<uint8_t> data;
};

class BinaryFileStream : public BinaryStream
{
public:
    ~BinaryFileStream()
    {
        if (file && ownsFile)
            fclose(file);
    }

    virtual const char *getClassName() const override { return "BinaryFileStream"; }

    virtual void nextPutBytes(size_t size, const uint8_t *bytes)
    {
        fwrite(bytes, size, 1, file);
    }

    FILE *file = nullptr;
    bool ownsFile = true;
};

class Stdio : public Object
{
public:
    virtual const char *getClassName() const override { return "Stdio"; }

    static BinaryFileStreamPtr getValidStdinStream();
    static BinaryFileStreamPtr getValidStdoutStream();
    static BinaryFileStreamPtr getValidStderrStream();

    static BinaryFileStreamPtr stdinStream;
    static BinaryFileStreamPtr stdoutStream;
    static BinaryFileStreamPtr stderrStream;
};

class TestCase : public Object
{
public:
    virtual const char *getClassName() const override { return "TestCase"; }

    virtual void run();
};

} // End of namespace Sysmel

#endif // SYSMEL_OBJECT_HPP
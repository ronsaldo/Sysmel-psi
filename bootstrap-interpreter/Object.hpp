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
    typedef std::shared_ptr<class Class> ClassPtr;
    typedef std::shared_ptr<class Metaclass> MetaclassPtr;
    typedef std::shared_ptr<class Symbol> SymbolPtr;
    typedef std::shared_ptr<class BinaryStream> BinaryStreamPtr;
    typedef std::shared_ptr<class BinaryFileStream> BinaryFileStreamPtr;
    typedef std::function<ValuePtr(const std::vector<ValuePtr> &arguments)> PrimitiveImplementationSignature;

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

        ValuePtr getTypeOrClass() const override
        {
            return getClass();
        }

        virtual bool isSatisfiedByType(const ValuePtr &sourceType)
        {
            auto myClass = getClass();
            // TODO: Add gradual check here
            auto otherClass = sourceType->getClass();
            if (!otherClass)
                return false;

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

        static UndefinedObjectPtr uniqueInstance();

    private:
        static UndefinedObjectPtr singleton;
    };

    class Behavior : public Object
    {
    public:
        virtual const char *getClassName() const override { return "Behavior"; }

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
            auto currentBehavior = std::static_pointer_cast<Behavior>(shared_from_this());
            while (currentBehavior)
            {
                if (currentBehavior == targetSuperclass)
                    return true;

                currentBehavior = std::static_pointer_cast<Behavior>(currentBehavior->superclass);
            }

            return false;
        }

        ValuePtr superclass;
        std::map<SymbolPtr, ValuePtr> methodDict;
    };

    class ClassDescription : public Behavior
    {
    public:
        virtual const char *getClassName() const override { return "ClassDescription"; }
    };

    class Class : public ClassDescription
    {
    public:
        virtual const char *getClassName() const override { return "Class"; }

        virtual void printStringOn(std::ostream &out) const override
        {
            out << name;
        }

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

    class PrimitiveMethod : public Object
    {
    public:
        PrimitiveMethod(PrimitiveImplementationSignature cimplementation)
            : implementation(cimplementation) {}

        virtual const char *getClassName() const override { return "PrimitiveMethod"; }
        virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments) override;

        PrimitiveImplementationSignature implementation;
    };

    class CompiledMethod : public Object
    {
    public:
        virtual const char *getClassName() const override { return "CompiledMethod"; }
        virtual ValuePtr applyWithArguments(const std::vector<ValuePtr> &arguments) override;
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

} // End of namespace Sysmel

#endif // SYSMEL_OBJECT_HPP
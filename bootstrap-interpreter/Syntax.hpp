#ifndef SYSMEL_SYNTAX_HPP
#define SYSMEL_SYNTAX_HPP

#pragma once

#include "Object.hpp"
#include "Source.hpp"
#include "LargeInteger.hpp"
#include "Scanner.hpp"
#include <vector>
#include <sstream>

namespace Sysmel
{

class SyntacticValue : public Object
{
public:
    virtual bool isSyntacticValue() const override {return true;}
    virtual void printStringOn(std::ostream &out) const override {out << "SyntacticValue";}
    virtual SourcePositionPtr getSourcePosition() const override {return sourcePosition;}

    SourcePositionPtr sourcePosition;
};

class SyntaxValueSequence : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxValueSequence(";
        for(size_t i = 0; i < elements.size(); ++i)
        {
            if(i > 0)
                out << ". ";
            elements[i]->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        for(auto &element : elements)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    std::vector<ValuePtr> elements;
};

class SyntaxAssociation : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxAssociation(";
        if(key)
            key->printStringOn(out);
        out << " : ";
        if(value)
            value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(key)
        {
            function(key);
            key->traverseChildren(function);
        }

        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr key;
    ValuePtr value;
};

class SyntaxBindableName : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxBindableName(";
        if(typeExpression)
            typeExpression->printStringOn(out);
        if(nameExpression)
        {
            if(typeExpression)
                out << ", "; 
            nameExpression->printStringOn(out);
        }

        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(typeExpression)
        {
            function(typeExpression);
            typeExpression->traverseChildren(function);
        }

        if(nameExpression)
        {
            function(nameExpression);
            nameExpression->traverseChildren(function);
        }
    }

    ValuePtr typeExpression;
    ValuePtr nameExpression;
    bool isImplicit = false;
    bool isExistential = false;
    bool isVariadic = false;
    bool isMutable = false;
    bool hasPostTypeExpression = false;
};

class SyntaxDictionary : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxDictionary(";
        bool isFirst = true;
        for (auto &element : elements)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ". ";
            element->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        for(const auto &element : elements)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    std::vector<ValuePtr> elements;
};

class SyntaxTuple : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxTuple(";
        for(size_t i = 0; i < elements.size(); ++i)
        {
            if(i > 0)
                out << ". ";
            elements[i]->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        for(const auto &element : elements)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    std::vector<ValuePtr> elements;
};


class SyntaxError : public SyntacticValue
{
public:
    virtual bool isSyntaxError() const override
    {
        return true;
    }
    
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxError(" << errorMessage;
        if (innerNode)
        {
            out << ": ";
             innerNode->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(innerNode)
        {
            function(innerNode);
            innerNode->traverseChildren(function);
        }
    }

    std::string errorMessage;
    ValuePtr innerNode;
};

class SyntaxIdentifierReference : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxIdentifierReference(" << value << ")";
    }

    std::string value;    
};

class SyntaxFunctionalDependentType : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxFunctionalDependentType(";
        if (argumentPattern)
            out << argumentPattern->printString();
        if (resultType)
            out << " :: " <<  resultType->printString();
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(argumentPattern)
        {
            function(argumentPattern);
            argumentPattern->traverseChildren(function);
        }
        if(resultType)
        {
            function(resultType);
            resultType->traverseChildren(function);
        }
    }

    ValuePtr argumentPattern;
    ValuePtr resultType;
};

typedef std::shared_ptr<SyntaxFunctionalDependentType> SyntaxFunctionalDependentTypePtr;

class SyntaxAssignment : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxAssignment(";
        store->printStringOn(out);
        out << " := ";
        value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(store)
        {
            function(store);
            store->traverseChildren(function);
        }
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr store;
    ValuePtr value;
};

class SyntaxBindPattern : public SyntacticValue
{
public:

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(pattern)
        {
            function(pattern);
            pattern->traverseChildren(function);
        }
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr pattern;
    ValuePtr value;
};

class SyntaxBlock : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxBlock(";
        if(functionType)
            out << functionType->printString() << ", ";
        out << body->printString() << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(functionType)
        {
            function(functionType);
            functionType->traverseChildren(function);
        }
        if(body)
        {
            function(body);
            body->traverseChildren(function);
        }
    }

    ValuePtr functionType;
    ValuePtr body;
};

class SyntaxLexicalBlock : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLexicalBlock(";
        if(body)
            out << body->printString();
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(body)
        {
            function(body);
            body->traverseChildren(function);
        }
    }

    ValuePtr body;
};

class SyntaxLiteral : public SyntacticValue
{
public:
};

class SyntaxLiteralFloat : public SyntaxLiteral
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLiteralFloat(" << value << ")";
    }

    double value;
};

class SyntaxLiteralInteger : public SyntaxLiteral
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLiteralInteger(" << value << ")";
    }

    LargeInteger value;    
};

class SyntaxLiteralCharacter : public SyntaxLiteral
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLiteralCharacter(" << value << ")";
    }

    char32_t value;
};

class SyntaxLiteralString : public SyntaxLiteral
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLiteralString(" << value << ")";
    }

    std::string value;    
};

class SyntaxLiteralSymbol : public SyntaxLiteral
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxLiteralSymbol(" << value << ")";
    }

    std::string value;    
};

class SyntaxBinaryExpressionSequence : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxBinaryExpressionSequence(";
        bool isFirst = true;
        for(auto &element : elements)
        {
            if(isFirst)
                isFirst = false;
            else
                out << ", ";
            element->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        for(const auto &element : elements)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    std::vector<ValuePtr> elements;
};

class SyntaxMessageCascade : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxMessageCascade(";
        if(receiver)
            out << receiver->printString() << ",";
        for(auto &message : messages)
        {
            out << ", ";
            message->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(receiver)
        {
            function(receiver);
            receiver->traverseChildren(function);
        }
        for(const auto &element : messages)
        {
            function(element);
            element->traverseChildren(function);
        }
    }

    ValuePtr receiver;
    std::vector<ValuePtr> messages;
};

class SyntaxMessageCascadeMessage : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxMessageCascadeMessage(";
        selector->printStringOn(out);
        for(auto &argument : arguments)
        {
            out << ", ";
            argument->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(selector)
        {
            function(selector);
            selector->traverseChildren(function);
        }
        for(const auto &argument : arguments)
        {
            function(argument);
            argument->traverseChildren(function);
        }
    }

    ValuePtr selector;
    std::vector<ValuePtr> arguments;
};

class SyntaxApplication : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxApplication(";
        functional->printStringOn(out);
        for(auto &argument : arguments)
        {
            out << ", ";
            argument->printStringOn(out);
        }
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(functional)
        {
            function(functional);
            functional->traverseChildren(function);
        }
        for(const auto &argument : arguments)
        {
            function(argument);
            argument->traverseChildren(function);
        }
    }

    ValuePtr functional;
    std::vector<ValuePtr> arguments;
    TokenKind kind;
};

typedef std::shared_ptr<SyntaxMessageCascade> SyntaxMessageCascadePtr;

class SyntaxMessageSend : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxMessageSend(";
        if(receiver)
        {
            receiver->printStringOn(out);
            out << ",";
        }
            
        selector->printStringOn(out);
        for(auto &argument : arguments)
        {
            out << ", ";
            argument->printStringOn(out);
        }
        out << ")";
    }

    virtual SyntaxMessageCascadePtr asMessageCascade() const override
    {
        auto firstMessage = std::make_shared<SyntaxMessageCascadeMessage> ();
        firstMessage->sourcePosition = sourcePosition;
        firstMessage->selector = selector;
        firstMessage->arguments = arguments;

        auto messageCascade = std::make_shared<SyntaxMessageCascade> ();
        messageCascade->sourcePosition = sourcePosition;
        messageCascade->receiver = receiver;
        messageCascade->messages.push_back(firstMessage);
        return messageCascade;
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(receiver)
        {
            function(receiver);
            receiver->traverseChildren(function);
        }
        if(selector)
        {
            function(selector);
            selector->traverseChildren(function);
        }
        for(const auto &argument : arguments)
        {
            function(argument);
            argument->traverseChildren(function);
        }
    }

    ValuePtr receiver;
    ValuePtr selector;
    std::vector<ValuePtr> arguments;
};



class SyntaxQuote : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxQuote(";
        value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr value;
};

class SyntaxQuasiQuote : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxQuasiQuote(";
        value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr value;
};

class SyntaxQuasiUnquote : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxQuasiUnquote(";
        value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr value;
};

class SyntaxSplice : public SyntacticValue
{
public:
    virtual void printStringOn(std::ostream &out) const override
    {
        out << "SyntaxSplice(";
        value->printStringOn(out);
        out << ")";
    }

    virtual void traverseChildren(const std::function<void (ValuePtr)> &function) const override
    {
        if(value)
        {
            function(value);
            value->traverseChildren(function);
        }
    }

    ValuePtr value;
};

} // end of namespace Sysmel

#endif //SYSMEL_SYNTAX_HPP
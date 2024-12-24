#include "parser.hpp"
#include "syntax.hpp"
#include <assert.h>

namespace Sysmel
{
struct ParserState
{
    SourceCodePtr sourceCode;
    const std::vector<TokenPtr> *tokens;
    size_t position = 0;

    bool atEnd() const
    {
        return position >= tokens->size();
    }

    TokenKind peekKind(int offset = 0) const
    {
        auto peekPosition = position + offset;
        if (peekPosition < tokens->size())
            return tokens->at(peekPosition)->kind;
        else
            return TokenKind::EndOfSource;
    }

    TokenPtr peek(int offset = 0) const
    {
        auto peekPosition = position + offset;
        if (peekPosition < tokens->size())
            return tokens->at(peekPosition);
        else
            return nullptr;
    }

    void advance()
    {
        assert(position < tokens->size());
        ++position;
    }

    TokenPtr next()
    {
        assert(position < tokens->size());
        auto token = tokens->at(position);
        ++position;
        return token;
    }

    /*def advanceWithExpectedError(self, message: str):
        if self.peekKind() == TokenKind.ERROR:
            errorToken = self.next()
            return self, ParseTreeErrorNode(errorToken.sourcePosition, errorToken.errorMessage)
        elif self.atEnd():
            return self, ParseTreeErrorNode(self.currentSourcePosition(), message)
        else:
            errorPosition = self.currentSourcePosition()
            self.advance()
            return self, ParseTreeErrorNode(errorPosition, message)*/
    ValuePtr advanceWithExpectedError(const char *message)
    {
        if (peekKind() == TokenKind::Error)
        {
            auto errorToken = next();
            auto errorNode = std::make_shared<SyntaxError> ();
            errorNode->sourcePosition = errorToken->position;
            errorNode->errorMessage = errorToken->errorMessage;
            return errorNode;
        }
        else if (atEnd())
        {
            auto errorNode = std::make_shared<SyntaxError> ();
            errorNode->sourcePosition = currentSourcePosition();
            errorNode->errorMessage = message;
            return errorNode;
        }
        else
        {
            auto errorPosition = currentSourcePosition();
            advance();
            auto errorNode = std::make_shared<SyntaxError> ();
            errorNode->sourcePosition = errorPosition;
            errorNode->errorMessage = message;
            return errorNode;
        }
    }

    size_t memento() const 
    {
        return position;
    }

    void restore(size_t memento)
    {
        position = memento;
    }

    SourcePositionPtr sourcePositionFrom(size_t startingPosition)
    {
        assert(startingPosition < tokens->size());
        auto startSourcePosition = tokens->at(startingPosition)->position;
        if (position > 0)
        {
            auto endSourcePosition = previousSourcePosition();
            return startSourcePosition->to(endSourcePosition);
        }
        else
        {
            auto endSourcePosition = currentSourcePosition();
            return startSourcePosition->until(endSourcePosition);
        }
    }

    SourcePositionPtr previousSourcePosition()
    {
        assert(position > 0);
        return tokens->at(position - 1)->position;
    }

    SourcePositionPtr currentSourcePosition()
    {
        if(position < tokens->size())
            return tokens->at(position)->position;
        assert(!tokens->empty());
        //assert(!tokens->at(tokens->size() - 1)->kind == TokenKind::EndOfSource);
        return tokens->at(tokens->size() - 1)->position;
    }

    ValuePtr makeErrorAtCurrentSourcePosition(const char *errorMessage)
    {
        auto node = std::make_shared<SyntaxError> ();
        node->sourcePosition = currentSourcePosition();
        node->errorMessage = errorMessage;
        return node;
    }
};

int64_t parseIntegerConstant(const std::string &constant)
{
    int64_t result = 0;
    int64_t radix = 10;
    bool hasSeenRadix = false;
    for(size_t i = 0; i < constant.size(); ++i)
    {
        auto c = constant[i];
        if(!hasSeenRadix && (c == 'r' || c == 'R'))
        {
            hasSeenRadix = true;
            radix = result;
            result = 0;
        }
        else
        {
            if ('0' <= c && c <= '9')
                result = result*radix + c - '0';
            else if ('A' <= c && c <= 'Z')
                result = result*radix + c - 'A' + 10;
            else if ('a' <= c && c <= 'z')
                result = result*radix + c - 'a' + 10;
        }
    }
    return result;
}

ValuePtr parseLiteralInteger(ParserState &state)
{
    auto token = state.next();
    assert(token->kind == TokenKind::Nat);
    auto literal = std::make_shared<SyntaxLiteralInteger> ();
    literal->sourcePosition = token->position;
    literal->value = parseIntegerConstant(token->getValue());
    return literal;
}

ValuePtr parseLiteralFloat(ParserState &state)
{
    auto token = state.next();
    assert(token->kind == TokenKind::Float);
    auto literal = std::make_shared<SyntaxLiteralFloat> ();
    literal->sourcePosition = token->position;
    literal->value = atof(token->getValue().c_str());
    return literal;
}

std::string parseCEscapedString(const std::string &str)
{
    std::string unescaped;
    unescaped.reserve(str.size());

    for(size_t i = 0; i < str.size(); ++i)
    {
        auto c = str[i];
        if (c == '\\')
        {
            auto c1 = str[++i];
            switch(c1)
            {
            case 'n':
                unescaped.push_back('\n');
                break;
            case 'r':
                unescaped.push_back('\r');
                break;
            case 't':
                unescaped.push_back('\t');
                break;
            default:
                unescaped.push_back(c1);
                break;
            }
        }
        else
        {
            unescaped.push_back(c);
        }
    }

    return unescaped;
}

ValuePtr parseLiteralCharacter(ParserState &state)
{
    auto token = state.next();
    assert(token->kind == TokenKind::Character);
    auto literal = std::make_shared<SyntaxLiteralCharacter> ();
    literal->sourcePosition = token->position;
    auto tokenValue = token->getValue();
    literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size()-2))[0];
    return literal;
}

ValuePtr parseLiteralString(ParserState &state)
{
    auto token = state.next();
    assert(token->kind == TokenKind::String);
    auto literal = std::make_shared<SyntaxLiteralString> ();
    literal->sourcePosition = token->position;
    auto tokenValue = token->getValue();
    literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size()-2));
    return literal;
}

ValuePtr parseLiteralSymbol(ParserState &state)
{
    auto token = state.next();
    assert(token->kind == TokenKind::Symbol);
    auto literal = std::make_shared<SyntaxLiteralSymbol> ();
    literal->sourcePosition = token->position;
    auto tokenValue = token->getValue().substr(1);
    if (tokenValue[0] == '\"')
        literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size()-2));
    else
        literal->value = tokenValue;
    
    return literal;
}


ValuePtr parseLiteral(ParserState &state)
{
    switch(state.peekKind())
    {
    case TokenKind::Nat: return parseLiteralInteger(state);
    case TokenKind::Float: return parseLiteralFloat(state);
    case TokenKind::Character: return parseLiteralCharacter(state);
    case TokenKind::String: return parseLiteralString(state);
    case TokenKind::Symbol: return parseLiteralSymbol(state);
    default:
        return state.advanceWithExpectedError("Expected a literal");
    }
}

/*
def parseLiteral(state: ParserState) -> tuple[ParserState, ParseTreeNode]:
    if state.peekKind() == TokenKind.NAT: return parseLiteralInteger(state)
    elif state.peekKind() == TokenKind.FLOAT: return parseLiteralFloat(state)
    elif state.peekKind() == TokenKind.STRING: return parseLiteralString(state)
    elif state.peekKind() == TokenKind.CHARACTER: return parseLiteralCharacter(state)
    elif state.peekKind() == TokenKind.SYMBOL: return parseLiteralSymbol(state)
    else: return state.advanceWithExpectedError('Expected a literal.')
*/
ValuePtr parseExpression(ParserState &state)
{
    return parseLiteral(state);
}

std::vector<ValuePtr> parseExpressionListUntilEndOrDelimiter(ParserState &state, TokenKind delimiter)
{
    std::vector<ValuePtr> elements;

    // Leading dots.
    while(state.peekKind() == TokenKind::Dot)
        state.advance();

    bool expectsExpression = true;

    while(!state.atEnd() && state.peekKind() != delimiter)
    {
        if (!expectsExpression)
            elements.push_back(state.makeErrorAtCurrentSourcePosition("Expected dot before expression."));
        auto expression = parseExpression(state);
        elements.push_back(expression);

        // Trailing dots.
        while(state.peekKind() == TokenKind::Dot)
        {
            expectsExpression = true;
            state.advance();
        }
    }


    return elements;
}

ValuePtr parseSequenceUntilEndOrDelimiter(ParserState &state, TokenKind delimiter)
{
    auto initialPosition = state.position;
    auto expressions = parseExpressionListUntilEndOrDelimiter(state, delimiter);
    if (expressions.size() == 1)
        return expressions[0];

    auto syntaxSequence = std::make_shared<SyntaxValueSequence> ();
    syntaxSequence->sourcePosition = state.sourcePositionFrom(initialPosition);
    syntaxSequence->elements = expressions;
    return syntaxSequence;
}

ValuePtr parseTopLevelExpression(ParserState &state)
{
    return parseSequenceUntilEndOrDelimiter(state, TokenKind::EndOfSource);
}
ValuePtr parseTokens(const SourceCodePtr &sourceCode, const std::vector<TokenPtr> &tokens)
{
    auto state = ParserState{sourceCode, &tokens};
    return parseTopLevelExpression(state);
}

} // End of namespace Sysmel
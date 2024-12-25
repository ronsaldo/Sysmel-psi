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

        ValuePtr advanceWithExpectedError(const char *message)
        {
            if (peekKind() == TokenKind::Error)
            {
                auto errorToken = next();
                auto errorNode = std::make_shared<SyntaxError>();
                errorNode->sourcePosition = errorToken->position;
                errorNode->errorMessage = errorToken->errorMessage;
                return errorNode;
            }
            else if (atEnd())
            {
                auto errorNode = std::make_shared<SyntaxError>();
                errorNode->sourcePosition = currentSourcePosition();
                errorNode->errorMessage = message;
                return errorNode;
            }
            else
            {
                auto errorPosition = currentSourcePosition();
                advance();
                auto errorNode = std::make_shared<SyntaxError>();
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
            if (position < tokens->size())
                return tokens->at(position)->position;
            assert(!tokens->empty());
            // assert(!tokens->at(tokens->size() - 1)->kind == TokenKind::EndOfSource);
            return tokens->at(tokens->size() - 1)->position;
        }

        ValuePtr makeErrorAtCurrentSourcePosition(const char *errorMessage)
        {
            auto node = std::make_shared<SyntaxError>();
            node->sourcePosition = currentSourcePosition();
            node->errorMessage = errorMessage;
            return node;
        }

        ValuePtr expectAddingErrorToNode(TokenKind expectedKind, ValuePtr node)
        {
            if (peekKind() == expectedKind)
            {
                advance();
                return node;
            }

            auto errorPosition = currentSourcePosition();
            auto syntaxErrorNode = std::make_shared<SyntaxError>();
            syntaxErrorNode->sourcePosition = errorPosition;
            syntaxErrorNode->innerNode = node;

            std::ostringstream out;
            out << "Expected token of kind " << getTokenKindName(expectedKind) << ".";
            syntaxErrorNode->errorMessage = out.str();
            return syntaxErrorNode;
        }
    };

    ValuePtr parseSequenceUntilEndOrDelimiter(ParserState &state, TokenKind delimiter);
    ValuePtr parseTerm(ParserState &state);

    int64_t parseIntegerConstant(const std::string &constant)
    {
        int64_t result = 0;
        int64_t radix = 10;
        bool hasSeenRadix = false;
        for (size_t i = 0; i < constant.size(); ++i)
        {
            auto c = constant[i];
            if (!hasSeenRadix && (c == 'r' || c == 'R'))
            {
                hasSeenRadix = true;
                radix = result;
                result = 0;
            }
            else
            {
                if ('0' <= c && c <= '9')
                    result = result * radix + c - '0';
                else if ('A' <= c && c <= 'Z')
                    result = result * radix + c - 'A' + 10;
                else if ('a' <= c && c <= 'z')
                    result = result * radix + c - 'a' + 10;
            }
        }
        return result;
    }

    ValuePtr parseLiteralInteger(ParserState &state)
    {
        auto token = state.next();
        assert(token->kind == TokenKind::Nat);
        auto literal = std::make_shared<SyntaxLiteralInteger>();
        literal->sourcePosition = token->position;
        literal->value = parseIntegerConstant(token->getValue());
        return literal;
    }

    ValuePtr parseLiteralFloat(ParserState &state)
    {
        auto token = state.next();
        assert(token->kind == TokenKind::Float);
        auto literal = std::make_shared<SyntaxLiteralFloat>();
        literal->sourcePosition = token->position;
        literal->value = atof(token->getValue().c_str());
        return literal;
    }

    std::string parseCEscapedString(const std::string &str)
    {
        std::string unescaped;
        unescaped.reserve(str.size());

        for (size_t i = 0; i < str.size(); ++i)
        {
            auto c = str[i];
            if (c == '\\')
            {
                auto c1 = str[++i];
                switch (c1)
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
        auto literal = std::make_shared<SyntaxLiteralCharacter>();
        literal->sourcePosition = token->position;
        auto tokenValue = token->getValue();
        literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size() - 2))[0];
        return literal;
    }

    ValuePtr parseLiteralString(ParserState &state)
    {
        auto token = state.next();
        assert(token->kind == TokenKind::String);
        auto literal = std::make_shared<SyntaxLiteralString>();
        literal->sourcePosition = token->position;
        auto tokenValue = token->getValue();
        literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size() - 2));
        return literal;
    }

    ValuePtr parseLiteralSymbol(ParserState &state)
    {
        auto token = state.next();
        assert(token->kind == TokenKind::Symbol);
        auto literal = std::make_shared<SyntaxLiteralSymbol>();
        literal->sourcePosition = token->position;
        auto tokenValue = token->getValue().substr(1);
        if (tokenValue[0] == '\"')
            literal->value = parseCEscapedString(tokenValue.substr(1, tokenValue.size() - 2));
        else
            literal->value = tokenValue;

        return literal;
    }

    ValuePtr parseLiteral(ParserState &state)
    {
        switch (state.peekKind())
        {
        case TokenKind::Nat:
            return parseLiteralInteger(state);
        case TokenKind::Float:
            return parseLiteralFloat(state);
        case TokenKind::Character:
            return parseLiteralCharacter(state);
        case TokenKind::String:
            return parseLiteralString(state);
        case TokenKind::Symbol:
            return parseLiteralSymbol(state);
        default:
            return state.advanceWithExpectedError("Expected a literal");
        }
    }

    ValuePtr parseIdentifier(ParserState &state)
    {
        auto token = state.next();
        assert(token->kind == TokenKind::Identifier);

        auto node = std::make_shared<SyntaxIdentifierReference>();
        node->sourcePosition = token->position;
        node->value = token->getValue();
        return node;
    }

    bool isBinaryExpressionOperator(TokenKind kind)
    {
        switch (kind)
        {
        case TokenKind::Operator:
        case TokenKind::Star:
        case TokenKind::LessThan:
        case TokenKind::GreaterThan:
        case TokenKind::Bar:
            return true;
        default:
            return false;
        }
    }

    ValuePtr parseParenthesis(ParserState &state)
    {
        auto startPosition = state.position;
        assert(state.peekKind() == TokenKind::LeftParent);
        state.advance();

        if (isBinaryExpressionOperator(state.peekKind()) && state.peekKind(1) == TokenKind::RightParent)
        {
            auto token = state.next();
            state.advance();
        }

        if (state.peekKind() == TokenKind::RightParent)
        {
            state.advance();
            auto tuple = std::make_shared<SyntaxTuple>();
            tuple->sourcePosition = state.sourcePositionFrom(startPosition);
            return tuple;
        }

        auto expression = parseSequenceUntilEndOrDelimiter(state, TokenKind::RightParent);
        expression = state.expectAddingErrorToNode(TokenKind::RightParent, expression);
        return expression;
    }

    ValuePtr parseBinaryExpressionSequence(ParserState &state)
    {
        auto startPosition = state.position;
        return parseTerm(state);
    }

    ValuePtr parseAssociationExpression(ParserState &state)
    {
        auto startPosition = state.position;
        auto key = parseBinaryExpressionSequence(state);

        if(state.peekKind() != TokenKind::Colon)
            return key;

        state.advance();
        auto value = parseAssociationExpression(state);
        auto assoc = std::make_shared<SyntaxAssociation> ();
        assoc->sourcePosition = state.sourcePositionFrom(startPosition);
        assoc->key = key;
        assoc->value = value;
        return assoc;
    }

    ValuePtr parseKeywordApplication(ParserState &state)
    {
        assert(state.peekKind() == TokenKind::Keyword);
        auto startPosition = state.position;

        std::string symbolValue;
        std::vector<ValuePtr> arguments;

        while(state.peekKind() == TokenKind::Keyword)
        {
            auto keywordToken = state.next();
            symbolValue.append(keywordToken->getValue());

            auto argument = parseAssociationExpression(state);
            arguments.push_back(argument);
        }

        auto identifier = std::make_shared<SyntaxLiteralSymbol> ();
        identifier->sourcePosition = state.sourcePositionFrom(startPosition);
        identifier->value = symbolValue;

        auto messageSend = std::make_shared<SyntaxMessageSend> ();
        messageSend->sourcePosition = state.sourcePositionFrom(startPosition);
        messageSend->selector = identifier;
        messageSend->arguments = arguments;
        return messageSend;
    }

    ValuePtr parseMessageCascade(ParserState &state)
    {
        // TODO: Implement this part
        return parseAssociationExpression(state);
    }
    
    ValuePtr parseLowPrecedenceExpression(ParserState &state)
    {
        if(state.peekKind() == TokenKind::Keyword)
            return parseKeywordApplication(state);
        return parseMessageCascade(state);
    }

    ValuePtr parseAssignmentExpression(ParserState &state)
    {
        auto startPosition = state.position;
        auto assignedStore = parseLowPrecedenceExpression(state);
        if (state.peekKind() == TokenKind::Assignment)
        {
            state.advance();
            auto assignedValue = parseAssignmentExpression(state);
            auto assignment = std::make_shared<SyntaxAssignment> ();
            assignment->sourcePosition = state.sourcePositionFrom(startPosition);
            assignment->store = assignedStore;
            assignment->value = assignedValue;
            return assignment;
        }
        else
        {
            return assignedStore;
        }
    }

    ValuePtr parseCommaExpression(ParserState &state)
    {
        auto startingPosition = state.position;
        auto element = parseAssignmentExpression(state);

        if (state.peekKind() != TokenKind::Comma)
            return element;

        std::vector<ValuePtr> elements;
        elements.push_back(element);

        while(state.peekKind() == TokenKind::Comma)
        {
            state.advance();
            element = parseAssignmentExpression(state);
            elements.push_back(element);
        }

        auto tuple = std::make_shared<SyntaxTuple> ();
        tuple->sourcePosition = state.sourcePositionFrom(startingPosition);
        tuple->elements = elements;
        return tuple;
    }

    ValuePtr parseFunctionalType(ParserState &state)
    {
        auto startPosition = state.position;
        auto argumentPatternOrExpression = parseCommaExpression(state);

        if(state.peekKind() == TokenKind::ColonColon)
        {
            state.advance();
            auto resultTypeExpression = parseFunctionalType(state);
            auto functionalType = std::make_shared<SyntaxFunctionalDependentType> ();
            functionalType->sourcePosition = state.sourcePositionFrom(startPosition);
            functionalType->argumentPattern = argumentPatternOrExpression;
            functionalType->resultType = resultTypeExpression;
        }

        return argumentPatternOrExpression;
    }

    ValuePtr parseBlock(ParserState &state)
    {
        // {
        auto startPosition = state.position;
        assert(state.peekKind() == TokenKind::LeftCurlyBracket);
        state.advance();

        ValuePtr functionalType;
        if (state.peekKind() == TokenKind::Bar)
        {
            state.advance();
            if (state.peekKind() == TokenKind::Bar)
            {
                auto functionalTypeNode = std::make_shared<SyntaxFunctionalDependentType>();
                functionalTypeNode->sourcePosition = state.currentSourcePosition();
                functionalType = functionalTypeNode;
            }
            else
            {
                functionalType = parseFunctionalType(state);
                state.expectAddingErrorToNode(TokenKind::Bar, functionalType);
            }
        }
        // Body
        auto body = parseSequenceUntilEndOrDelimiter(state, TokenKind::RightCurlyBracket);
        body = state.expectAddingErrorToNode(TokenKind::RightCurlyBracket, body);

        if (functionalType)
        {
            auto block = std::make_shared<SyntaxBlock>();
            block->sourcePosition = state.sourcePositionFrom(startPosition);
            block->functionType = functionalType;
            block->body = body;
            return block;
        }
        else
        {
            auto lexicalBlock = std::make_shared<SyntaxLexicalBlock>();
            lexicalBlock->sourcePosition = state.sourcePositionFrom(startPosition);
            lexicalBlock->body = body;
            return lexicalBlock;
        }
    }

    ValuePtr parseTerm(ParserState &state)
    {
        switch (state.peekKind())
        {
        case TokenKind::Identifier:
            return parseIdentifier(state);
        case TokenKind::LeftParent:
            return parseParenthesis(state);
        case TokenKind::LeftCurlyBracket:
            return parseBlock(state);
        default:
            return parseLiteral(state);
        }
    }

    /*def parseTerm(state: ParserState) -> tuple[ParserState, ParseTreeNode]:
        elif state.peekKind() == TokenKind.LEFT_CURLY_BRACKET: return parseBlock(state)
        elif state.peekKind() == TokenKind.DICTIONARY_START: return parseDictionary(state)
        elif state.peekKind() == TokenKind.COLON: return parseBindableName(state)
    */

    ValuePtr parseFunctionalTypeWithOptionalArgument(ParserState &state)
    {
        auto startPosition = state.position;
        if (state.peekKind() == TokenKind::ColonColon)
        {
            state.advance();
            auto resultTypeExpression = parseFunctionalType(state);
            auto functionalNode = std::make_shared<SyntaxFunctionalDependentType>();
            functionalNode->sourcePosition = state.sourcePositionFrom(startPosition);
            functionalNode->resultType = resultTypeExpression;
            return functionalNode;
        }
        else 
        {
            return parseFunctionalType(state);
        }
    }

    ValuePtr parseBindExpression(ParserState &state)
    {
        auto startPosition = state.position;
        auto patternExpressionOrValue = parseFunctionalTypeWithOptionalArgument(state);
        if (state.peekKind() == TokenKind::BindOperator)
        {
            state.advance();
            auto boundValue = parseBindExpression(state);
            auto bindPattern = std::make_shared<SyntaxBindPattern>();
            bindPattern->sourcePosition = state.sourcePositionFrom(startPosition);
            bindPattern->pattern = patternExpressionOrValue;
            bindPattern->value = boundValue;
            return bindPattern;
        }
        else
        {
            return patternExpressionOrValue;
        }
    }

    ValuePtr parseExpression(ParserState &state)
    {
        return parseBindExpression(state);
    }

    std::vector<ValuePtr> parseExpressionListUntilEndOrDelimiter(ParserState &state, TokenKind delimiter)
    {
        std::vector<ValuePtr> elements;

        // Leading dots.
        while (state.peekKind() == TokenKind::Dot)
            state.advance();

        bool expectsExpression = true;

        while (!state.atEnd() && state.peekKind() != delimiter)
        {
            if (!expectsExpression)
                elements.push_back(state.makeErrorAtCurrentSourcePosition("Expected dot before expression."));
            auto expression = parseExpression(state);
            elements.push_back(expression);

            // Trailing dots.
            while (state.peekKind() == TokenKind::Dot)
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

        auto syntaxSequence = std::make_shared<SyntaxValueSequence>();
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
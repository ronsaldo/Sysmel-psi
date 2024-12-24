#include "scanner.hpp"
#include <assert.h>

namespace Sysmel
{

const char *TokenKindNames[] = {
#define TokenKindName(name) #name ,
#include "token_kind.inc"
#undef TokenKindName
};

const char *getTokenKindName(TokenKind kind)
{
    return TokenKindNames[int(kind)];
}

inline bool isDigit(int character)
{
    return '0' <= character && character <= '9';
}

inline bool isIdentifierStart(int character)
{
    return
        ('A' <= character && character <= 'Z') ||
        ('a' <= character && character <= 'z') ||
        (character == '_')
        ;
}

inline bool isIdentifierMiddle(int character)
{
    return isIdentifierStart(character) || isDigit(character);
}

bool isOperatorCharacter(int character)
{
    for (auto c : "+-/\\*~<>=@%|&?!^")
        if(c == character)
            return true;
    return false;
}

struct ScannerState
{
    SourceCodePtr sourceCode;
    int32_t position = 0;
    int32_t line = 1;
    int32_t column = 1;
    bool isPreviousCR = false;

    bool atEnd() const
    {
        return size_t(position) >= sourceCode->text.size();
    }

    int peek(int peekOffset = 0)
    {
        size_t peekPosition = position + peekOffset;
        if(peekPosition < sourceCode->text.size())
            return sourceCode->text[peekPosition];
        else
            return -1;
    }

    void advanceSinglePosition()
    {
        assert(!atEnd());
        auto c = sourceCode->text[position];
        ++position;
        switch(c)
        {
        case '\r':
            ++line;
            column = 1;
            isPreviousCR = true;
            break;
        case '\n':
            if (!isPreviousCR)
            {
                ++line;
                column = 1;
            }
            isPreviousCR = false;
            break;
        case '\t':
            column = (column + 4) % 4 * 4 + 1;
            isPreviousCR = false;
            break;
        default:
            ++column;
            isPreviousCR = false;
            break;
        }
    }

    void advance(int count = 1)
    {
        for(int i = 0; i < count; ++i)
        {
            advanceSinglePosition();
        }
    }

    TokenPtr makeToken(TokenKind kind)
    {
        auto sourcePosition = std::make_shared<SourcePosition> ();
        sourcePosition->sourceCode  = sourceCode;
        sourcePosition->startIndex  = position;
        sourcePosition->startLine   = line;
        sourcePosition->startColumn = column;
        sourcePosition->endIndex    = position;
        sourcePosition->endLine     = line;
        sourcePosition->endColumn   = column;
        
        auto token = std::make_shared<Token> ();
        token->kind = kind;
        token->position = sourcePosition;
        return token;
    }

    TokenPtr makeTokenStartingFrom(TokenKind kind, const ScannerState &initialState)
    {
        auto sourcePosition = std::make_shared<SourcePosition> ();
        sourcePosition->sourceCode  = sourceCode;
        sourcePosition->startIndex  = initialState.position;
        sourcePosition->startLine   = initialState.line;
        sourcePosition->startColumn = initialState.column;
        sourcePosition->endIndex    = position;
        sourcePosition->endLine     = line;
        sourcePosition->endColumn   = column;

        auto token = std::make_shared<Token> ();
        token->kind = kind;
        token->position = sourcePosition;
        return token;
    }

    TokenPtr makeErrorTokenStartingFrom(const std::string &errorMessage, const ScannerState &initialState)
    {
        auto sourcePosition = std::make_shared<SourcePosition> ();
        sourcePosition->sourceCode  = sourceCode;
        sourcePosition->startIndex  = initialState.position;
        sourcePosition->startLine   = initialState.line;
        sourcePosition->startColumn = initialState.column;
        sourcePosition->endIndex    = position;
        sourcePosition->endLine     = line;
        sourcePosition->endColumn   = column;

        auto token = std::make_shared<Token> ();
        token->kind = TokenKind::Error;
        token->position = sourcePosition;
        token->errorMessage = errorMessage;
        return token;
    }
};


TokenPtr skipWhite(ScannerState &state)
{
    bool hasSeenComment = false;
    
    do
    {
        hasSeenComment = false;
        while (!state.atEnd() && state.peek() <= ' ')
            state.advance();

        if(state.peek() == '#')
        {
            // Single line comment.
            if(state.peek(1) == '#')
            {
                state.advance(2);

                while (!state.atEnd())
                {
                    if (state.peek() == '\r' || state.peek() == '\n')
                        break;
                    state.advance();
                }
                hasSeenComment = true;
            }
            else if(state.peek(1) == '*')
            {
                auto commentInitialState = state;
                state.advance(2);
                bool hasCommentEnd = false;
                while (!state.atEnd())
                {
                    hasCommentEnd = state.peek() == '*' &&  state.peek() == '#';
                    if (hasCommentEnd)
                    {
                        state.advance(2);
                        break;
                    }
                }
                if (!hasCommentEnd)
                {
                    return state.makeErrorTokenStartingFrom("Incomplete multiline comment.", commentInitialState);
                }
            }
        }
    } while (hasSeenComment);
    
    return nullptr;
}

bool scanAdvanceKeyword(ScannerState &state)
{
    if(!isIdentifierStart(state.peek()))
        return false;

    auto initialState = state;
    while (isIdentifierMiddle(state.peek()))
        state.advance();

    if(state.peek() != ':')
    {
        state = initialState;
        return false;
    }

    return true;
}
TokenPtr scanSingleToken(ScannerState &state)
{
    auto whiteToken = skipWhite(state);
    if(whiteToken)
        return whiteToken;

    if(state.atEnd())
        return state.makeToken(TokenKind::EndOfSource);

    auto initialState = state;
    auto c = state.peek();

    // Identifiers, keywords and multi-keywords
    if(isIdentifierStart(c))
    {
        state.advance();
        while (isIdentifierMiddle(state.peek()))
            state.advance();

        if(state.peek() == ':')
        {
            state.advance();
            bool isMultiKeyword = false;
            bool hasAdvanced = true;
            while(hasAdvanced)
            {
                hasAdvanced = scanAdvanceKeyword(state);
                isMultiKeyword = isMultiKeyword || hasAdvanced;
            }

            if(isMultiKeyword)
                return state.makeTokenStartingFrom(TokenKind::MultiKeyword, initialState);
            else
                return state.makeTokenStartingFrom(TokenKind::Keyword, initialState);
        }

        return state.makeTokenStartingFrom(TokenKind::Identifier, initialState);
    }

    // Numbers
    if(isDigit(c))
    {
        state.advance();
        while(isDigit(state.peek()))
            state.advance();

        // Parse the radix
        if(state.peek() == 'r')
        {
            state.advance();
            while(isIdentifierMiddle(state.peek()))
                state.advance();
            return state.makeTokenStartingFrom(TokenKind::Nat, initialState);
        }

        // Parse the decimal point
        if(state.peek() == '.' && isDigit(state.peek(1)))
        {
            state.advance(2);
            while(isDigit(state.peek()))
                state.advance();

            // Parse the exponent
            if(state.peek() == 'e' || state.peek() == 'E')
            {
                if(isDigit(state.peek(1)) ||
                ((state.peek(1) == '+' || state.peek(1) == '-') && isDigit(state.peek(2))))
                {
                    state.advance(2);
                    while(isDigit(state.peek()))
                        state.advance();
                }
            }

            return state.makeTokenStartingFrom(TokenKind::Float, initialState);
        }

        return state.makeTokenStartingFrom(TokenKind::Nat, initialState);
    }

    // Symbols
    if(c == '#')
    {
        auto c1 = state.peek(1);
        if(isIdentifierStart(c1))
        {
            state.advance(2);
            while (isIdentifierMiddle(state.peek()))
                state.advance();


            if (state.peek() == ':')
            {
                state.advance();
                bool hasAdvanced = true;
                while(hasAdvanced)
                {
                    hasAdvanced = scanAdvanceKeyword(state);
                }

                return state.makeTokenStartingFrom(TokenKind::Symbol, initialState);                
            }
            return state.makeTokenStartingFrom(TokenKind::Symbol, initialState);                
        }
        else if(isOperatorCharacter(c1))
        {
            state.advance(2);
            while(isOperatorCharacter(state.peek()))
                state.advance();
            return state.makeTokenStartingFrom(TokenKind::Symbol, initialState); 
        }
    }

    switch(c)
    {
    case '(':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::LeftParent, initialState); 
    case ')':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::RightParent, initialState); 
    case '[':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::LeftBracket, initialState); 
    case ']':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::RightBracket, initialState); 
    case '{':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::LeftCurlyBracket, initialState); 
    case '}':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::RightCurlyBracket, initialState); 
    case ';':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::Semicolon, initialState);
    case ',':
        state.advance();
        return state.makeTokenStartingFrom(TokenKind::Comma, initialState); 
    case '.':
        state.advance();
        if(state.peek() == '.' && state.peek(1) == '.')
        {
            state.advance(2);
            return state.makeTokenStartingFrom(TokenKind::Ellipsis, initialState);
        }
        return state.makeTokenStartingFrom(TokenKind::Dot, initialState);
    case ':':
        state.advance();
        if(state.peek() == ':')
        {
            state.advance();
            return state.makeTokenStartingFrom(TokenKind::ColonColon, initialState);
        }
        else if(state.peek() == '=')
        {
            state.advance();
            return state.makeTokenStartingFrom(TokenKind::Assignment, initialState);
        }
        return state.makeTokenStartingFrom(TokenKind::Colon, initialState);
    case '`':
        if (state.peek() == '\'')
        {
            state.advance(2);
            return state.makeTokenStartingFrom(TokenKind::Quote, initialState);
        }
        else if (state.peek() == '`')
        {
            state.advance(2);
            return state.makeTokenStartingFrom(TokenKind::QuasiQuote, initialState);
        }
        else if (state.peek() == ',')
        {
            state.advance(2);
            return state.makeTokenStartingFrom(TokenKind::QuasiUnquote, initialState);
        }
        else if (state.peek() == '@')
        {
            state.advance(2);
            return state.makeTokenStartingFrom(TokenKind::Splice, initialState);
        }
        break;
    case '|':
        state.advance();
        if (isOperatorCharacter(state.peek()))
        {
            while(isOperatorCharacter(state.peek()))
                state.advance();
            return state.makeTokenStartingFrom(TokenKind::Operator, initialState);
        }

        return state.makeTokenStartingFrom(TokenKind::Bar, initialState);
    default:
        break;
    }

    if(isOperatorCharacter(c))
    {
        state.advance();
        while(isOperatorCharacter(state.peek()))
            state.advance();

        auto token = state.makeTokenStartingFrom(TokenKind::Operator, initialState);
        auto tokenValue = token->getValue();
        if (tokenValue == "<")
            token->kind = TokenKind::LessThan;
        else if (tokenValue == ">")
            token->kind = TokenKind::GreaterThan;
        else if (tokenValue == "*")
            token->kind = TokenKind::Star;
        else if (tokenValue == "?")
            token->kind = TokenKind::Question;
        else if (tokenValue == "!")
            token->kind = TokenKind::Bang;
        else if (tokenValue == "<-")
            token->kind = TokenKind::BindOperator;
        return token;
    }

    std::string unknownCharacter;
    unknownCharacter.push_back(state.peek());
    state.advance();
    return state.makeErrorTokenStartingFrom("Unknown character: " + unknownCharacter, initialState);
}

std::vector<TokenPtr> scanSourceCode(const SourceCodePtr &sourceCode)
{
    std::vector<TokenPtr> tokens;
    ScannerState currentState;
    TokenPtr scannedToken;
    currentState.sourceCode = sourceCode;

    do
    {
        scannedToken = scanSingleToken(currentState);
        if(scannedToken)
        {
            tokens.push_back(scannedToken);
            if (scannedToken->kind == TokenKind::EndOfSource)
                break;
        }
        
    }
    while(scannedToken);

    return tokens;
}

} // End of namespace Sysmel
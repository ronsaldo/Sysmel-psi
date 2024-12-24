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

struct ScannerState
{
    SourceCodePtr sourceCode;
    int32_t position = 0;
    int32_t line = 1;
    int32_t column = 1;
    bool isPreviousCR = false;

    bool atEnd() const
    {
        return position >= sourceCode->text.size();
    }

    int peek(int peekOffset = 0)
    {
        auto peekPosition = position + peekOffset;
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

TokenPtr scanSingleToken(ScannerState &state)
{
    auto whiteToken = skipWhite(state);
    if(whiteToken)
        return whiteToken;

    if(state.atEnd())
        return state.makeToken(TokenKind::EndOfSource);

    std::string unknownCharacter;
    unknownCharacter.push_back(state.peek());
    auto initialState = state;
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
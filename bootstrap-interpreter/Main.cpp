#include "Scanner.hpp"
#include "Parser.hpp"
#include "Syntax.hpp"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

using namespace Sysmel;

void printHelp()
{
    printf(
"bootstrap-interpreter\n"
"-ep        Evaluate and Print Result.\n");
}

void printVersion()
{
    printf("bootstrap-interpreter version 0.1");
}

void dumpTokens(const std::vector<TokenPtr> &tokens)
{
    for(auto token : tokens)
    {
        if(token->errorMessage.empty())
            printf("%s\n", getTokenKindName(token->kind));
        else
            printf("%s: %s\n", getTokenKindName(token->kind), token->errorMessage.c_str());
    }
}

void dumpParseTree(const ValuePtr &parseTree)
{
    auto parseTreeString = parseTree->printString();
    printf("%s\n", parseTreeString.c_str());
}

bool checkSyntaxErrors(ValuePtr parseTree)
{
    auto parseErrors = parseTree->collectSyntaxErrors();
    for(auto &parseError : parseErrors)
    {
        auto position = parseError->sourcePosition;
        fprintf(stderr, "%s:%ld.%ld-%ld.%ld: %s\n",
            position->sourceCode->name.c_str(),
            position->startLine, position->startColumn,
            position->endLine, position->endColumn,
            parseError->errorMessage.c_str());
    }
    return !parseErrors.empty();
}

ValuePtr evaluateSourceCode(const SourceCodePtr &sourceCode)
{
    auto tokens = scanSourceCode(sourceCode);
    dumpTokens(tokens);
    
    auto parseTree = parseTokens(sourceCode, tokens);
    //dumpParseTree(parseTree);
    if(checkSyntaxErrors(parseTree))
        return nullptr;
    return parseTree;
}

bool evaluateAndPrintString(const std::string &sourceText)
{
    auto sourceCode = std::make_shared<SourceCode> ();
    sourceCode->directory = ".";
    sourceCode->name = "cli";
    sourceCode->language = "sysmel";
    sourceCode->text = sourceText;

    auto evaluationResult = evaluateSourceCode(sourceCode);
    if(!evaluationResult)
        return false;

    printf("%s\n", evaluationResult->printString().c_str());
    return true;
}

int main(int argc, const char **argv)
{
    std::vector<std::string> inputFileNames;

    for(int i = 1; i < argc; ++i)
    {
        auto argument = argv[i];
        if(argument[0] == '-' || argument[1] == '-')
        {
            if(!strcmp(argument, "-h"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(argument, "-v"))
            {
                printVersion();
                return 0;
            }
            else if(!strcmp(argument, "-ep") && i + 1 < argc)
            {
                if(!evaluateAndPrintString(argv[++i]))
                    return 1;
            }
        }
        else
        {
            inputFileNames.push_back(argument);
        }
    }

    for(auto &inputFileName : inputFileNames)
    {
        printf("TODO: evaluate input file %s\n", inputFileName.c_str());
    }

    return 0;
}
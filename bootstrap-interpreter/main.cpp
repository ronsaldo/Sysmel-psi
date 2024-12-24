#include "scanner.hpp"
#include "parser.hpp"
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

void evaluateAndPrintString(const std::string &sourceText)
{
    auto sourceCode = std::make_shared<SourceCode> ();
    sourceCode->directory = ".";
    sourceCode->name = "cli";
    sourceCode->language = "sysmel";
    sourceCode->text = sourceText;

    auto tokens = scanSourceCode(sourceCode);
    /*for(auto token : tokens)
    {
        if(token->errorMessage.empty())
            printf("%s\n", getTokenKindName(token->kind));
        else
            printf("%s: %s\n", getTokenKindName(token->kind), token->errorMessage.c_str());
    }*/
    auto parseTree = parseTokens(sourceCode, tokens);
    auto parseTreeString = parseTree->printString();
    printf("%s\n", parseTreeString.c_str());
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
                evaluateAndPrintString(argv[++i]);
                
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
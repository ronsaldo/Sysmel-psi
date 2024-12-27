#include "Syntax.hpp"
#include "Environment.hpp"
#include "Type.hpp"

namespace Sysmel
{
ValuePtr SyntaxBinaryExpressionSequence::analyzeInEnvironment(const EnvironmentPtr &environment)
{
    auto result = elements[0]->analyzeInEnvironment(environment);
    for (size_t i = 1; i < elements.size(); i += 2)
    {
        auto operatorSymbol = elements[i];
        auto operand = elements[i + 1];

        auto messageSend = std::make_shared<SyntaxMessageSend> ();
        messageSend->sourcePosition = result->getSourcePosition()->to(operand->getSourcePosition());
        messageSend->receiver = result;
        messageSend->selector = operatorSymbol;
        messageSend->arguments.push_back(operand);
        result = messageSend->analyzeInEnvironment(environment);

    }
    return result;
}

ValuePtr SyntaxMessageSend::analyzeInEnvironment(const EnvironmentPtr &environment)
{
    auto analyzedSelector = selector->analyzeInEnvironment(environment);
    auto analyzedSelectorSymbol = analyzedSelector->asAnalyzedSymbolValue();
    if(!receiver && analyzedSelectorSymbol)
    {
        auto functional = environment->lookupSymbolRecursively(analyzedSelectorSymbol);
        abort();
    }
    
    auto analyzedReceiver = receiver->analyzeInEnvironment(environment);
    auto receiverTypeOrClass = analyzedReceiver->getTypeOrClass();

    std::vector<ValuePtr> analyzedArguments;
    analyzedArguments.reserve(arguments.size());
    for(auto argument : arguments)
    {
        analyzedArguments.push_back(argument->analyzeInEnvironment(environment));
    }

    auto analyzedMessage = std::make_shared<SemanticMessageSend> ();
    analyzedMessage->sourcePosition = sourcePosition;
    analyzedMessage->type = GradualType::uniqueInstance();
    analyzedMessage->receiver = analyzedReceiver;
    analyzedMessage->selector = analyzedSelectorSymbol;
    analyzedMessage->arguments.swap(analyzedArguments);

    return analyzedMessage;
}

ValuePtr SyntaxMessageCascade::analyzeInEnvironment(const EnvironmentPtr &environment)
{
    auto analyzedReceiver = receiver->analyzeInEnvironment(environment);
    if(messages.empty())
        return analyzedReceiver;

    auto sequence = std::make_shared<SemanticValueSequence> ();
    sequence->sourcePosition = sourcePosition;
    sequence->elements.reserve(1 + messages.size());
    sequence->elements.push_back(analyzedReceiver);

    for (auto &message : messages)
    {
        auto cascadeMessage = std::static_pointer_cast<SyntaxMessageCascadeMessage> (message);
        auto nonCascadedMessage = cascadeMessage->asMessageSendWithReceiver(analyzedReceiver);
        auto analyzedMessage = nonCascadedMessage->analyzeInEnvironment(environment);
        sequence->elements.push_back(analyzedMessage);
    }

    return sequence;
}

SyntaxMessageSendPtr SyntaxMessageCascadeMessage::asMessageSendWithReceiver(const ValuePtr &receiver)
{
    auto messageSend = std::make_shared<SyntaxMessageSend> ();
    messageSend->sourcePosition = sourcePosition;
    messageSend->receiver = receiver;
    messageSend->selector = selector;
    messageSend->arguments = arguments;
    return messageSend;
}
}
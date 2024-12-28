#include "Semantics.hpp"

namespace Sysmel
{

class SemanticPiArgumentAnalysisContext : public ArgumentTypeAnalysisContext
{
public:
    virtual ValuePtr coerceArgumentWithIndex(size_t index, ValuePtr argument)
    {
        if(index >= semanticPi->argumentBindings.size() && !semanticPi->isVariadic)
            throwExceptionWithMessageAt("Argument index is out of bounds.", argument->getSourcePosition());
        else if(index >= semanticPi->argumentBindings.size() && semanticPi->isVariadic)
            index = semanticPi->argumentBindings.size() - 1;

        auto argumentBinding = semanticPi->argumentBindings[index];
        return argument;
    }

    virtual ValuePtr getResultType()
    {
        return semanticPi->body;
    }

    SemanticPiPtr semanticPi;
};

ArgumentTypeAnalysisContextPtr SemanticPi::createArgumentTypeAnalysisContext()
{
    auto context = std::make_shared<SemanticPiArgumentAnalysisContext> ();
    context->semanticPi = std::static_pointer_cast<SemanticPi> (shared_from_this());
    return context;
}

} // End of namespace Sysmel
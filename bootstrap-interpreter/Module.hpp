#ifndef SYSMEL_MODULE_HPP
#define SYSMEL_MODULE_HPP

#include "Object.hpp"
#include "Namespace.hpp"
#include "Environment.hpp"

namespace Sysmel
{
typedef std::shared_ptr<class Module> ModulePtr;

class Module : public Object
{
public:
    void initializeWithName(const std::string &newName)
    {
        name = newName;
        globalNamespace = std::make_shared<Namespace> ();

        auto intrinsics = IntrinsicsEnvironment::uniqueInstance();
        moduleEnvironment = std::make_shared<ModuleEnvironment> (std::static_pointer_cast<Module> (shared_from_this()), intrinsics);
        globalNamespaceEnvironment = std::make_shared<NamespaceEnvironment> (globalNamespace, moduleEnvironment);
    }

    LexicalEnvironmentPtr newLexicalEnvironment(const SourcePositionPtr &position)
    {
        return std::make_shared<LexicalEnvironment> (globalNamespaceEnvironment, position);
    }

    std::string name;
    NamespacePtr globalNamespace;
    EnvironmentPtr moduleEnvironment;
    EnvironmentPtr globalNamespaceEnvironment;
};
} // End of namespace Sysmel

#endif //SYSMEL_MODULE_HPP
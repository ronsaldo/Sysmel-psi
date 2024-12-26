#ifndef SYSMEL_MODULE_HPP
#define SYSMEL_MODULE_HPP

#include "Object.hpp"
#include "Namespace.hpp"

namespace Sysmel
{
typedef std::shared_ptr<class Module> ModulePtr;

class Module : public Object
{
public:
    NamespacePtr globalNamespace;
};
} // End of namespace Sysmel

#endif //SYSMEL_MODULE_HPP
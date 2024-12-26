#include "Value.hpp"
#include "Type.hpp"

namespace Sysmel
{
ValuePtr Value::getType()
{
    return GradualType::uniqueInstance();
}

ValuePtr Value::getClass() const
{
    return nullptr;
}

} // End of namespace Sysmel

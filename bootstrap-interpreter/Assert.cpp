#include "Assert.hpp"
#include <stdlib.h>
#include <stdio.h>

namespace Sysmel
{

[[noreturn]] void assertFailureWithMessage(const char *message)
{
    fprintf(stderr, "%s\n", message);
    abort();
}

} // End of namespace Sysmel
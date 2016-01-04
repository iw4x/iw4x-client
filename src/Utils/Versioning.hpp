#include <version.hpp>

// Revision number
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define REVISION_STR STRINGIZE(REVISION)
#define VERSION 4,2,REVISION
#define VERSION_STR "4.2." REVISION_STR

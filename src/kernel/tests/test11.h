#ifndef _TEST11_H_
#define _TEST11_H_

#include "sysapi_kernel.h"
#include "tests.h"

struct test11_shared {
        union sem sem;
        int in_mutex;
};

#endif

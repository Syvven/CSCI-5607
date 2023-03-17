#ifndef SECOND_UTILS_H_
#define SECOND_UTILS_H_

#include <iostream>

static int get_id()
{
    static int count = 0;
    return count++;
}

#endif 
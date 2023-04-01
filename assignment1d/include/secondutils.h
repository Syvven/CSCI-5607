#ifndef SECOND_UTILS_H_
#define SECOND_UTILS_H_

#include <iostream>
#include <atomic>

class Utils {
public:
    static int getUid()
    {
        static std::atomic<std::uint32_t> uid { 0 };
        return ++uid;
    }
};

#endif 
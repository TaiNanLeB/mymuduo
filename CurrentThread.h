#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;   //__thread是修饰:虽然是全局变量，但会在每个线程存一个拷贝
                                           
    void cacheTid();

    inline int tid()// inline 只在当前文件起作用
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}
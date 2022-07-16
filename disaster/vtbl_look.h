// vtbl_look.h
// Copyright(C) 2018-2021 Pacific Light & Hologram Inc. All rights reserved.

#pragma once

#include <assert.h>

struct virtual_count_t
{
#define CNX(a, b) a##b
#define CX(a, b) CNX(a, b)
#define X0(n) virtual int CX(get_, __COUNTER__)() { return n; }
#define X1(n) X0(n) X0(n+1)
#define X2(n) X1(n) X1(n+2)
#define X3(n) X2(n) X2(n+4)
#define X4(n) X3(n) X3(n+8)
#define X5(n) X4(n) X4(n+16)
#define X6(n) X5(n) X5(n+32)
#define X7(n) X6(n) X6(n+64)
#define X8(n) X7(n) X7(n+128)
    X8(0)
#undef CNX
#undef CX
#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
#undef X5
#undef X6
#undef X7
#undef X8
    // look, a vtbl _could_ get larger than 256 entries
    // it really shouldn't get larger, but if it does, stuff will crash
    // and I won't feel bad about it
} __declspec(selectany) the_count;

template <typename T>
int get_vtbl_size()
{
    struct S : public T
    {
        virtual int get_answer() { return 42; }
        S() = delete; // this
    };
    return reinterpret_cast<S*>(&the_count)->get_answer();
}

template <typename T, typename R, typename ...aaaargs>
int get_vtbl_func_index(R(T::* f)(aaaargs...))
{
    return (reinterpret_cast<T*>(&the_count)->*(int (T::*)())f)();
}

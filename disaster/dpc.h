// dpc.cpp
// Copyright(C) 2018-2022 Pacific Light & Hologram Inc. All rights reserved.

#pragma once
#include "vtbl_look.h"

#include <cstdint>
#include <memory>
using std::unique_ptr;
using std::make_unique;

struct emit_dpc_context;
void emit_dpc_thunk_aaaarg_0f(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_1f(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_1t(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_2f(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_2t(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_3f(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_3t(emit_dpc_context&);
void emit_dpc_thunk_aaaarg_vf(emit_dpc_context&, int);
void emit_dpc_thunk_last_aaaarg(emit_dpc_context&, int);

template <int aaaarg_idx, typename aaaarg, bool is_float = (std::is_floating_point<aaaarg>::value && aaaarg_idx < 4)>
struct emit_dpc_thunk_aaaarg;

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<0, aaaarg, /* is_float = */ false>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_0f(ctx); };
};

// 'this' as a float, have you gone mad?!

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<1, aaaarg, /* is_float = */ false>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_1f(ctx); };
};

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<1, aaaarg, /* is_float = */ true>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_1t(ctx); };
};

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<2, aaaarg, /* is_float = */ false>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_2f(ctx); };
};

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<2, aaaarg, /* is_float = */ true>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_2t(ctx); };
};

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<3, aaaarg, /* is_float = */ false>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_3f(ctx); };
};

template <typename aaaarg>
struct emit_dpc_thunk_aaaarg<3, aaaarg, /* is_float = */ true>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_3t(ctx); };
};

template <int aaaarg_idx, typename aaaarg>
struct emit_dpc_thunk_aaaarg<aaaarg_idx, aaaarg, /* is_float = */ false>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_aaaarg_vf(ctx, aaaarg_idx); };
};

template <int aaaarg_idx, typename... aaaargs>
struct emit_dpc_thunk;

template <int aaaarg_idx, typename aaaarg, typename... aaaargs>
struct emit_dpc_thunk<aaaarg_idx, aaaarg, aaaargs...>
{
    void operator()(emit_dpc_context& ctx)
    {
        // emit aaaarg
        emit_dpc_thunk_aaaarg<aaaarg_idx, aaaarg>()(ctx);

        // respecialized recurse
        emit_dpc_thunk<aaaarg_idx + 1, aaaargs...>()(ctx);
    };
};

template <int last_aaaarg_idx>
struct emit_dpc_thunk<last_aaaarg_idx>
{
    void operator()(emit_dpc_context& ctx) { emit_dpc_thunk_last_aaaarg(ctx, last_aaaarg_idx); };
};

template <typename T, typename ...aaaargs>
void emit_dpc_thunk_body(emit_dpc_context& ctx)
{
    emit_dpc_thunk<0, void*, aaaargs...>()(ctx);
}

intptr_t make_breakpoint_thunk();
void make_dpc_vtbl_thunk(intptr_t* capture_vtbl, intptr_t* unspill_vtbl, int vtbl_func_index, void(*emit_dpc_thunk_body)(emit_dpc_context&));

struct dpc_capture_base_t
{
    intptr_t* capture_vtbl{ nullptr }; // arrested for vtbl crimes
    uint64_t* dpc_cursor{ 0 };
    uint64_t* dpc_max{ 0 }; // TODO capture thunk bounds check and/or enforce safety/crash
    unique_ptr<uint64_t[]> dpc_storage;

    // these could be static, but this is more exciting
    unique_ptr<intptr_t[]> capture_vtbl_storage;
    intptr_t* capture_vtbl_end{ nullptr };
    unique_ptr<intptr_t[]> unspill_vtbl_storage;
    void (*replay_dpc_thunk)(uint64_t* dpc_begin, uint64_t* dpc_end, intptr_t* unspill_vtbl, intptr_t* replay_vtbl, void* replay_this) { nullptr };

    dpc_capture_base_t(uint32_t storage_size, uint32_t vtbl_size);
    ~dpc_capture_base_t();

    void reset_cursor();
    void respill_dpc(dpc_capture_base_t* other);
    void rebuild_replay_thunk();
    void replay_dpc(intptr_t* replay_vtbl, void* replay_this);
    bool is_capture_instance_complete();
    void install_debug_breakpoints();
};

template <typename TInterface>
struct dpc_capture_t : public dpc_capture_base_t
{
    dpc_capture_t(uint32_t storage_size) : dpc_capture_base_t(storage_size, get_vtbl_size<TInterface>()) { install_default_vtbl(); }

    TInterface* as_capture_instance()
    {
        return (TInterface*)&capture_vtbl;
    }

    TInterface* operator->()
    {
        return (TInterface*)&capture_vtbl;
    }

    static dpc_capture_t* from_capture_instance(TInterface* capture)
    {
        return (dpc_capture_t*)(intptr_t(capture) - offsetof(dpc_capture_t, capture_vtbl));
    }

    template <typename T, typename R, typename ...aaaargs>
    void install_debug_breakpoint(R(TInterface::* member_func)(aaaargs...))
    {
        const int vtbl_func_index = get_vtbl_func_index(member_func);
        capture_vtbl[vtbl_func_index] = make_breakpoint_thunk();
    }

    template <typename ...aaaargs>
    void install_dpc_func(void(TInterface::* member_func)(aaaargs...))
    {
        const int vtbl_func_index = get_vtbl_func_index(member_func);
        make_dpc_vtbl_thunk(capture_vtbl_storage.get(), unspill_vtbl_storage.get(), vtbl_func_index, emit_dpc_thunk_body<TInterface, aaaargs...>);
    }

    void install_default_vtbl();
};

struct dpc_playback_base_t
{
    unique_ptr<intptr_t[]> playback_vtbl_storage;
    intptr_t* playback_vtbl_end{ nullptr };

    dpc_playback_base_t(uint32_t vtbl_size);
    bool is_playback_instance_complete();
    void install_debug_breakpoints();
};

template <typename TInterface, typename TReplayThis>
struct dpc_playback_t : public dpc_playback_base_t
{
    dpc_playback_t() : dpc_playback_base_t(get_vtbl_size<TInterface>()) {}

    template <typename R, typename ...aaaargs>
    void install_debug_breakpoint(R(TInterface::* member_func)(aaaargs...))
    {
        const int vtbl_func_index = get_vtbl_func_index(member_func);
        playback_vtbl_storage[vtbl_func_index] = make_breakpoint_thunk();
    }

    template <typename ...aaaargs0, typename ...aaaargs1>
    void install_playback_func(void(TInterface::* capture_thunk)(aaaargs0...), void(TReplayThis::* playback_func)(aaaargs1...))
    {
        int vtbl_func_index = get_vtbl_func_index(capture_thunk);
        playback_vtbl_storage[vtbl_func_index] = reinterpret_cast<intptr_t&>(playback_func);
    }

    void replay_capture(TReplayThis* replay_this, dpc_capture_t<TInterface>& dpc)
    {
        dpc.replay_dpc(playback_vtbl_storage.get(), replay_this);
    }
};
  
// dpc.cpp
// Copyright(C) 2018-2022 Pacific Light & Hologram Inc. All rights reserved.

#include "dpc.h"
#include <asmjit/asmjit.h>

// https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64

typedef uint8_t UBYTE;
typedef uint16_t USHORT;
typedef uint32_t ULONG;
typedef int8_t BOOLEAN;
typedef uint32_t DWORD;
typedef uint64_t DWORD64;

typedef enum _UNWIND_OP_CODES {
    UWOP_PUSH_NONVOL = 0, /* info == register number */
    UWOP_ALLOC_LARGE,     /* no info, alloc size in next 2 slots */
    UWOP_ALLOC_SMALL,     /* info == size of allocation / 8 - 1 */
    UWOP_SET_FPREG,       /* no info, FP = RSP + UNWIND_INFO.FPRegOffset*16 */
    UWOP_SAVE_NONVOL,     /* info == register number, offset in next slot */
    UWOP_SAVE_NONVOL_FAR, /* info == register number, offset in next 2 slots */
    UWOP_SAVE_XMM128 = 8, /* info == XMM reg number, offset in next slot */
    UWOP_SAVE_XMM128_FAR, /* info == XMM reg number, offset in next 2 slots */
    UWOP_PUSH_MACHFRAME   /* info == 0: no error-code, 1: error-code */
} UNWIND_CODE_OPS;

typedef union _UNWIND_CODE {
    struct {
        UBYTE CodeOffset;
        UBYTE UnwindOp : 4;
        UBYTE OpInfo : 4;
    };
    USHORT FrameOffset;
} UNWIND_CODE, * PUNWIND_CODE;

#define UNW_FLAG_EHANDLER  0x01
#define UNW_FLAG_UHANDLER  0x02
#define UNW_FLAG_CHAININFO 0x04

typedef struct _UNWIND_INFO {
    UBYTE Version : 3;
    UBYTE Flags : 5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister : 4;
    UBYTE FrameOffset : 4;
    //UNWIND_CODE UnwindCode[1];
    /*  UNWIND_CODE MoreUnwindCode[((CountOfCodes + 1) & ~1) - 1];
    *   union {
    *       OPTIONAL ULONG ExceptionHandler;
    *       OPTIONAL ULONG FunctionEntry;
    *   };
    *   OPTIONAL ULONG ExceptionData[]; */
} UNWIND_INFO, * PUNWIND_INFO;

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, * PRUNTIME_FUNCTION;

extern "C" __declspec(dllimport) BOOLEAN RtlAddFunctionTable(
    PRUNTIME_FUNCTION FunctionTable,
    DWORD             EntryCount,
    DWORD64           BaseAddress
);

extern "C" __declspec(dllimport) BOOLEAN RtlDeleteFunctionTable(
    PRUNTIME_FUNCTION FunctionTable
);

asmjit::JitRuntime jit_rt;

#ifdef DEBUG

#define JIT_SAFE_CALL(x)                                                                                \
    do                                                                                                  \
    {                                                                                                   \
        asmjit::Error result = x;                                                                       \
        if (result != asmjit::kErrorOk)                                                                 \
        {                                                                                               \
            __debugbreak();                                                                             \
        }                                                                                               \
    } while (0)

#else

#define JIT_SAFE_CALL(x)                                                                                \
    do                                                                                                  \
    {                                                                                                   \
        asmjit::Error result = x;                                                                       \
        if (result != asmjit::kErrorOk)                                                                 \
        {                                                                                               \
            exit(0);                                                                                    \
        }                                                                                               \
    } while (0)

#endif

struct emit_dpc_context
{
    asmjit::x86::Assembler& ca;
    asmjit::x86::Assembler& ua;
};

void emit_dpc_thunk_aaaarg_0f(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    // dpc capture prolog stores vtbl func index in r10, keep that instead of 'this' from rcx
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax), r10));

    // no dpc unspill, replay func stores replay 'this' in rcx before calling
}

void emit_dpc_thunk_aaaarg_1f(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 1 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_offset), rdx));

    JIT_SAFE_CALL(ctx.ua.mov(rdx, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_1t(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 1 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.movq(ptr(rax, dpc_offset), xmm1));

    JIT_SAFE_CALL(ctx.ua.movq(xmm1, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_2f(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 2 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_offset), r8));

    JIT_SAFE_CALL(ctx.ua.mov(r8, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_2t(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 2 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.movq(ptr(rax, dpc_offset), xmm2));

    JIT_SAFE_CALL(ctx.ua.movq(xmm2, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_3f(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 3 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_offset), r9));

    JIT_SAFE_CALL(ctx.ua.mov(r9, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_3t(emit_dpc_context& ctx)
{
    using namespace asmjit;
    using namespace x86;

    constexpr int dpc_offset = 3 * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.movq(ptr(rax, dpc_offset), xmm3));

    JIT_SAFE_CALL(ctx.ua.movq(xmm3, ptr(rax, dpc_offset)));
}

void emit_dpc_thunk_aaaarg_vf(emit_dpc_context& ctx, int aaaarg_idx)
{
    // TODO verify aaaarg larger than a register (i.e slightly larger struct)
    using namespace asmjit;
    using namespace x86;

    int dpc_offset = aaaarg_idx * sizeof uint64_t;
    int stack_offset = (aaaarg_idx + 1) * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.mov(r11, ptr(rsp, stack_offset)));
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_offset), r11));

    JIT_SAFE_CALL(ctx.ua.mov(r11, ptr(rax, dpc_offset)));
    JIT_SAFE_CALL(ctx.ua.mov(ptr(rsp, stack_offset), r11));
}

// for partial unspill
constexpr uint64_t dpc_trailer_1 = 0x21474E494E524157ull;
constexpr uint64_t dpc_trailer_2 = 0x2152455645524F46ull;

void emit_dpc_thunk_last_aaaarg(emit_dpc_context& ctx, int last_aaaarg_idx)
{
    using namespace asmjit;
    using namespace x86;

    // fin
    int dpc_trailer_begin = last_aaaarg_idx * sizeof uint64_t;
    int dpc_trailer_end = (last_aaaarg_idx + 2) * sizeof uint64_t;
    JIT_SAFE_CALL(ctx.ca.mov(r10, dpc_trailer_1));
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_trailer_begin), r10));
    JIT_SAFE_CALL(ctx.ca.mov(r10, dpc_trailer_2));
    JIT_SAFE_CALL(ctx.ca.mov(ptr(rax, dpc_trailer_begin + sizeof uint64_t), r10));
    JIT_SAFE_CALL(ctx.ca.add(rax, dpc_trailer_end));

    JIT_SAFE_CALL(ctx.ua.add(rax, dpc_trailer_end));
}

intptr_t make_breakpoint_thunk()
{
    using namespace asmjit;
    using namespace x86;

    CodeHolder code;
    JIT_SAFE_CALL(code.init(jit_rt.environment()));

    Assembler a(&code);

#ifdef DEBUG
    JIT_SAFE_CALL(a.int3());
#else
    // TODO a more evil crash perhaps?
    JIT_SAFE_CALL(a.int3());
#endif

    void* vf;
    JIT_SAFE_CALL(jit_rt.add(&vf, &code));
    return (intptr_t)vf;
}

void make_dpc_vtbl_thunk(intptr_t* capture_vtbl, intptr_t* unspill_vtbl, int vtbl_func_index, void(*emit_dpc_thunk_body)(emit_dpc_context&))
{
    using namespace asmjit;
    using namespace x86;

    auto& capture_vf = capture_vtbl[vtbl_func_index];
    auto& unspill_vf = unspill_vtbl[vtbl_func_index];
    assert(!capture_vf);
    assert(!unspill_vf);

    CodeHolder capture_code;
    CodeHolder unspill_code;

    JIT_SAFE_CALL(capture_code.init(jit_rt.environment()));
    JIT_SAFE_CALL(unspill_code.init(jit_rt.environment()));

    Assembler ca(&capture_code);
    Assembler ua(&unspill_code);

    // capture prolog
    int32_t capture_to_dpc_adj = -int32_t(offsetof(dpc_capture_base_t, capture_vtbl)) + offsetof(dpc_capture_base_t, dpc_cursor);
    JIT_SAFE_CALL(ca.mov(rax, ptr(rcx, capture_to_dpc_adj)));
    JIT_SAFE_CALL(ca.mov(r10, vtbl_func_index * sizeof uint64_t));

    // no unspill prolog

    // emit both dpc function bodies
    emit_dpc_context dpc_ctx{ ca, ua };
    emit_dpc_thunk_body(dpc_ctx);

    // capture epilog
    JIT_SAFE_CALL(ca.mov(ptr(rcx, capture_to_dpc_adj), rax));
    JIT_SAFE_CALL(ca.xor_(rax, rax)); // keep rax to ourselves
    JIT_SAFE_CALL(ca.ret());

    // unspill epilog
    JIT_SAFE_CALL(ua.ret());

    JIT_SAFE_CALL(jit_rt.add(&capture_vf, &capture_code));
    JIT_SAFE_CALL(jit_rt.add(&unspill_vf, &unspill_code));
}

dpc_capture_base_t::dpc_capture_base_t(uint32_t storage_size, uint32_t vtbl_size)
    : dpc_storage(make_unique<uint64_t[]>(storage_size))
    , capture_vtbl_storage(make_unique<intptr_t[]>(vtbl_size))
    , unspill_vtbl_storage(make_unique<intptr_t[]>(vtbl_size))
{
    dpc_max = dpc_storage.get() + storage_size;
    reset_cursor();

    capture_vtbl = capture_vtbl_storage.get();
    capture_vtbl_end = capture_vtbl + vtbl_size;

    rebuild_replay_thunk();
}

dpc_capture_base_t::~dpc_capture_base_t()
{
    auto unspill_vtbl_ptr = unspill_vtbl_storage.get();
    for (auto capture_vtbl_ptr = capture_vtbl_storage.get(); capture_vtbl_ptr < capture_vtbl_end; ++capture_vtbl_ptr, ++unspill_vtbl_ptr)
    {
        if (*unspill_vtbl_ptr)
            JIT_SAFE_CALL(jit_rt.release(*unspill_vtbl_ptr));
        if (*capture_vtbl_ptr)
            JIT_SAFE_CALL(jit_rt.release(*capture_vtbl_ptr));
    }
    JIT_SAFE_CALL(jit_rt.release(replay_dpc_thunk));
}

void dpc_capture_base_t::reset_cursor()
{
    dpc_cursor = dpc_storage.get();
}

void dpc_capture_base_t::respill_dpc(dpc_capture_base_t* other)
{
    auto other_dpc_size = intptr_t(other->dpc_cursor) - intptr_t(other->dpc_storage.get());
    if (intptr_t(dpc_cursor) + other_dpc_size <= intptr_t(dpc_max))
    {
        memcpy(dpc_cursor, other->dpc_storage.get(), other_dpc_size);
        dpc_cursor = (uint64_t*)(intptr_t(dpc_cursor) + other_dpc_size);
        return;
    }

    assert(false); // TODO partial respill
}

void dpc_capture_base_t::rebuild_replay_thunk()
{
    if (!replay_dpc_thunk)
    {
        using namespace asmjit;
        using namespace x86;

        CodeHolder code;
        JIT_SAFE_CALL(code.init(jit_rt.environment()));

        Assembler a(&code);

        // void replay_dpc_thunk(dpc_begin, dpc_end, unspill_vtbl, replay_vtbl, replay_this)
        // begin non-leaf prolog
        Label lbl_prolog_start = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_prolog_start));
        JIT_SAFE_CALL(a.sub(rsp, 0x48));
        Label lbl_stack_alloc = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_stack_alloc));
        JIT_SAFE_CALL(a.mov(ptr(rsp, 0x50), rcx)); // dpc_begin -> dpc_cursor
        JIT_SAFE_CALL(a.mov(ptr(rsp, 0x58), rdx)); // dpc_end
        JIT_SAFE_CALL(a.mov(ptr(rsp, 0x60), r8)); // unspill_vtbl
        JIT_SAFE_CALL(a.mov(ptr(rsp, 0x68), r9)); // replay_vtbl
        Label lbl_prolog_end = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_prolog_end));

        Label lbl_dpc_loop_start = a.newLabel();
        Label lbl_dpc_loop_end = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_dpc_loop_start));

        // call unspill to unpack parameters
        JIT_SAFE_CALL(a.mov(rax, ptr(rsp, 0x50))); // dpc_cursor to rax for unspill
        JIT_SAFE_CALL(a.cmp(ptr(rsp, 0x58), rax));
        JIT_SAFE_CALL(a.je(lbl_dpc_loop_end));
        JIT_SAFE_CALL(a.mov(r10, ptr(rax))); // vtbl_func_index to r10
        JIT_SAFE_CALL(a.mov(r8, ptr(rsp, 0x60))); // restore unspill_vtbl
        JIT_SAFE_CALL(a.add(r8, r10)); // call unspill func
        JIT_SAFE_CALL(a.call(ptr(r8)));

        // replay dpc function call with replay_this
        JIT_SAFE_CALL(a.mov(rcx, ptr(rsp, 0x70))); // replay_this
        JIT_SAFE_CALL(a.mov(r11, ptr(rsp, 0x68))); // replay_vtbl
        JIT_SAFE_CALL(a.add(r11, r10)); // + vtbl_func_index
        JIT_SAFE_CALL(a.mov(ptr(rsp, 0x50), rax));
        JIT_SAFE_CALL(a.call(ptr(r11))); // call replay virtual func

        // loop until done
        JIT_SAFE_CALL(a.jmp(lbl_dpc_loop_start));
        JIT_SAFE_CALL(a.bind(lbl_dpc_loop_end));

        // begin non-leaf epilog
        JIT_SAFE_CALL(a.add(rsp, 0x48));
        JIT_SAFE_CALL(a.ret());
        Label lbl_func_end = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_func_end));

        // add unwind info so debugger/exception stackwalker is happy later
        UNWIND_CODE uc[] = {
            {(uint8_t)code.labelOffset(lbl_stack_alloc), UWOP_ALLOC_SMALL, 0x40 / 8}, // sub rsp 0x48
        };

        UNWIND_INFO ui{};
        ui.Version = 1;
        ui.Flags = 0;
        ui.SizeOfProlog = UBYTE(code.labelOffset(lbl_prolog_end) - code.labelOffset(lbl_prolog_start));
        ui.FrameRegister = 0;
        ui.CountOfCodes = _countof(uc);

        RUNTIME_FUNCTION rf;
        rf.BeginAddress = 0;
        rf.EndAddress = ULONG(code.labelOffset(lbl_func_end) - code.labelOffset(lbl_prolog_start));
        rf.UnwindData = rf.EndAddress;

        JIT_SAFE_CALL(a.embedDataArray(Type::kIdU8, &ui, sizeof(ui)));
        JIT_SAFE_CALL(a.embedDataArray(Type::kIdU8, uc, sizeof(ui)));
        Label lbl_rf_start = a.newLabel();
        JIT_SAFE_CALL(a.bind(lbl_rf_start));
        JIT_SAFE_CALL(a.embedDataArray(Type::kIdU8, &rf, sizeof(rf)));

        JIT_SAFE_CALL(jit_rt.add(&replay_dpc_thunk, &code));

        uint64_t base_address = code.baseAddress();
        uint64_t rf_start = code.labelOffset(lbl_rf_start); 
        RtlAddFunctionTable(PRUNTIME_FUNCTION(rf_start + base_address), 1, base_address);
        // FIXME call RtlDeleteFunctionTable(PRUNTIME_FUNCTION(rf_start + base_address)) somehow later
    }
}

void dpc_capture_base_t::replay_dpc(intptr_t* replay_vtbl, void* replay_this)
{
    (*replay_dpc_thunk)(dpc_storage.get(), dpc_cursor, unspill_vtbl_storage.get(), replay_vtbl, replay_this);
}

bool dpc_capture_base_t::is_capture_instance_complete()
{
    for (auto capture_vtbl_ptr = capture_vtbl_storage.get(); capture_vtbl_ptr < capture_vtbl_end; ++capture_vtbl_ptr)
    {
        // please keep your observable instance inside the C++ type system at all times 
        assert(*capture_vtbl_ptr);

        if (!*capture_vtbl_ptr)
            return false;
    }
    return true;
}

void dpc_capture_base_t::install_debug_breakpoints()
{
    auto unspill_vtbl_ptr = unspill_vtbl_storage.get();
    for (auto capture_vtbl_ptr = capture_vtbl_storage.get(); capture_vtbl_ptr < capture_vtbl_end; ++capture_vtbl_ptr)
    {
        // unless you work here
        if (!*capture_vtbl_ptr)
            *capture_vtbl_ptr = make_breakpoint_thunk();

        if (!*unspill_vtbl_ptr)
            *unspill_vtbl_ptr = make_breakpoint_thunk();
    }
}

dpc_playback_base_t::dpc_playback_base_t(uint32_t vtbl_size)
    : playback_vtbl_storage(make_unique<intptr_t[]>(vtbl_size))
{
    playback_vtbl_end = playback_vtbl_storage.get() + vtbl_size;
}

bool dpc_playback_base_t::is_playback_instance_complete()
{
    for (auto playback_vtbl_ptr = playback_vtbl_storage.get(); playback_vtbl_ptr < playback_vtbl_end; ++playback_vtbl_ptr)
    {
        // please keep your observable instance inside the C++ type system at all times 
        assert(*playback_vtbl_ptr);

        if (!*playback_vtbl_ptr)
            return false;
    }
    return true;
}

void dpc_playback_base_t::install_debug_breakpoints()
{
    for (auto playback_vtbl_ptr = playback_vtbl_storage.get(); playback_vtbl_ptr < playback_vtbl_end; ++playback_vtbl_ptr)
    {
        // unless you work here
        if (!*playback_vtbl_ptr)
            *playback_vtbl_ptr = make_breakpoint_thunk();
    }
}

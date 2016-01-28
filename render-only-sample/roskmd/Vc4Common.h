#ifndef _VC4COMMON_HPP_
#define _VC4COMMON_HPP_ 1

//
// Copyright (C) Microsoft.  All rights reserved.
//
//
// Module Name:
//
//  Vc4Common.hpp
//
// Abstract:
//
//    This is VC4DOD shared utilities include file
//
// Author:
//
//    Jordan Rhee (jordanrh) November 2015
//
// Environment:
//
//    Kernel mode only.
//

//
// Macros to be used for proper PAGED/NON-PAGED code placement
//

#define VC4_NONPAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    //__pragma(code_seg(.text))

#define VC4_NONPAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define VC4_PAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("PAGE"))

#define VC4_PAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define VC4_INIT_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("INIT"))

#define VC4_INIT_SEGMENT_END \
    __pragma(code_seg(pop))

//
// We have some non-paged functions that supposed to be called on low IRQL.
// The following macro defines unified assert to be put at the beginning of
// such functions.
//
// NOTE: We can't use standard PAGED_CODE macro as it requires function to be
// placed in paged segment during compilation.
//
#define VC4_ASSERT_MAX_IRQL(Irql) NT_ASSERT(KeGetCurrentIrql() <= (Irql))
#define VC4_ASSERT_LOW_IRQL() VC4_ASSERT_MAX_IRQL(APC_LEVEL)

//
// These inline function and macro allow to specify "true" or "false" as
// condition for various statements avoiding compiler warnings C4127 and C4702
//
// NOTE: The "volatile" version has non-trivial perf effect, please, use with
// caution!!!
//
__forceinline bool Vc4GetFalse () { return false; }
#define VC4_FALSE (Vc4GetFalse())
#define VC4_TRUE (!VC4_FALSE)

__forceinline bool Vc4GetVolatileFalse () { volatile bool f = false; return f; }
#define VC4_VOLATILE_FALSE (Vc4GetVolatileFalse())
#define VC4_VOLATILE_TRUE (!VC4_VOLATILE_FALSE)

//
// Pool allocation tags for use by VC4DOD
//
enum VC4_ALLOC_TAG : ULONG {
    TEMP                     = '04CV', // will be freed in the same routine
    GLOBAL                   = '14CV',
    DEVICE                   = '24CV',
}; // enum VC4_ALLOC_TAG

_When_(Status < 0, _Post_satisfies_(return < 0)) _When_(Status >= 0, _Post_satisfies_(return >= 0))
__forceinline NTSTATUS Vc4SanitizeNtstatus (
    NTSTATUS Status,
    NTSTATUS PassThrough1 = STATUS_SUCCESS,
    NTSTATUS PassThrough2 = STATUS_SUCCESS,
    NTSTATUS PassThrough3 = STATUS_SUCCESS
    )
{
    if ((Status == PassThrough1) ||
        (Status == PassThrough2) ||
        (Status == PassThrough3) ||
        (Status == STATUS_INSUFFICIENT_RESOURCES) ||
        (Status == STATUS_NO_MEMORY)) {

        return Status;
    } // if

    return NT_SUCCESS(Status) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
} // Vc4SanitizeNtstatus (...)

//
// Default memory allocation and object construction for C++ modules
//

__forceinline void* __cdecl operator new (
    size_t Size,
    POOL_TYPE PoolType,
    VC4_ALLOC_TAG Tag
    ) throw ()
{
    if (!Size) Size = 1;
    return ExAllocatePoolWithTag(PoolType, Size, ULONG(Tag));
} // operator new ( size_t, POOL_TYPE, VC4_ALLOC_TAG )

__forceinline void __cdecl operator delete ( void* Ptr ) throw ()
{
    if (Ptr) ExFreePool(Ptr);
} // operator delete ( void* )

__forceinline void __cdecl operator delete (void* Ptr, size_t) throw ()
{
    if (Ptr) ExFreePool(Ptr);
} // operator delete (void*, size_t)

__forceinline void* __cdecl operator new[] (
    size_t Size,
    POOL_TYPE PoolType,
    VC4_ALLOC_TAG Tag
    ) throw ()
{
    if (!Size) Size = 1;
    return ExAllocatePoolWithTag(PoolType, Size, ULONG(Tag));
} // operator new[] ( size_t, POOL_TYPE, VC4_ALLOC_TAG )

__forceinline void __cdecl operator delete[] ( void* Ptr ) throw ()
{
    if (Ptr) ExFreePool(Ptr);
} // operator delete[] ( void* )

__forceinline void* __cdecl operator new ( size_t, void* Ptr ) throw ()
{
    return Ptr;
} // operator new ( size_t, void* )

__forceinline void __cdecl operator delete ( void*, void* ) throw ()
{} // void operator delete ( void*, void* )

__forceinline void* __cdecl operator new[] ( size_t, void* Ptr ) throw ()
{
    return Ptr;
} // operator new[] ( size_t, void* )

__forceinline void __cdecl operator delete[] ( void*, void* ) throw ()
{} // void operator delete[] ( void*, void* )

//
// class VC4_FINALLY
//
class VC4_FINALLY {
private:

    VC4_FINALLY () = delete;

    template < typename T_OP > class _FINALIZER {
        friend class VC4_FINALLY;
        T_OP op;
        __forceinline _FINALIZER ( T_OP Op ) throw () : op(Op) {}
    public:
        __forceinline ~_FINALIZER () { (void)this->op(); }
    }; // class _FINALIZER

    template < typename T_OP > class _FINALIZER_EX {
        friend class VC4_FINALLY;
        bool doNot;
        T_OP op;
        __forceinline _FINALIZER_EX ( T_OP Op, bool DoNot ) throw () : doNot(DoNot), op(Op) {}
    public:
        __forceinline ~_FINALIZER_EX () { if (!this->doNot) (void)this->op(); }
        __forceinline void DoNot (bool DoNot = true) throw () { this->doNot = DoNot; }
        __forceinline void DoNow () throw () { this->DoNot(); (void)this->op(); }
    }; // class _FINALIZER_EX

public:

    template < typename T_OP > __forceinline static _FINALIZER<T_OP> Do ( T_OP Op ) throw ()
    {
        return _FINALIZER<T_OP>(Op);
    }; // Do<...> (...)

    template < typename T_OP > __forceinline static _FINALIZER_EX<T_OP> DoUnless (
        T_OP Op,
        bool DoNot = false
        ) throw ()
    {
        return _FINALIZER_EX<T_OP>(Op, DoNot);
    }; // DoUnless<...> (...)
}; // class VC4_FINALLY

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS Vc4OpenDevice (
    _In_ UNICODE_STRING* FileNamePtr,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    _Out_ FILE_OBJECT** FileObjectPPtr,
    VC4_ALLOC_TAG AllocTag = VC4_ALLOC_TAG::DEVICE
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS Vc4SendWriteSynchronously (
    _In_ FILE_OBJECT* FileObjectPtr,
    _In_reads_bytes_opt_(InputBufferLength) void* InputBufferPtr,
    ULONG InputBufferLength,
    _Out_ ULONG* InformationPtr
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS Vc4SendIoctlSynchronously (
    _In_ FILE_OBJECT* FileObjectPtr,
    ULONG IoControlCode,
    _In_reads_bytes_opt_(InputBufferLength) void* InputBufferPtr,
    ULONG InputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) void* OutputBufferPtr,
    ULONG OutputBufferLength,
    BOOLEAN InternalDeviceIoControl,
    _Out_ ULONG* InformationPtr
    );

#endif // _VC4COMMON_HPP_

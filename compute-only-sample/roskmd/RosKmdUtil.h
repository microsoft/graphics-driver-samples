#ifndef _ROSKMDUTIL_H_
#define _ROSKMDUTIL_H_ 1

//
// Copyright (C) Microsoft.  All rights reserved.
//
//
// Module Name:
//
//  RosKmdUtil.h
//
// Abstract:
//
//    This is RosKmd shared utilities include file
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

#define ROS_NONPAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    //__pragma(code_seg(.text))

#define ROS_NONPAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define ROS_PAGED_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("PAGE"))

#define ROS_PAGED_SEGMENT_END \
    __pragma(code_seg(pop))

#define ROS_INIT_SEGMENT_BEGIN \
    __pragma(code_seg(push)) \
    __pragma(code_seg("INIT"))

#define ROS_INIT_SEGMENT_END \
    __pragma(code_seg(pop))

//
// We have some non-paged functions that supposed to be called on low IRQL.
// The following macro defines unified assert to be put at the beginning of
// such functions.
//
// NOTE: We can't use standard PAGED_CODE macro as it requires function to be
// placed in paged segment during compilation.
//
#define ROS_ASSERT_MAX_IRQL(Irql) NT_ASSERT(KeGetCurrentIrql() <= (Irql))
#define ROS_ASSERT_LOW_IRQL() ROS_ASSERT_MAX_IRQL(APC_LEVEL)

//
// Pool allocation tags for use by RosKmd
//
enum ROS_ALLOC_TAG : ULONG {
    TEMP                     = '04CV', // will be freed in the same routine
    GLOBAL                   = '14CV',
    DEVICE                   = '24CV',
}; // enum ROS_ALLOC_TAG

_When_(Status < 0, _Post_satisfies_(return < 0)) _When_(Status >= 0, _Post_satisfies_(return >= 0))
__forceinline NTSTATUS RosSanitizeNtstatus (
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
} // RosSanitizeNtstatus (...)

//
// Default memory allocation and object construction for C++ modules
//

void* __cdecl operator new (
    size_t Size,
    POOL_TYPE PoolType,
    ROS_ALLOC_TAG Tag
    ) throw ();

void __cdecl operator delete ( void* Ptr ) throw ();

void __cdecl operator delete (void* Ptr, size_t) throw ();

void* __cdecl operator new[] (
    size_t Size,
    POOL_TYPE PoolType,
    ROS_ALLOC_TAG Tag
    ) throw ();

void __cdecl operator delete[] ( void* Ptr ) throw ();

void* __cdecl operator new ( size_t, void* Ptr ) throw ();

void __cdecl operator delete ( void*, void* ) throw ();

void* __cdecl operator new[] ( size_t, void* Ptr ) throw ();

void __cdecl operator delete[] ( void*, void* ) throw ();

//
// class ROS_FINALLY
//
class ROS_FINALLY {
private:

    ROS_FINALLY () = delete;

    template < typename T_OP > class _FINALIZER {
        friend class ROS_FINALLY;
        T_OP op;
        __forceinline _FINALIZER ( T_OP Op ) throw () : op(Op) {}
    public:
        __forceinline ~_FINALIZER () { (void)this->op(); }
    }; // class _FINALIZER

    template < typename T_OP > class _FINALIZER_EX {
        friend class ROS_FINALLY;
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
}; // class ROS_FINALLY

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS RosOpenDevice (
    _In_ UNICODE_STRING* FileNamePtr,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    _Out_ FILE_OBJECT** FileObjectPPtr,
    ROS_ALLOC_TAG AllocTag = ROS_ALLOC_TAG::DEVICE
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS RosSendWriteSynchronously (
    _In_ FILE_OBJECT* FileObjectPtr,
    _In_reads_bytes_opt_(InputBufferLength) void* InputBufferPtr,
    ULONG InputBufferLength,
    _Out_ ULONG_PTR* InformationPtr
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS RosSendIoctlSynchronously (
    _In_ FILE_OBJECT* FileObjectPtr,
    ULONG IoControlCode,
    _In_reads_bytes_opt_(InputBufferLength) void* InputBufferPtr,
    ULONG InputBufferLength,
    _Out_writes_bytes_opt_(OutputBufferLength) void* OutputBufferPtr,
    ULONG OutputBufferLength,
    BOOLEAN InternalDeviceIoControl,
    _Out_ ULONG_PTR* InformationPtr
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DXGI_FORMAT DxgiFormatFromD3dDdiFormat (D3DDDIFORMAT Format);

_IRQL_requires_max_(PASSIVE_LEVEL)
D3DDDIFORMAT
TranslateDxgiFormat (DXGI_FORMAT dxgiFormat);

#if (defined(_X86_) || defined(_AMD64_))
__forceinline
VOID
WRITE_REGISTER_NOFENCE_ULONG (
    _In_ _Notliteral_ volatile ULONG *Register,
    _In_ ULONG Value
    )
{
    WRITE_REGISTER_ULONG(Register, Value);
}

__forceinline
ULONG
READ_REGISTER_NOFENCE_ULONG (
    _In_ _Notliteral_ volatile ULONG *Register
    )
{

    return READ_REGISTER_ULONG(Register);
}

__forceinline
VOID
READ_REGISTER_NOFENCE_BUFFER_ULONG (
    _In_reads_(Count) _Notliteral_ volatile ULONG *Register,
    _Out_writes_all_(Count) PULONG Buffer,
    _In_ ULONG Count
    )
{
    READ_REGISTER_BUFFER_ULONG(Register, Buffer, Count);
}

#endif // (defined(_X86_) || defined(_AMD64_))

#endif // _ROSKMDUTIL_H_

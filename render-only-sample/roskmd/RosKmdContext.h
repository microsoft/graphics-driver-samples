#pragma once

#include "RosKmd.h"

class RosKmDevice;

class RosKmContext
{
    //
    // RosKmdCreateContext acts as the constructor so we list it as a friend
    // RosKmdEscape also acts as an initializer
    //
    friend NTSTATUS RosKmdCreateContext(IN_CONST_HANDLE, INOUT_PDXGKARG_CREATECONTEXT);
    friend NTSTATUS RosKmdEscape(IN_CONST_HANDLE, IN_CONST_PDXGKARG_ESCAPE);

public:

    static NTSTATUS
        __stdcall
        DdiRender(
            IN_CONST_HANDLE         hContext,
            INOUT_PDXGKARG_RENDER   pRender);

    static NTSTATUS
        __stdcall
        DdiCreateContext(
            IN_CONST_HANDLE                 hDevice,
            INOUT_PDXGKARG_CREATECONTEXT    pCreateContext);

    static NTSTATUS
        __stdcall
        DdiDestroyContext(
            IN_CONST_HANDLE     hContext);

protected:
    RosKmDevice           *m_pDevice;
    UINT                    m_Node;
    HANDLE                  m_hRTContext;

    DXGK_CREATECONTEXTFLAGS m_Flags;

public:

    UINT
        GetNode()
    {
        return m_Node;
    }

    RosKmDevice *
        GetDevice()
    {
        return m_pDevice;
    }

    DXGK_CREATECONTEXTFLAGS
        GetFlags() const
    {
        return m_Flags;
    }

    static RosKmContext * Cast(IN_CONST_HANDLE hContext)
    {
        RosKmContext * rosKmContext = reinterpret_cast<RosKmContext *>(hContext);

        NT_ASSERT(rosKmContext->m_magic == RosKmContext::kMagic);

        return rosKmContext;
    }

    NTSTATUS
        RenderKm(
            INOUT_PDXGKARG_RENDER   pRender);

public: // PAGED

    _Check_return_
    _Function_class_DXGK_(DXGKDDI_PRESENT)
    _IRQL_requires_(PASSIVE_LEVEL)
    NTSTATUS Present (INOUT_PDXGKARG_PRESENT Args);

private:

    static const UINT32 kMagic = 'CTXT';
    UINT32 m_magic;

};

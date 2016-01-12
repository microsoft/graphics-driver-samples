#pragma once

#include "d3dumddi_.h"
#include "roscompilerdebug.h"
#include "DisasmBase.hpp"
#include "HLSLBinary.hpp"
#include "HLSLDisasm.hpp"
#if VC4
#include "..\roscommon\Vc4Qpu.h"
#include "Vc4Disasm.hpp"
#include "Vc4Emit.hpp"
#include "Vc4Shader.hpp"
#endif // VC4

// Vertex shader
//   0 - h/w vertex shader code.
//   1 - vertex shader uniform.
//   2 - h/w coordinate shader code.
//   3 - coordinate shader uniform.
// Pixel shader
//   0 - h/w pixel shader code.
//   1 - pixel shader uniform.
//
#define ROS_VERTEX_SHADER_STORAGE 0
#define ROS_VERTEX_SHADER_UNIFORM_STORAGE 1
#define ROS_COORDINATE_SHADER_STORAGE  2
#define ROS_COORDINATE_SHADER_UNIFORM_STORAGE  3

#define ROS_PIXEL_SHADER_STORAGE 0
#define ROS_PIXEL_SHADER_UNIFORM_STORAGE 1

class RosCompiler
{
public:

    RosCompiler(
        D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
        const UINT *pCode,
        const D3D11_1_DDI_BLEND_DESC* pBlendState,
        const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthState,
        const D3D11_1_DDI_RASTERIZER_DESC* pRasterState,
        UINT numInputSignatureEntries,
        const D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
        UINT numOutputSignatureEntries,
        const D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
        UINT numPatchConstantSignatureEntries,
        const D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries);
    ~RosCompiler();

    HRESULT Compile();

    HRESULT InitializeStorage()
    {
        HRESULT hr = E_NOTIMPL;
#if VC4
        uint8_t cStorage = 0;
        switch (m_ProgramType)
        {
        case D3D10_SB_VERTEX_SHADER:
            cStorage = 4;
            break;
        case D3D10_SB_PIXEL_SHADER:
            cStorage = 2;
            break;
        default:
            assert(false);
        }

        for (uint8_t i = 0; i < cStorage; i++)
        {
            hr = m_Storage[i].Initialize();
            if (FAILED(hr))
            {
                for (uint8_t j = 0; j < i; j++)
                {
                    m_Storage[j].Reset();
                }
                break;
            }
        }
#endif // VC4
        return hr;
    }

    HRESULT GetShaderCode(void *pDest, UINT* pCSOffset = NULL)
    {
        HRESULT hr = E_NOTIMPL;
#if VC4
        switch (m_ProgramType)
        {
        case D3D10_SB_VERTEX_SHADER:
            memcpy(pDest, 
                   m_Storage[ROS_VERTEX_SHADER_STORAGE].GetStorage(),
                   m_Storage[ROS_VERTEX_SHADER_STORAGE].GetUsedSize());
            memcpy((BYTE*)pDest + m_Storage[ROS_VERTEX_SHADER_STORAGE].GetUsedSize(),
                   m_Storage[ROS_COORDINATE_SHADER_STORAGE].GetStorage(),
                   m_Storage[ROS_COORDINATE_SHADER_STORAGE].GetUsedSize());
            assert(pCSOffset);
            *pCSOffset = m_Storage[ROS_VERTEX_SHADER_STORAGE].GetUsedSize();
            hr = S_OK;
            break;
        case D3D10_SB_PIXEL_SHADER:
            memcpy(pDest,
                m_Storage[ROS_PIXEL_SHADER_STORAGE].GetStorage(),
                m_Storage[ROS_PIXEL_SHADER_STORAGE].GetUsedSize());
            hr = S_OK;
            break;
        default:
            assert(false);
        }
#endif // VC4
        return hr;
    }

    UINT GetShaderCodeSize()
    {
        UINT CodeSize = 0;
#if VC4
        switch (m_ProgramType)
        {
        case D3D10_SB_VERTEX_SHADER:
            CodeSize = m_Storage[ROS_VERTEX_SHADER_STORAGE].GetUsedSize() +
                       m_Storage[ROS_COORDINATE_SHADER_STORAGE].GetUsedSize();
            break;
        case D3D10_SB_PIXEL_SHADER:
            CodeSize = m_Storage[ROS_PIXEL_SHADER_STORAGE].GetUsedSize();
            break;
        default:
            assert(false);
        }
#endif // VC4
        return CodeSize;
    }

    VC4_UNIFORM_FORMAT* GetShaderUniformFormat(UINT Type, UINT *pUniformFormatEntries)
    {
#if VC4
        assert((Type == ROS_VERTEX_SHADER_UNIFORM_STORAGE) ||
            (Type == ROS_COORDINATE_SHADER_UNIFORM_STORAGE) ||
            (Type == ROS_PIXEL_SHADER_UNIFORM_STORAGE));
        *pUniformFormatEntries = m_Storage[Type].GetUsedSize<VC4_UNIFORM_FORMAT>();
        return m_Storage[Type].GetStorage<VC4_UNIFORM_FORMAT>();
#endif // VC4

    }

    const D3D11_1_DDI_BLEND_DESC* GetBlendState()
    {
        return m_pBlendState;
    }

    const D3D10_DDI_DEPTH_STENCIL_DESC* GetDepthState()
    {
        return m_pDepthState;
    }

    const D3D11_1_DDI_RASTERIZER_DESC* GetRasterState()
    {
        return m_pRasterState;
    }

    UINT GetInputSignature(D3D11_1DDIARG_SIGNATURE_ENTRY ** ppInputSignatureEntries)
    {
        *ppInputSignatureEntries = const_cast<D3D11_1DDIARG_SIGNATURE_ENTRY*>(m_pInputSignatureEntries);
        return m_numInputSignatureEntries;
    }

    UINT GetOutputSignature(D3D11_1DDIARG_SIGNATURE_ENTRY ** ppOutputSignatureEntries)
    {
        *ppOutputSignatureEntries = const_cast<D3D11_1DDIARG_SIGNATURE_ENTRY*>(m_pOutputSignatureEntries);
        return m_numOutputSignatureEntries;
    }

private:

    void Disassemble_HLSL() 
    { 
        HLSLDisasm().Run(m_pCode); 
    }

    void Disassemble_Signatures()
    {
        HLSLDisasm().Run(TEXT("Input Signature Entries"), m_pInputSignatureEntries, m_numInputSignatureEntries);
        HLSLDisasm().Run(TEXT("Output Signature Entries"), m_pOutputSignatureEntries, m_numOutputSignatureEntries);
        HLSLDisasm().Run(TEXT("Patch Constant Signature Entries"), m_pPatchConstantSignatureEntries, m_numPatchConstantSignatureEntries);
    }

#if VC4
    void Disassemble_HW(Vc4ShaderStorage &Storage, TCHAR *pTitle)
    {
        Vc4Disasm().Run((const VC4_QPU_INSTRUCTION*)Storage.GetStorage(), Storage.GetUsedSize(), pTitle);
    }
#endif // VC4

private:

    //
    // HLSL code data.
    //
    D3D10_SB_TOKENIZED_PROGRAM_TYPE m_ProgramType;
    const UINT *m_pCode;

    //
    // State(s).
    //
    const D3D11_1_DDI_BLEND_DESC* m_pBlendState;
    const D3D10_DDI_DEPTH_STENCIL_DESC* m_pDepthState;
    const D3D11_1_DDI_RASTERIZER_DESC* m_pRasterState;

    //
    // I/O signature(s).
    //
    UINT m_numInputSignatureEntries;
    const D3D11_1DDIARG_SIGNATURE_ENTRY *m_pInputSignatureEntries;
    UINT m_numOutputSignatureEntries;
    const D3D11_1DDIARG_SIGNATURE_ENTRY *m_pOutputSignatureEntries;
    UINT m_numPatchConstantSignatureEntries;
    const D3D11_1DDIARG_SIGNATURE_ENTRY *m_pPatchConstantSignatureEntries;

#if VC4
    //
    // Hardware shader data.
    //
    Vc4ShaderStorage m_Storage[4];
#endif // VC4

};

RosCompiler* RosCompilerCreate(D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType,
                               const UINT *pCode,
                               const D3D11_1_DDI_BLEND_DESC* pBlendState,
                               const D3D10_DDI_DEPTH_STENCIL_DESC* pDepthState,
                               const D3D11_1_DDI_RASTERIZER_DESC* pRasterState,
                               UINT numInputSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries,
                               UINT numOutputSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries,
                               UINT numPatchConstantSignatureEntries,
                               const D3D11_1DDIARG_SIGNATURE_ENTRY *pPatchConstantSignatureEntries);

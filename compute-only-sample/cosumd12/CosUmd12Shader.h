#pragma once

#pragma once

#include "CosUmd12.h"

class CosUmd12Device;

class CosUmd12Shader
{
public:
    explicit CosUmd12Shader(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_SHADER_0026* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;

        char * storage = (char *)this + sizeof(*this);
        int storageSize = CalculateSize(pArgs) - sizeof(*this);

        size_t size = m_args.pShaderCode[1] * sizeof(UINT);
        ASSERT(storageSize >= size);

        UINT * dstCode = (UINT *)storage;
        storageSize -= size;
        storage += size;

        memcpy(dstCode, m_args.pShaderCode, size);
        m_args.pShaderCode = dstCode;

        // Only Compute Shader is supported, so IOSignatures.Tessellation is not used
        size = sizeof(D3D12DDIARG_STAGE_IO_SIGNATURES);
        ASSERT(storageSize >= size);

        D3D12DDIARG_STAGE_IO_SIGNATURES * dstSignature = (D3D12DDIARG_STAGE_IO_SIGNATURES *)storage;
        storageSize -= size;
        storage += size;

        *dstSignature = *m_args.IOSignatures.Standard;
        m_args.IOSignatures.Standard = dstSignature;

        if (dstSignature->NumInputSignatureEntries)
        {
            size = dstSignature->NumInputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);
            ASSERT(storageSize >= size);

            D3D12DDIARG_SIGNATURE_ENTRY_0012 * inputEntries = (D3D12DDIARG_SIGNATURE_ENTRY_0012 *)storage;
            storageSize -= size;
            storage += size;

            memcpy(inputEntries, m_args.IOSignatures.Standard->pInputSignature, size);
            dstSignature->pInputSignature = inputEntries;
        }

        if (dstSignature->NumOutputSignatureEntries)
        {
            size = dstSignature->NumOutputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);
            ASSERT(storageSize >= size);

            D3D12DDIARG_SIGNATURE_ENTRY_0012 * outputEntries = (D3D12DDIARG_SIGNATURE_ENTRY_0012 *)storage;
            storageSize -= size;
            storage += size;

            memcpy(outputEntries, m_args.IOSignatures.Standard->pOutputSignature, size);
            dstSignature->pOutputSignature = outputEntries;
        }

        ASSERT(storageSize == 0);
    }

    ~CosUmd12Shader()
    {
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_SHADER_0026 * pArgs)
    {
        int size = sizeof(CosUmd12Shader);

        size += pArgs->pShaderCode[1] * sizeof(UINT);

        // Only Compute Shader is supported, so IOSignatures.Tessellation is not used
        size += sizeof(D3D12DDIARG_STAGE_IO_SIGNATURES);

        size += pArgs->IOSignatures.Standard->NumInputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);
        size += pArgs->IOSignatures.Standard->NumOutputSignatureEntries * sizeof(D3D12DDIARG_SIGNATURE_ENTRY_0012);

        return size;
    }

    static CosUmd12Shader* CastFrom(D3D12DDI_HSHADER);
    D3D12DDI_HSHADER CastTo() const;

private:

    friend class CosUmd12CommandList;

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_SHADER_0026 m_args;

};

inline CosUmd12Shader* CosUmd12Shader::CastFrom(D3D12DDI_HSHADER hRootSignature)
{
    return static_cast< CosUmd12Shader* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HSHADER CosUmd12Shader::CastTo() const
{
    // TODO: Why does MAKE_D3D12DD_HSHADER not exist?
    return MAKE_D3D10DDI_HSHADER(const_cast< CosUmd12Shader* >(this));
}


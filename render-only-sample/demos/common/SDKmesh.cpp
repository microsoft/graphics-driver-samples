//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//--------------------------------------------------------------------------------------
// File: SDKMesh.cpp
//
// The SDK Mesh format (.sdkmesh) is not a recommended file format for games.  
// It was designed to meet the specific needs of the SDK samples.  Any real-world 
// applications should avoid this file format in favor of a destination format that 
// meets the specific needs of the application.
//--------------------------------------------------------------------------------------

#include <SDKDDKVer.h>
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <directxmath.h>
#include <intrin.h>

#include "SDKMesh.h"

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::CreateVertexBuffer( ID3D11Device* pd3dDevice, SDKMESH_VERTEX_BUFFER_HEADER* pHeader, void* pVertices )
{
    HRESULT hr = S_OK;
    pHeader->DataOffset = 0;
    //Vertex Buffer
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = ( UINT )( pHeader->SizeBytes );
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags =  D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pVertices;
    hr = pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &pHeader->pVB11 );

    return hr;
}

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::CreateIndexBuffer( ID3D11Device* pd3dDevice, SDKMESH_INDEX_BUFFER_HEADER* pHeader, void* pIndices)
{
    HRESULT hr = S_OK;
    pHeader->DataOffset = 0;

    //Index Buffer
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = ( UINT )( pHeader->SizeBytes );
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    void *pMyIndices = pIndices;

#if 1 // Hideyuki 9_1 only support 16bits index, so convert it.
    if (pHeader->IndexType == IT_32BIT)
    {
        DWORD *pOrgIndices = (DWORD *) pIndices;
        bufferDesc.ByteWidth = (UINT)(pHeader->NumIndices * sizeof(USHORT));
        pMyIndices = (USHORT *) malloc(bufferDesc.ByteWidth);
        for (DWORD i = 0; i < pHeader->NumIndices; i++)
        {
            if (pOrgIndices[i] & 0xFFFF0000)
            {
                __debugbreak();
            }
            ((USHORT*)pMyIndices)[i] = (USHORT)(pOrgIndices[i] & 0x0000FFFF);
        }
    }
#endif

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pMyIndices;

    hr = pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &pHeader->pIB11 );

    if (pMyIndices != pIndices)
    {
        free(pMyIndices);
    }

    return hr;
}

HRESULT CDXUTSDKMesh::CreateFromMemory( ID3D11Device* pDev11,
                                        BYTE* pData,
                                        UINT DataBytes,
                                        bool bCreateAdjacencyIndices,
                                        bool bCopyStatic)
{
    HRESULT hr = E_FAIL;
    
    m_pDev11 = pDev11;
    m_bCopyStatic = bCopyStatic;

    // Set outstanding resources to zero
    m_NumOutstandingResources = 0;

    if( bCopyStatic )
    {
        SDKMESH_HEADER* pHeader = ( SDKMESH_HEADER* )pData;

        SIZE_T StaticSize = ( SIZE_T )( pHeader->HeaderSize + pHeader->NonBufferDataSize );
        m_pHeapData = new BYTE[ StaticSize ];
        if( !m_pHeapData )
            return hr;

        m_pStaticMeshData = m_pHeapData;

        CopyMemory( m_pStaticMeshData, pData, StaticSize );
    }
    else
    {
        m_pHeapData = pData;
        m_pStaticMeshData = pData;
    }

    // Pointer fixup
    m_pMeshHeader = ( SDKMESH_HEADER* )m_pStaticMeshData;

    m_pVertexBufferArray = ( SDKMESH_VERTEX_BUFFER_HEADER* )( m_pStaticMeshData + m_pMeshHeader->VertexStreamHeadersOffset ); 

    m_pIndexBufferArray = ( SDKMESH_INDEX_BUFFER_HEADER* )( m_pStaticMeshData + m_pMeshHeader->IndexStreamHeadersOffset );
    
    m_pMeshArray = ( SDKMESH_MESH* )( m_pStaticMeshData + m_pMeshHeader->MeshDataOffset );

    m_pSubsetArray = ( SDKMESH_SUBSET* )( m_pStaticMeshData + m_pMeshHeader->SubsetDataOffset );

    m_pFrameArray = ( SDKMESH_FRAME* )( m_pStaticMeshData + m_pMeshHeader->FrameDataOffset );

    m_pMaterialArray = ( SDKMESH_MATERIAL* )( m_pStaticMeshData + m_pMeshHeader->MaterialDataOffset );

    // Setup subsets
    for( UINT i = 0; i < m_pMeshHeader->NumMeshes; i++ )
    {
        m_pMeshArray[i].pSubsets = ( UINT* )( m_pStaticMeshData + m_pMeshArray[i].SubsetOffset );
        m_pMeshArray[i].pFrameInfluences = ( UINT* )( m_pStaticMeshData + m_pMeshArray[i].FrameInfluenceOffset );
    }

    // error condition
    if( m_pMeshHeader->Version != SDKMESH_FILE_VERSION )
    {
        hr = E_NOINTERFACE;
        goto Error;
    }

    // Setup buffer data pointer
    BYTE* pBufferData = pData + m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Get the start of the buffer data
    UINT64 BufferDataStart = m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Create VBs
    m_ppVertices = new BYTE*[m_pMeshHeader->NumVertexBuffers];
    for( UINT i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
    {
        BYTE* pVertices = NULL;
        pVertices = ( BYTE* )( pBufferData + ( m_pVertexBufferArray[i].DataOffset - BufferDataStart ) );

        if( pDev11 )
            CreateVertexBuffer( pDev11, &m_pVertexBufferArray[i], pVertices );

        m_ppVertices[i] = pVertices;
    }

    // Create IBs
    m_ppIndices = new BYTE*[m_pMeshHeader->NumIndexBuffers];
    for( UINT i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
    {
        BYTE* pIndices = NULL;
        pIndices = ( BYTE* )( pBufferData + ( m_pIndexBufferArray[i].DataOffset - BufferDataStart ) );

        if( pDev11 )
            CreateIndexBuffer( pDev11, &m_pIndexBufferArray[i], pIndices );

        m_ppIndices[i] = pIndices;
    }

    // Load Materials
    // Not supported for now
    
    // Create a place to store our bind pose frame matrices
    m_pBindPoseFrameMatrices = new MATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pBindPoseFrameMatrices )
        goto Error;

    // Create a place to store our transformed frame matrices
    m_pTransformedFrameMatrices = new MATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pTransformedFrameMatrices )
        goto Error;
    m_pWorldPoseFrameMatrices = new MATRIX[ m_pMeshHeader->NumFrames ];
    if( !m_pWorldPoseFrameMatrices )
        goto Error;

    hr = S_OK;

Error:
    return hr;
}

//--------------------------------------------------------------------------------------
CDXUTSDKMesh::CDXUTSDKMesh() : m_NumOutstandingResources( 0 ),
                               m_bLoading( false ),
                               m_hFile( 0 ),
                               m_hFileMappingObject( 0 ),
                               m_pMeshHeader( NULL ),
                               m_pStaticMeshData( NULL ),
                               m_pHeapData( NULL ),
                               m_pAdjacencyIndexBufferArray( NULL ),
                               m_pAnimationData( NULL ),
                               m_pAnimationHeader( NULL ),
                               m_ppVertices( NULL ),
                               m_ppIndices( NULL ),
                               m_pBindPoseFrameMatrices( NULL ),
                               m_pTransformedFrameMatrices( NULL ),
                               m_pWorldPoseFrameMatrices( NULL ),
                               m_pDev11( NULL ),
                               m_bCopyStatic( false )
{
}


//--------------------------------------------------------------------------------------
CDXUTSDKMesh::~CDXUTSDKMesh()
{
    Destroy();
}

//--------------------------------------------------------------------------------------
HRESULT CDXUTSDKMesh::Create( ID3D11Device* pDev11, PBYTE pMesh, ULONG MeshSize, bool bCreateAdjacencyIndices )
{
    return CreateFromMemory( pDev11, pMesh, MeshSize, bCreateAdjacencyIndices, true );
}

//--------------------------------------------------------------------------------------
void CDXUTSDKMesh::Destroy()
{
    if( m_pStaticMeshData )
    {
        if( m_pMaterialArray )
        {
            for( UINT64 m = 0; m < m_pMeshHeader->NumMaterials; m++ )
            {
                if( m_pDev11 )
                {
                    //ID3D11Resource* pRes = NULL;
                    if( m_pMaterialArray[m].pDiffuseRV11 && !IsErrorResource( m_pMaterialArray[m].pDiffuseRV11 ) )
                    {
                        //m_pMaterialArray[m].pDiffuseRV11->GetResource( &pRes );
                        //SAFE_RELEASE( pRes );

                        m_pMaterialArray[m].pDiffuseRV11->Release();
                    }
                    if( m_pMaterialArray[m].pNormalRV11 && !IsErrorResource( m_pMaterialArray[m].pNormalRV11 ) )
                    {
                        //m_pMaterialArray[m].pNormalRV11->GetResource( &pRes );
                        //SAFE_RELEASE( pRes );

                        m_pMaterialArray[m].pNormalRV11->Release();
                    }
                    if( m_pMaterialArray[m].pSpecularRV11 && !IsErrorResource( m_pMaterialArray[m].pSpecularRV11 ) )
                    {
                        //m_pMaterialArray[m].pSpecularRV11->GetResource( &pRes );
                        //SAFERELEASE( pRes );

                        m_pMaterialArray[m].pSpecularRV11->Release();
                    }
                }
            }
        }
    }

    if( m_pVertexBufferArray )
    {
        for( UINT64 i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
        {
            if (m_pVertexBufferArray[i].pVB11)
            {
                m_pVertexBufferArray[i].pVB11->Release();
            }
        }
    }

    if( m_pIndexBufferArray )
    {
        for( UINT64 i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
        {
            if (m_pIndexBufferArray[i].pIB11)
            {
                m_pIndexBufferArray[i].pIB11->Release();
            }
        }
    }
    
    if( m_pAdjacencyIndexBufferArray )
    {
        for( UINT64 i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
        {
            if (m_pAdjacencyIndexBufferArray[i].pIB11)
            {
                m_pAdjacencyIndexBufferArray[i].pIB11->Release();
            }
        }
    }
    
    if (m_pAdjacencyIndexBufferArray)
        delete [] m_pAdjacencyIndexBufferArray;

    if (m_bCopyStatic && m_pHeapData)
        delete [] m_pHeapData;
    m_pStaticMeshData = NULL;
    
    if (m_pAnimationData)
        delete [] m_pAnimationData;
    if (m_pBindPoseFrameMatrices)
        delete [] m_pBindPoseFrameMatrices;
    if (m_pTransformedFrameMatrices)
        delete [] m_pTransformedFrameMatrices;
    if (m_pWorldPoseFrameMatrices)
        delete [] m_pWorldPoseFrameMatrices;

    if (m_ppVertices)
        delete [] m_ppVertices;
    if (m_ppIndices)
        delete [] m_ppIndices;

    m_pMeshHeader = NULL;
    m_pVertexBufferArray = NULL;
    m_pIndexBufferArray = NULL;
    m_pMeshArray = NULL;
    m_pSubsetArray = NULL;
    m_pFrameArray = NULL;
    m_pMaterialArray = NULL;

    m_pAnimationHeader = NULL;
    m_pAnimationFrameData = NULL;
}

//--------------------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY CDXUTSDKMesh::GetPrimitiveType11( SDKMESH_PRIMITIVE_TYPE PrimType )
{
    D3D11_PRIMITIVE_TOPOLOGY retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    switch( PrimType )
    {
        case PT_TRIANGLE_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case PT_TRIANGLE_STRIP:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        case PT_LINE_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case PT_LINE_STRIP:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case PT_POINT_LIST:
            retType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case PT_TRIANGLE_LIST_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            break;
        case PT_TRIANGLE_STRIP_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            break;
        case PT_LINE_LIST_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            break;
        case PT_LINE_STRIP_ADJ:
            retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            break;
    };

    return retType;
}

//--------------------------------------------------------------------------------------
DXGI_FORMAT CDXUTSDKMesh::GetIBFormat11( UINT iMesh )
{
#if 1 // Hideyuki 9_1 only support R16.
    return DXGI_FORMAT_R16_UINT;
#else
    switch( m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].IndexType )
    {
        case IT_16BIT:
            return DXGI_FORMAT_R16_UINT;
        case IT_32BIT:
            return DXGI_FORMAT_R32_UINT;
    };
    return DXGI_FORMAT_R16_UINT;
#endif
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetVB11( UINT iMesh, UINT iVB )
{
    if (m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].pVB11)
    {
        m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].pVB11->AddRef();
    }
    return m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].pVB11;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetIB11( UINT iMesh )
{
    if (m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11)
    {
        m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11->AddRef();
    }

    return m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11;
}

SDKMESH_INDEX_TYPE CDXUTSDKMesh::GetIndexType( UINT iMesh ) 
{
    return ( SDKMESH_INDEX_TYPE ) m_pIndexBufferArray[m_pMeshArray[ iMesh ].IndexBuffer].IndexType;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetAdjIB11( UINT iMesh )
{
    if (m_pAdjacencyIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11)
    {
        m_pAdjacencyIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11->AddRef();
    }

    return m_pAdjacencyIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].pIB11;
}

//--------------------------------------------------------------------------------------
char* CDXUTSDKMesh::GetMeshPathA()
{
    return m_strPath;
}

//--------------------------------------------------------------------------------------
WCHAR* CDXUTSDKMesh::GetMeshPathW()
{
    return m_strPathW;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumMeshes()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMeshes;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumMaterials()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMaterials;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumVBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumVertexBuffers;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumIBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumIndexBuffers;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetVB11At( UINT iVB )
{
    if (m_pVertexBufferArray[ iVB ].pVB11)
    {
        m_pVertexBufferArray[ iVB ].pVB11->AddRef();
    }
    return m_pVertexBufferArray[ iVB ].pVB11;
}

//--------------------------------------------------------------------------------------
ID3D11Buffer* CDXUTSDKMesh::GetIB11At( UINT iIB )
{
    if (m_pIndexBufferArray[ iIB ].pIB11)
    {
        m_pIndexBufferArray[ iIB ].pIB11->AddRef();
    }
    return m_pIndexBufferArray[ iIB ].pIB11;
}

//--------------------------------------------------------------------------------------
BYTE* CDXUTSDKMesh::GetRawVerticesAt( UINT iVB )
{
    return m_ppVertices[iVB];
}

//--------------------------------------------------------------------------------------
BYTE* CDXUTSDKMesh::GetRawIndicesAt( UINT iIB )
{
    return m_ppIndices[iIB];
}

//--------------------------------------------------------------------------------------
SDKMESH_MATERIAL* CDXUTSDKMesh::GetMaterial( UINT iMaterial )
{
    return &m_pMaterialArray[ iMaterial ];
}

//--------------------------------------------------------------------------------------
SDKMESH_MESH* CDXUTSDKMesh::GetMesh( UINT iMesh )
{
    return &m_pMeshArray[ iMesh ];
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumSubsets( UINT iMesh )
{
    return m_pMeshArray[ iMesh ].NumSubsets;
}

//--------------------------------------------------------------------------------------
SDKMESH_SUBSET* CDXUTSDKMesh::GetSubset( UINT iMesh, UINT iSubset )
{
    return &m_pSubsetArray[ m_pMeshArray[ iMesh ].pSubsets[iSubset] ];
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetVertexStride( UINT iMesh, UINT iVB )
{
    return ( UINT )m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].StrideBytes;
}

//--------------------------------------------------------------------------------------
UINT CDXUTSDKMesh::GetNumFrames()
{
    return m_pMeshHeader->NumFrames;
}

//--------------------------------------------------------------------------------------
SDKMESH_FRAME* CDXUTSDKMesh::GetFrame( UINT iFrame )
{
    return &m_pFrameArray[ iFrame ];
}

//--------------------------------------------------------------------------------------
SDKMESH_FRAME* CDXUTSDKMesh::FindFrame( char* pszName )
{
    for( UINT i = 0; i < m_pMeshHeader->NumFrames; i++ )
    {
        if( _stricmp( m_pFrameArray[i].Name, pszName ) == 0 )
        {
            return &m_pFrameArray[i];
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------
UINT64 CDXUTSDKMesh::GetNumVertices( UINT iMesh, UINT iVB )
{
    return m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].NumVertices;
}

//--------------------------------------------------------------------------------------
UINT64 CDXUTSDKMesh::GetNumIndices( UINT iMesh )
{
    return m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].NumIndices;
}

//--------------------------------------------------------------------------------------
VECTOR3 CDXUTSDKMesh::GetMeshBBoxCenter( UINT iMesh )
{
    return m_pMeshArray[iMesh].BoundingBoxCenter;
}

//--------------------------------------------------------------------------------------
VECTOR3 CDXUTSDKMesh::GetMeshBBoxExtents( UINT iMesh )
{
    return m_pMeshArray[iMesh].BoundingBoxExtents;
}


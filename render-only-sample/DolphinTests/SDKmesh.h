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
// File: SDKMesh.h
//
// Disclaimer:
//   The SDK Mesh format (.sdkmesh) is not a recommended file format for shipping titles.  
//   It was designed to meet the specific needs of the SDK samples.  Any real-world 
//   applications should avoid this file format in favor of a destination format that 
//   meets the specific needs of the application.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef _SDKMESH_
#define _SDKMESH_

//--------------------------------------------------------------------------------------
// Hard Defines for the various structures
//--------------------------------------------------------------------------------------
#define SDKMESH_FILE_VERSION 101
#define MAX_VERTEX_ELEMENTS 32
#define MAX_VERTEX_STREAMS 16
#define MAX_FRAME_NAME 100
#define MAX_MESH_NAME 100
#define MAX_SUBSET_NAME 100
#define MAX_MATERIAL_NAME 100
#define MAX_TEXTURE_NAME MAX_PATH
#define MAX_MATERIAL_PATH MAX_PATH
#define INVALID_FRAME ((UINT)-1)
#define INVALID_MESH ((UINT)-1)
#define INVALID_MATERIAL ((UINT)-1)
#define INVALID_SUBSET ((UINT)-1)
#define INVALID_ANIMATION_DATA ((UINT)-1)
#define INVALID_SAMPLER_SLOT ((UINT)-1)
#define ERROR_RESOURCE_VALUE 1

template<typename TYPE> BOOL IsErrorResource( TYPE data )
{
    if( ( TYPE )ERROR_RESOURCE_VALUE == data )
        return TRUE;
    return FALSE;
}
//--------------------------------------------------------------------------------------
// Enumerated Types.  These will have mirrors in both D3D9 and D3D11
//--------------------------------------------------------------------------------------
enum SDKMESH_PRIMITIVE_TYPE
{
    PT_TRIANGLE_LIST = 0,
    PT_TRIANGLE_STRIP,
    PT_LINE_LIST,
    PT_LINE_STRIP,
    PT_POINT_LIST,
    PT_TRIANGLE_LIST_ADJ,
    PT_TRIANGLE_STRIP_ADJ,
    PT_LINE_LIST_ADJ,
    PT_LINE_STRIP_ADJ,
    PT_QUAD_PATCH_LIST,
    PT_TRIANGLE_PATCH_LIST,
};

enum SDKMESH_INDEX_TYPE
{
    IT_16BIT = 0,
    IT_32BIT,
};

enum FRAME_TRANSFORM_TYPE
{
    FTT_RELATIVE = 0,
    FTT_ABSOLUTE,       //This is not currently used but is here to support absolute transformations in the future
};

//--------------------------------------------------------------------------------------
// Structures.  Unions with pointers are forced to 64bit.
//--------------------------------------------------------------------------------------

typedef struct _VECTOR3 {
    float x;
    float y;
    float z;
} VECTOR3;

typedef struct _VECTOR4 : public VECTOR3{
    float w;
} VECTOR4;

typedef struct MATRIX 
{
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        }_m;
        float m[4][4];
    };
} MATRIX, *LPMATRIX;

struct SDKMESH_HEADER
{
    //Basic Info and sizes
    UINT Version;
    BYTE IsBigEndian;
    UINT64 HeaderSize;
    UINT64 NonBufferDataSize;
    UINT64 BufferDataSize;

    //Stats
    UINT NumVertexBuffers;
    UINT NumIndexBuffers;
    UINT NumMeshes;
    UINT NumTotalSubsets;
    UINT NumFrames;
    UINT NumMaterials;

    //Offsets to Data
    UINT64 VertexStreamHeadersOffset;
    UINT64 IndexStreamHeadersOffset;
    UINT64 MeshDataOffset;
    UINT64 SubsetDataOffset;
    UINT64 FrameDataOffset;
    UINT64 MaterialDataOffset;
};

// This is just a D3DVERTEXELEMENT9, but we don't
// actually want the dependency, so we redefine it here.
typedef struct _SDKMESHVERTEXELEMENT9
{
    WORD    Stream;     // Stream index
    WORD    Offset;     // Offset in the stream in bytes
    BYTE    Type;       // Data type, e.g. D3DDECLTYPE_FLOAT3
    BYTE    Method;     // Processing method, e.g. D3DDECLMETHOD_DEFAULT
    BYTE    Usage;      // Semantics, e.g. D3DDECLUSAGE_POSITION
    BYTE    UsageIndex; // Semantic index
} SDKMESHVERTEXELEMENT9, *LPSDKMESHVERTEXELEMENT9;

struct SDKMESH_VERTEX_BUFFER_HEADER
{
    UINT64 NumVertices;
    UINT64 SizeBytes;
    UINT64 StrideBytes;
    SDKMESHVERTEXELEMENT9 Decl[MAX_VERTEX_ELEMENTS];
    union
    {
        UINT64 DataOffset;              //(This also forces the union to 64bits)
        ID3D11Buffer* pVB11;
    };
};

struct SDKMESH_INDEX_BUFFER_HEADER
{
    UINT64 NumIndices;
    UINT64 SizeBytes;
    UINT IndexType;

    union
    {
        UINT64 DataOffset;              //(This also forces the union to 64bits)
        ID3D11Buffer* pIB11;
    };
};

struct SDKMESH_MESH
{
    char Name[MAX_MESH_NAME];
    BYTE NumVertexBuffers;
    UINT VertexBuffers[MAX_VERTEX_STREAMS];
    UINT IndexBuffer;
    UINT NumSubsets;
    UINT NumFrameInfluences; //aka bones

    VECTOR3 BoundingBoxCenter;
    VECTOR3 BoundingBoxExtents;

    union
    {
        UINT64 SubsetOffset;    //Offset to list of subsets (This also forces the union to 64bits)
        UINT* pSubsets;     //Pointer to list of subsets
    };
    union
    {
        UINT64 FrameInfluenceOffset;  //Offset to list of frame influences (This also forces the union to 64bits)
        UINT* pFrameInfluences;      //Pointer to list of frame influences
    };
};

struct SDKMESH_SUBSET
{
    char Name[MAX_SUBSET_NAME];
    UINT MaterialID;
    UINT PrimitiveType;
    UINT64 IndexStart;
    UINT64 IndexCount;
    UINT64 VertexStart;
    UINT64 VertexCount;
};

struct SDKMESH_FRAME
{
    char Name[MAX_FRAME_NAME];
    UINT Mesh;
    UINT ParentFrame;
    UINT ChildFrame;
    UINT SiblingFrame;
    MATRIX Matrix;
    UINT AnimationDataIndex;        //Used to index which set of keyframes transforms this frame
};

struct SDKMESH_MATERIAL
{
    char    Name[MAX_MATERIAL_NAME];

    // Use MaterialInstancePath
    char    MaterialInstancePath[MAX_MATERIAL_PATH];

    // Or fall back to d3d8-type materials
    char    DiffuseTexture[MAX_TEXTURE_NAME];
    char    NormalTexture[MAX_TEXTURE_NAME];
    char    SpecularTexture[MAX_TEXTURE_NAME];

    VECTOR4 Diffuse;
    VECTOR4 Ambient;
    VECTOR4 Specular;
    VECTOR4 Emissive;
    FLOAT Power;

    union
    {
        UINT64 Force64_1;           //Force the union to 64bits
        ID3D11Texture2D* pDiffuseTexture11;
    };
    union
    {
        UINT64 Force64_2;           //Force the union to 64bits
        ID3D11Texture2D* pNormalTexture11;
    };
    union
    {
        UINT64 Force64_3;           //Force the union to 64bits
        ID3D11Texture2D* pSpecularTexture11;
    };

    union
    {
        UINT64 Force64_4;           //Force the union to 64bits
        ID3D11ShaderResourceView* pDiffuseRV11;
    };
    union
    {
        UINT64 Force64_5;           //Force the union to 64bits
        ID3D11ShaderResourceView* pNormalRV11;
    };
    union
    {
        UINT64 Force64_6;           //Force the union to 64bits
        ID3D11ShaderResourceView* pSpecularRV11;
    };
};

struct SDKANIMATION_FILE_HEADER
{
    UINT Version;
    BYTE IsBigEndian;
    UINT FrameTransformType;
    UINT NumFrames;
    UINT NumAnimationKeys;
    UINT AnimationFPS;
    UINT64 AnimationDataSize;
    UINT64 AnimationDataOffset;
};

struct SDKANIMATION_DATA
{
    VECTOR3 Translation;
    VECTOR4 Orientation;
    VECTOR3 Scaling;
};

struct SDKANIMATION_FRAME_DATA
{
    char FrameName[MAX_FRAME_NAME];
    union
    {
        UINT64 DataOffset;
        SDKANIMATION_DATA* pAnimationData;
    };
};

//--------------------------------------------------------------------------------------
// CDXUTSDKMesh class.  This class reads the sdkmesh file format for use by the samples
//--------------------------------------------------------------------------------------
class CDXUTSDKMesh
{
private:
    UINT m_NumOutstandingResources;
    bool m_bLoading;
    HANDLE m_hFile;
    HANDLE m_hFileMappingObject;
    ID3D11Device* m_pDev11;
    ID3D11DeviceContext* m_pDevContext11;
    bool m_bCopyStatic;

protected:
    //These are the pointers to the two chunks of data loaded in from the mesh file
    BYTE* m_pStaticMeshData;
    BYTE* m_pHeapData;
    BYTE* m_pAnimationData;
    BYTE** m_ppVertices;
    BYTE** m_ppIndices;

    //Keep track of the path
    WCHAR                           m_strPathW[MAX_PATH];
    char                            m_strPath[MAX_PATH];

    //General mesh info
    SDKMESH_HEADER* m_pMeshHeader;
    SDKMESH_VERTEX_BUFFER_HEADER* m_pVertexBufferArray;
    SDKMESH_INDEX_BUFFER_HEADER* m_pIndexBufferArray;
    SDKMESH_MESH* m_pMeshArray;
    SDKMESH_SUBSET* m_pSubsetArray;
    SDKMESH_FRAME* m_pFrameArray;
    SDKMESH_MATERIAL* m_pMaterialArray;

    // Adjacency information (not part of the m_pStaticMeshData, so it must be created and destroyed separately )
    SDKMESH_INDEX_BUFFER_HEADER* m_pAdjacencyIndexBufferArray;

    //Animation (TODO: Add ability to load/track multiple animation sets)
    SDKANIMATION_FILE_HEADER* m_pAnimationHeader;
    SDKANIMATION_FRAME_DATA* m_pAnimationFrameData;
    MATRIX* m_pBindPoseFrameMatrices;
    MATRIX* m_pTransformedFrameMatrices;
    MATRIX* m_pWorldPoseFrameMatrices;

protected:

    HRESULT                         CreateVertexBuffer( ID3D11Device* pd3dDevice,
                                                        SDKMESH_VERTEX_BUFFER_HEADER* pHeader, void* pVertices );

    HRESULT                         CreateIndexBuffer( ID3D11Device* pd3dDevice, SDKMESH_INDEX_BUFFER_HEADER* pHeader,
                                                       void* pIndices );

    virtual HRESULT                 CreateFromMemory( ID3D11Device* pDev11,
                                                      BYTE* pData,
                                                      UINT DataBytes,
                                                      bool bCreateAdjacencyIndices,
                                                      bool bCopyStatic );

public:
                                    CDXUTSDKMesh();
    virtual                         ~CDXUTSDKMesh();
    virtual void                    Destroy();

    virtual HRESULT                 Create( ID3D11Device* pDev11, PBYTE pMesh, ULONG MeshSize, bool bCreateAdjacencyIndices=false );
        
    //Helpers (D3D11 specific)
    static D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveType11( SDKMESH_PRIMITIVE_TYPE PrimType );
    DXGI_FORMAT                     GetIBFormat11( UINT iMesh );
    ID3D11Buffer* GetVB11( UINT iMesh, UINT iVB );
    ID3D11Buffer* GetIB11( UINT iMesh );
    SDKMESH_INDEX_TYPE GetIndexType( UINT iMesh ); 

    ID3D11Buffer* GetAdjIB11( UINT iMesh );

    //Helpers (general)
    char* GetMeshPathA();
    WCHAR* GetMeshPathW();
    UINT                            GetNumMeshes();
    UINT                            GetNumMaterials();
    UINT                            GetNumVBs();
    UINT                            GetNumIBs();

    ID3D11Buffer* GetVB11At( UINT iVB );
    ID3D11Buffer* GetIB11At( UINT iIB );

    BYTE* GetRawVerticesAt( UINT iVB );
    BYTE* GetRawIndicesAt( UINT iIB );
    SDKMESH_MATERIAL* GetMaterial( UINT iMaterial );
    SDKMESH_MESH* GetMesh( UINT iMesh );
    UINT                            GetNumSubsets( UINT iMesh );
    SDKMESH_SUBSET* GetSubset( UINT iMesh, UINT iSubset );
    UINT                            GetVertexStride( UINT iMesh, UINT iVB );
    UINT                            GetNumFrames();
    SDKMESH_FRAME*                  GetFrame( UINT iFrame );
    SDKMESH_FRAME*                  FindFrame( _In_z_ char* pszName );
    UINT64                          GetNumVertices( UINT iMesh, UINT iVB );
    UINT64                          GetNumIndices( UINT iMesh );
    VECTOR3                         GetMeshBBoxCenter( UINT iMesh );
    VECTOR3                         GetMeshBBoxExtents( UINT iMesh );
    UINT                            GetOutstandingResources();
    UINT                            GetOutstandingBufferResources();
    bool                            CheckLoadDone();
    bool                            IsLoaded();
    bool                            IsLoading();
    void                            SetLoading( bool bLoading );
    BOOL                            HadLoadingError();
};

#endif

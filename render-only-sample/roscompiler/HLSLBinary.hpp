#pragma once

#define IFC(expr) {hr = expr; if (FAILED(hr)){goto Cleanup;}}

#pragma warning(disable : 4706)
#define IFCOOM(expr) {if(!expr){IFC(E_OUTOFMEMORY);}}

#define RRETURN(hr) {if(FAILED(hr)) __debugbreak(); return hr;}

typedef UINT CShaderToken;

const UINT MAX_INSTRUCTION_LENGTH = 128;
const UINT D3D11_SB_MAX_INSTRUCTION_OPERANDS = 8;
const UINT D3D11_SB_MAX_FP_ENTRIES = 253;

typedef enum D3D11_SB_OPCODE_CLASS
{
    D3D11_SB_FLOAT_OP,
    D3D11_SB_INT_OP,
    D3D11_SB_UINT_OP,
    D3D11_SB_BIT_OP,
    D3D11_SB_FLOW_OP,
    D3D11_SB_TEX_OP,
    D3D11_SB_DCL_OP,
} D3D11_SB_OPCODE_CLASS;

struct CInstructionInfo
{
    CInstructionInfo() {}
    ~CInstructionInfo()    {}

    void Set (BYTE NumDstOperands, BYTE NumSrcOperands, TCHAR* Name, D3D11_SB_OPCODE_CLASS OpClass)
    {
        m_NumSrcOperands = NumSrcOperands;
        m_NumDstOperands = NumDstOperands;
        m_Name = Name;
        m_OpClass = OpClass;
    }
    TCHAR*          m_Name;
    BYTE            m_NumSrcOperands;
    BYTE            m_NumDstOperands;
    D3D11_SB_OPCODE_CLASS m_OpClass;
};

extern CInstructionInfo g_InstructionInfo[D3D10_SB_NUM_OPCODES];

UINT GetNumInstructionOperands(D3D10_SB_OPCODE_TYPE OpCode);
UINT GetNumInstructionSrcOperands(D3D10_SB_OPCODE_TYPE OpCode);
UINT GetNumInstructionDstOperands(D3D10_SB_OPCODE_TYPE OpCode);
D3D11_SB_OPCODE_CLASS GetOpcodeClass(D3D10_SB_OPCODE_TYPE OpCode);
TCHAR* GetOpcodeString(D3D10_SB_OPCODE_TYPE OpCode);
void InitInstructionInfo();

//*****************************************************************************
//
// class COperandIndex
//
// Represents a dimension index of an operand
//
//*****************************************************************************

class COperandIndex
{
public:
    COperandIndex() {}
    // Value for the immediate index type
    union
    {
        UINT        m_RegIndex;
        UINT        m_RegIndexA[2];
        INT64       m_RegIndex64;
    };
    // Data for the relative index type
    D3D10_SB_OPERAND_TYPE    m_RelRegType;
    D3D10_SB_4_COMPONENT_NAME m_ComponentName;
    D3D10_SB_OPERAND_INDEX_DIMENSION         m_IndexDimension;
    // First index of the relative register
    union
    {
        UINT        m_RelIndex;
        UINT        m_RelIndexA[2];
        INT64       m_RelIndex64;
    };
    // Second index of the relative register
    union
    {
        UINT        m_RelIndex1;
        UINT        m_RelIndexA1[2];
        INT64       m_RelIndex641;
    };
};

//*****************************************************************************
//
// class COperandBase
//
// A base class for shader instruction operands
//
//*****************************************************************************

class COperandBase
{
public:
    COperandBase() {Clear();}
    COperandBase(const COperandBase & Op) { memcpy(this, &Op, sizeof(*this)); }
    D3D10_SB_OPERAND_TYPE OperandType() const {return m_Type;}
    const COperandIndex* OperandIndex(UINT Index) const {return &m_Index[Index];}
    D3D10_SB_OPERAND_INDEX_REPRESENTATION OperandIndexType(UINT Index) const {return m_IndexType[Index];}
    D3D10_SB_OPERAND_INDEX_DIMENSION OperandIndexDimension() const {return m_IndexDimension;}
    D3D10_SB_OPERAND_NUM_COMPONENTS NumComponents() const {return m_NumComponents;}
    // Get the register index for a given dimension
    UINT RegIndex(UINT Dimension = 0) const {return m_Index[Dimension].m_RegIndex;}
    // Get the register index from the lowest dimension
    UINT RegIndexForMinorDimension() const
    {
        switch (m_IndexDimension)
        {
            default:
            case D3D10_SB_OPERAND_INDEX_1D:
                return RegIndex(0);
            case D3D10_SB_OPERAND_INDEX_2D:
                return RegIndex(1);
            case D3D10_SB_OPERAND_INDEX_3D:
                return RegIndex(2);
        }
    }
    // Get the write mask
    UINT WriteMask() const {return m_WriteMask;}
    // Get the swizzle
    UINT SwizzleComponent(UINT index) const {return m_Swizzle[index];}
    // Get immediate 32 bit value
    UINT Imm32() const {return m_Value[0];}
    void SetModifier(D3D10_SB_OPERAND_MODIFIER Modifier)
    {
        m_Modifier = Modifier;
        if (Modifier != D3D10_SB_OPERAND_MODIFIER_NONE)
        {
            m_bExtendedOperand = true;
            m_ExtendedOperandType = D3D10_SB_EXTENDED_OPERAND_MODIFIER;
        }
    }
    D3D10_SB_OPERAND_MODIFIER Modifier() const {return m_Modifier;}

    __success(return != FALSE) BOOL Disassemble(__out_ecount(StringSize) LPSTR pString, __in_range(>,0) UINT StringSize) const;

public:  //esp in the unions...it's just redundant to not directly access things
    void Clear()
    {
        memset(this, 0, sizeof(*this));
    }
    D3D10_SB_OPERAND_TYPE                        m_Type;
    COperandIndex                                m_Index[3];
    D3D10_SB_OPERAND_NUM_COMPONENTS              m_NumComponents;
    D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE  m_ComponentSelection;
    BOOL                                         m_bExtendedOperand;
    D3D10_SB_OPERAND_MODIFIER                    m_Modifier;
    D3D10_SB_EXTENDED_OPERAND_TYPE               m_ExtendedOperandType;
    D3D11_SB_OPERAND_MIN_PRECISION               m_MinPrecision;
    union
    {
        UINT                     m_WriteMask;
        BYTE                     m_Swizzle[4];
    };
    D3D10_SB_4_COMPONENT_NAME    m_ComponentName;
    union
    {
        UINT                                m_Value[4];
        float                               m_Valuef[4];
        UINT                                m_ValueA[4][2];
        INT64                               m_Value64[4];
    };
    struct
    {
        D3D10_SB_OPERAND_INDEX_REPRESENTATION    m_IndexType[3];
        D3D10_SB_OPERAND_INDEX_DIMENSION         m_IndexDimension;
    };

    friend class CShaderAsm;
    friend class CShaderCodeParser;
    friend class CInstruction;
    friend class COperand;
    friend class COperandDst;
};

//*****************************************************************************
//
// class COperand
//
// Encapsulates a source operand in shader instructions
//
//*****************************************************************************

class COperand: public COperandBase
{
public:
    COperand(): COperandBase() {}
    COperand(UINT Imm32): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_WriteMask = 0;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Value[0] = Imm32;
        m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
    }
    COperand(int Imm32): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_WriteMask = 0;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Value[0] = Imm32;
        m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
    }
    COperand(float Imm32): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_WriteMask = 0;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Valuef[0] = Imm32;
        m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
    }
    COperand(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    // Immediate constant
    COperand(float v1, float v2, float v3, float v4): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Valuef[0] = v1;
        m_Valuef[1] = v2;
        m_Valuef[2] = v3;
        m_Valuef[3] = v4;
    }
    // Immediate constant
    COperand(float v1, float v2, float v3, float v4,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Valuef[0] = v1;
        m_Valuef[1] = v2;
        m_Valuef[2] = v3;
        m_Valuef[3] = v4;
    }

    // Immediate constant
    COperand(int v1, int v2, int v3, int v4): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Value[0] = v1;
        m_Value[1] = v2;
        m_Value[2] = v3;
        m_Value[3] = v4;
    }
    // Immediate constant
    COperand(int v1, int v2, int v3, int v4,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Value[0] = v1;
        m_Value[1] = v2;
        m_Value[2] = v3;
        m_Value[3] = v4;
    }

    COperand(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }

    // Used for operands without indices
    COperand(D3D10_SB_OPERAND_TYPE Type): COperandBase()
    {
        m_Type = Type;
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_bExtendedOperand = FALSE;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        if( Type == D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID )
        {
            m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
        }
        else
        {
            m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
        }
    }

    friend class CShaderAsm;
    friend class CShaderCodeParser;
    friend class CInstruction;
};

//*****************************************************************************
//
// class COperand4
//
// Encapsulates a source operand with 4 components in shader instructions
//
//*****************************************************************************

class COperand4: public COperandBase
{
public:
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, D3D10_SB_4_COMPONENT_NAME Component): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
        m_ComponentName = Component;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    // 4-component source operand with relative addressing
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
             D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex, D3D10_SB_4_COMPONENT_NAME RelComponentName): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        if (RegIndex == 0)
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[0].m_RegIndex = RegIndex;
        m_Index[0].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RelIndex = RelRegIndex;
        m_Index[0].m_RelIndex1 = 0xFFFFFFFF;
        m_Index[0].m_ComponentName = RelComponentName;
    }
    // 4-component source operand with relative addressing
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
        D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex, UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        if (RegIndex == 0)
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[0].m_RegIndex = RegIndex;
        m_Index[0].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RelIndex = RelRegIndex;
        m_Index[0].m_RelIndex1 = RelRegIndex1;
        m_Index[0].m_ComponentName = RelComponentName;
    }
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    // 4-component source operand with relative addressing
    COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
             D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex, UINT RelRegIndex1,
             D3D10_SB_4_COMPONENT_NAME RelComponentName): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        if (RegIndex == 0)
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[0].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RegIndex = RegIndex;
        m_Index[0].m_RelIndex = RelRegIndex;
        m_Index[0].m_RelIndex1 = RelRegIndex1;
        m_Index[0].m_ComponentName = RelComponentName;
    }

    friend class CShaderAsm;
    friend class CShaderCodeParser;
    friend class CInstruction;
};
//*****************************************************************************
//
// class COperandDst
//
// Encapsulates a destination operand in shader instructions
//
//*****************************************************************************

class COperandDst: public COperandBase
{
public:
    COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
        m_WriteMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
        m_WriteMask = WriteMask;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
    }
    COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask,
         D3D10_SB_OPERAND_TYPE RelRegType,
         UINT RelRegIndex, UINT RelRegIndex1,
         D3D10_SB_4_COMPONENT_NAME RelComponentName):COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
        m_WriteMask = WriteMask;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        if (RegIndex == 0)
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[0].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RegIndex = RegIndex;
        m_Index[0].m_RelIndex  = RelRegIndex;
        m_Index[0].m_RelIndex1 = RelRegIndex1;
        m_Index[0].m_ComponentName = RelComponentName;

    }
    COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask,
                D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex, UINT RelRegIndex1,
                D3D10_SB_4_COMPONENT_NAME RelComponentName, UINT) : COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
        m_WriteMask = WriteMask;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex;
        if (RelRegIndex == 0)
            m_IndexType[1] = D3D10_SB_OPERAND_INDEX_RELATIVE;
        else
            m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[1].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[1].m_RegIndex = RelRegIndex;
        m_Index[1].m_RelIndex  = RelRegIndex1;
        m_Index[1].m_RelIndex1 = 0;
        m_Index[1].m_ComponentName = RelComponentName;

    }
    // 2D dst (e.g. for GS input decl)
    COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,UINT WriteMask): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
        m_WriteMask = WriteMask;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
    }
    // Used for operands without indices
    COperandDst(D3D10_SB_OPERAND_TYPE Type): COperandBase()
    {
        if( Type == D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH )
        {
            m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
        }
        else
        {
            m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
        }
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
    }

    friend class CShaderAsm;
    friend class CShaderCodeParser;
    friend class CInstruction;
};

//*****************************************************************************
//
// class COperand2D
//
// Encapsulates 2 dimensional source operand with 4 components in shader instructions
//
//*****************************************************************************

class COperand2D: public COperandBase
{
public:
    COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
    }
    COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1, D3D10_SB_4_COMPONENT_NAME Component): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
        m_ComponentName = Component;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
    }
    // 2-dimensional 4-component operand with relative addressing the second index
    // For example:
    //      c2[x12[3].w + 7]
    //  Type = c
    //  RelRegType = x
    //  RegIndex0 = 2
    //  RegIndex1 = 7
    //  RelRegIndex = 12
    //  RelRegIndex1 = 3
    //  RelComponentName = w
    //
    COperand2D(D3D10_SB_OPERAND_TYPE Type,
              UINT RegIndex0,
              UINT RegIndex1,
              D3D10_SB_OPERAND_TYPE RelRegType,
              UINT RelRegIndex,
              UINT RelRegIndex1,
              D3D10_SB_4_COMPONENT_NAME RelComponentName): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[1].m_RegIndex = RegIndex1;
        m_Index[1].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[1].m_RelIndex  = RelRegIndex;
        m_Index[1].m_RelIndex1 = RelRegIndex1;
        m_Index[1].m_ComponentName = RelComponentName;
    }
    // 2-dimensional 4-component operand with relative addressing a second index
    // For example:
    //      c2[r12.y + 7]
    //  Type = c
    //  RelRegType = r
    //  RegIndex0 = 2
    //  RegIndex1 = 7
    //  RelRegIndex = 12
    //  RelRegIndex1 = 3
    //  RelComponentName = y
    //
    COperand2D(D3D10_SB_OPERAND_TYPE Type,
              UINT RegIndex0,
              UINT RegIndex1,
              D3D10_SB_OPERAND_TYPE RelRegType,
              UINT RelRegIndex,
              D3D10_SB_4_COMPONENT_NAME RelComponentName): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        m_Index[1].m_RegIndex = RegIndex1;
        m_Index[1].m_RelRegType = RelRegType;
        if( RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[1].m_RelIndex  = RelRegIndex;
        m_Index[1].m_ComponentName = RelComponentName;
    }
    // 2-dimensional 4-component operand with relative addressing both operands
    COperand2D(D3D10_SB_OPERAND_TYPE Type,
              BOOL bIndexRelative0, BOOL bIndexRelative1,
              UINT RegIndex0, UINT RegIndex1,
              D3D10_SB_OPERAND_TYPE RelRegType0, UINT RelRegIndex0, UINT RelRegIndex10, D3D10_SB_4_COMPONENT_NAME RelComponentName0,
              D3D10_SB_OPERAND_TYPE RelRegType1, UINT RelRegIndex1, UINT RelRegIndex11, D3D10_SB_4_COMPONENT_NAME RelComponentName1): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = D3D10_SB_4_COMPONENT_X;
        m_Swizzle[1] = D3D10_SB_4_COMPONENT_Y;
        m_Swizzle[2] = D3D10_SB_4_COMPONENT_Z;
        m_Swizzle[3] = D3D10_SB_4_COMPONENT_W;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        if (bIndexRelative0)
            if (RegIndex0 == 0)
                m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
            else
                m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_Index[0].m_RelRegType = RelRegType0;
        if( RelRegType0 == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RelIndex = RelRegIndex0;
        m_Index[0].m_RelIndex1 = RelRegIndex10;
        m_Index[0].m_ComponentName = RelComponentName0;
        if (bIndexRelative1)
            if (RegIndex1 == 0)
                m_IndexType[1] = D3D10_SB_OPERAND_INDEX_RELATIVE;
            else
                m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        else
            m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
        m_Index[1].m_RelRegType = RelRegType1;
        if( RelRegType1 == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[1].m_RelIndex = RelRegIndex1;
        m_Index[1].m_RelIndex1 = RelRegIndex11;
        m_Index[1].m_ComponentName = RelComponentName1;
    }
    COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
              BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
    }
    // 2-dimensional 4-component operand with relative addressing and swizzle
    COperand2D(D3D10_SB_OPERAND_TYPE Type,
              BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
              BOOL bIndexRelative0, BOOL bIndexRelative1,
              UINT RegIndex0, D3D10_SB_OPERAND_TYPE RelRegType0, UINT RelRegIndex0, UINT RelRegIndex10, D3D10_SB_4_COMPONENT_NAME RelComponentName0,
              UINT RegIndex1, D3D10_SB_OPERAND_TYPE RelRegType1, UINT RelRegIndex1, UINT RelRegIndex11, D3D10_SB_4_COMPONENT_NAME RelComponentName1): COperandBase()
    {
        m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
        m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
        m_Swizzle[0] = SwizzleX;
        m_Swizzle[1] = SwizzleY;
        m_Swizzle[2] = SwizzleZ;
        m_Swizzle[3] = SwizzleW;
        m_Type = Type;
        m_bExtendedOperand = FALSE;
        m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
        m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        if (bIndexRelative0)
            if (RegIndex0 == 0)
                m_IndexType[0] = D3D10_SB_OPERAND_INDEX_RELATIVE;
            else
                m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        else
            m_IndexType[0] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[0].m_RegIndex = RegIndex0;
        m_Index[0].m_RelRegType = RelRegType0;
        if( RelRegType0 == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[0].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[0].m_RelIndex = RelRegIndex0;
        m_Index[0].m_RelIndex1 = RelRegIndex10;
        m_Index[0].m_ComponentName = RelComponentName0;
        if (bIndexRelative1)
            if (RegIndex1 == 0)
                m_IndexType[1] = D3D10_SB_OPERAND_INDEX_RELATIVE;
            else
                m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
        else
            m_IndexType[1] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
        m_Index[1].m_RegIndex = RegIndex1;
        m_Index[1].m_RelRegType = RelRegType1;
        if( RelRegType1 == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP )
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
        }
        else
        {
            m_Index[1].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
        }
        m_Index[1].m_RelIndex = RelRegIndex1;
        m_Index[1].m_RelIndex1 = RelRegIndex11;
        m_Index[1].m_ComponentName = RelComponentName1;
    }

    friend class CShaderAsm;
    friend class CShaderCodeParser;
    friend class CInstruction;
};

//*****************************************************************************
//
//  CInstruction
//
//*****************************************************************************

// Structures for additional per-instruction fields unioned in CInstruction.
// These structures don't contain ALL info used by the particular instruction,
// only additional info not already in CInstruction.  Some instructions don't
// need such structures because CInstruction already has the correct data
// fields.
struct CResourceSpaceDecl
{
    UINT SetIdx;
    UINT MinShaderRegister;
    UINT MaxShaderRegister;
    UINT Space;
};

struct CGlobalFlagsDecl
{
    UINT Flags;
};

struct CInputSystemInterpretedValueDecl
{
    D3D10_SB_NAME  Name;
};

struct CInputSystemGeneratedValueDecl
{
    D3D10_SB_NAME  Name;
};

struct CInputPSDecl
{
    D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct CInputPSSystemInterpretedValueDecl
{
    D3D10_SB_NAME  Name;
    D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct CInputPSSystemGeneratedValueDecl
{
    D3D10_SB_NAME  Name;
    D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct COutputSystemInterpretedValueDecl
{
    D3D10_SB_NAME  Name;
};

struct COutputSystemGeneratedValueDecl
{
    D3D10_SB_NAME  Name;
};

struct CIndexRangeDecl
{
    UINT    RegCount;
};

struct COutputTopologyDecl
{
    D3D10_SB_PRIMITIVE_TOPOLOGY    Topology;
};

struct CInputPrimitiveDecl
{
    D3D10_SB_PRIMITIVE             Primitive;
};

struct CGSMaxOutputVertexCountDecl
{
    UINT    MaxOutputVertexCount;
};

struct CGSInstanceCountDecl
{
    UINT    InstanceCount;
};

struct CExtendedResourceDecl
{
    D3D10_SB_RESOURCE_DIMENSION Dimension;
};

//
// Used for typed, raw and structured UAVs
//
struct CResourceDecl
{
    // Used for all
    CResourceSpaceDecl              Space;
    CResourceDecl*                  m_pNext; // shader holds a linked list of dcls

    // For each resource type we have a different set of info structs
    union
    {
        struct
        {
            D3D10_SB_RESOURCE_DIMENSION     Dimension;
            D3D10_SB_OPCODE_TYPE            UAVType;
            UINT                            Coherency; //0 if local
            UINT                            Stride;
            UINT                            Counter;
            D3D10_SB_RESOURCE_RETURN_TYPE   Type;
        } UAVInfo;

        struct
        {
            D3D10_SB_RESOURCE_DIMENSION     Dimension;
            D3D10_SB_RESOURCE_RETURN_TYPE   ReturnType[4];
            UINT                            SampleCount;

            // for raw and structured resources
            D3D10_SB_OPCODE_TYPE            UAVType;
            UINT                            Stride;
        } SRVInfo;

        struct
        {
            D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN AccessPattern;
            UINT Size; // currently only available in SM 5.1
        } CBInfo;

        struct
        {
            D3D10_SB_SAMPLER_MODE   SamplerMode;
        } SamplerInfo;
    };
};

//
// Used for raw and structured thread group shared memory
//
struct CTGSMDecl
{
    D3D10_SB_OPCODE_TYPE Type;
    UINT Index;
    UINT StructStride;
    UINT StructCount;
};

struct CTempsDecl
{
    UINT    NumTemps;
};

struct CIndexableTempDecl
{
    UINT    IndexableTempNumber;
    UINT    NumRegisters;
    UINT    Mask; // .x, .xy, .xzy or .xyzw (D3D10_SB_OPERAND_4_COMPONENT_MASK_* )
};

struct CCustomData
{
    D3D10_SB_CUSTOMDATA_CLASS  Type;
    UINT                    DataSizeInBytes;
    void*                   pData;
};

struct CSyncFlags
{
    bool bThreadsInGroup;
    bool bThreadGroupSharedMemory;
    bool bUnorderedAccessViewMemoryGlobal;
    bool bUnorderedAccessViewMemoryGroup; // exclusive to global
};

struct CFunctionTable
{
    int FtIdx;
    UINT FbCount;
    CShaderToken* pFbStartToken;
};

struct CInterfaceTable
{
    int FpIdx;
    bool bDynamic;
    UINT NumCallSites;
    UINT FpArraySize;
    UINT FtCount;
    CShaderToken* pFtStartToken;
};

struct CHSDSInputControlPointCountDecl
{
    UINT    InputControlPointCount;
};

struct CHSOutputControlPointCountDecl
{
    UINT    OutputControlPointCount;
};

struct CTessellatorDomainDecl
{
    D3D11_SB_TESSELLATOR_DOMAIN TessellatorDomain;
};

struct CTessellatorPartitioningDecl
{
    D3D11_SB_TESSELLATOR_PARTITIONING TessellatorPartitioning;
};

struct CTessellatorOutputPrimitiveDecl
{
    D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE TessellatorOutputPrimitive;
};

struct CHSMaxTessFactorDecl
{
    float MaxTessFactor;
};

struct CHSForkPhaseDecl
{
    UINT InstanceCount;
};

struct CHSJoinPhaseDecl
{
    UINT InstanceCount;
};

class CInstruction
{
protected:
    static const UINT MAX_PRIVATE_DATA_COUNT = 2;

public:

    CInstruction() : m_OpCode(D3D10_SB_OPCODE_ADD)
    {
        Clear();
    }

    CInstruction(D3D10_SB_OPCODE_TYPE OpCode)
    {
        Clear();
        m_OpCode = OpCode;
    }

    CInstruction(D3D10_SB_OPCODE_TYPE OpCode,
                 const COperandBase& Operand0,
                 D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test)
    {
        Clear();
        m_OpCode = OpCode;
        m_NumOperands = 1;
        m_Test = Test;
        m_Operands[0] = Operand0;
    }

    CInstruction(D3D10_SB_OPCODE_TYPE OpCode,
                 const COperandBase& Operand0,
                 const COperandBase& Operand1)
    {
        Clear();
        m_OpCode = OpCode;
        m_NumOperands = 2;
        m_Operands[0] = Operand0;
        m_Operands[1] = Operand1;
    }

    CInstruction(D3D10_SB_OPCODE_TYPE OpCode,
                 const COperandBase& Operand0,
                 const COperandBase& Operand1,
                 const COperandBase& Operand2)
    {
        Clear();
        m_OpCode = OpCode;
        m_NumOperands = 3;
        m_Operands[0] = Operand0;
        m_Operands[1] = Operand1;
        m_Operands[2] = Operand2;

    }

    CInstruction(D3D10_SB_OPCODE_TYPE OpCode,
                 const COperandBase& Operand0,
                 const COperandBase& Operand1,
                 const COperandBase& Operand2,
                 const COperandBase& Operand3)
    {
        Clear();
        m_OpCode = OpCode;
        m_NumOperands = 4;
        m_Operands[0] = Operand0;
        m_Operands[1] = Operand1;
        m_Operands[2] = Operand2;
        m_Operands[3] = Operand3;
    }

    ~CInstruction()
    {
        if (m_OpCode == D3D10_SB_OPCODE_CUSTOMDATA)
        {
            delete[] m_CustomData.pData;
            m_CustomData.pData = NULL;
        }
    }

    void Clear(bool bFullInitialization = false)
    {
        if (bFullInitialization)
        {
            if (m_OpCode == D3D10_SB_OPCODE_CUSTOMDATA)
            {
                delete [] m_CustomData.pData;
                m_CustomData.pData = NULL;
            }

            memset(this, 0, sizeof(CInstruction));
        }
        else
        {
            memset(this, 0, offsetof(CInstruction, m_Operands));
        }
    }

    const COperandBase& Operand(UINT Index) const
    {
        return m_Operands[Index];
    }

    D3D10_SB_OPCODE_TYPE OpCode() const
    {
        return m_OpCode;
    }

    void SetNumOperands(UINT NumOperands)
    {
        m_NumOperands = NumOperands;
    }

    UINT NumOperands() const
    {
        return m_NumOperands;
    }

    void SetTest(D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test)
    {
        m_Test = Test;
    }

    D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test() const
    {
        return m_Test;
    }

    void SetTexelOffset( const INT8 texelOffset[3] )
    {
        m_bExtended = TRUE;
        m_OpCodeEx[m_ExtendedOpCodeCount++] = D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS;
        memcpy(m_TexelOffset, texelOffset,sizeof(m_TexelOffset));
    }

    void SetTexelOffset( INT8 x, INT8 y, INT8 z)
    {
        m_bExtended = TRUE;
        m_OpCodeEx[m_ExtendedOpCodeCount++] = D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS;
        m_TexelOffset[0] = x;
        m_TexelOffset[1] = y;
        m_TexelOffset[2] = z;
    }

    __success(return != FALSE) BOOL Disassemble(__out_ecount(StringSize) __nullterminated LPSTR pString, __in_range(>,0) UINT StringSize);

    // Private data is used by D3D runtime
    void SetPrivateData(UINT Value, UINT index = 0)
    {
        if (index < MAX_PRIVATE_DATA_COUNT)
        {
            m_PrivateData[index] = Value;
        }
    }

    UINT PrivateData(UINT index = 0) const
    {
        if (index >= MAX_PRIVATE_DATA_COUNT)
            return 0xFFFFFFFF;
        return m_PrivateData[index];
    }

    D3D10_SB_OPCODE_TYPE        m_OpCode;
    UINT                        m_NumOperands;
    BOOL                        m_bExtended;
    BOOL                        m_bExtendedResourceDim;
    UINT                        m_ExtendedOpCodeCount;
    D3D10_SB_EXTENDED_OPCODE_TYPE  m_OpCodeEx[D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES];
    INT8                        m_TexelOffset[3];
    UINT                        m_PrivateData[MAX_PRIVATE_DATA_COUNT];
    BOOL                        m_bSaturate;

    union // extra info needed by some instructions
    {
        CInputSystemInterpretedValueDecl    m_InputDeclSIV;
        CInputSystemGeneratedValueDecl      m_InputDeclSGV;
        CInputPSDecl                        m_InputPSDecl;
        CInputPSSystemInterpretedValueDecl  m_InputPSDeclSIV;
        CInputPSSystemGeneratedValueDecl    m_InputPSDeclSGV;
        COutputSystemInterpretedValueDecl   m_OutputDeclSIV;
        COutputSystemGeneratedValueDecl     m_OutputDeclSGV;
        CIndexRangeDecl                     m_IndexRangeDecl;
        CResourceDecl                       m_ResourceDecl;
        CExtendedResourceDecl               m_ExtendedResourceDecl;
        CInputPrimitiveDecl                 m_InputPrimitiveDecl;
        COutputTopologyDecl                 m_OutputTopologyDecl;
        CGSMaxOutputVertexCountDecl         m_GSMaxOutputVertexCountDecl;
        CGSInstanceCountDecl                m_GSInstanceCount;
        CTGSMDecl                           m_TGSMInfo;
        CTempsDecl                          m_TempsDecl;
        CIndexableTempDecl                  m_IndexableTempDecl;
        CGlobalFlagsDecl                    m_GlobalFlagsDecl;
        CCustomData                         m_CustomData;
        D3D10_SB_INSTRUCTION_TEST_BOOLEAN   m_Test;
        D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE m_ResInfoReturnType;
        D3D10_SB_INSTRUCTION_RETURN_TYPE    m_InstructionReturnType;
        CSyncFlags                          m_SyncFlags;
        int                                 m_FunctionBodyIdx;
        CFunctionTable                      m_FunctionTable;
        CInterfaceTable                     m_InterfaceTable;
        int                                 m_InterfaceCallSiteIdx;
        CHSDSInputControlPointCountDecl     m_InputControlPointCountDecl;
        CHSOutputControlPointCountDecl      m_OutputControlPointCountDecl;
        CTessellatorDomainDecl              m_TessellatorDomainDecl;
        CTessellatorPartitioningDecl        m_TessellatorPartitioningDecl;
        CTessellatorOutputPrimitiveDecl     m_TessellatorOutputPrimitiveDecl;
        CHSMaxTessFactorDecl                m_HSMaxTessFactorDecl;
        CHSForkPhaseDecl                    m_HSForkPhaseDecl;
        CHSJoinPhaseDecl                    m_HSJoinPhaseDecl;
    };

    // Carefull: the memory is zeroes by the constructors up to this point!
    COperandBase                m_Operands[D3D11_SB_MAX_INSTRUCTION_OPERANDS];
};

// ****************************************************************************
//
// class CShaderAsm
//
// The class is used to build a binary representation of a shader.
// Usage scenario:
//      1. Call Init with the initial internal buffer size in UINTs. The
//         internal buffer will grow if needed
//      2. Call StartShader()
//      3. Call Emit*() functions to assemble a shader
//      4. Call EndShader()
//      5. Call GetShader() to get the binary representation
//
//
// ****************************************************************************
class CShaderAsm
{
public:
    CShaderAsm():
        m_dwFunc(NULL),
        m_Status(S_OK),
        m_Index(0),
        m_StartOpIndex(0),
        m_BufferSize(0)
    {
    };

    ~CShaderAsm()
    {
        delete[] m_dwFunc;
    };

    // Initializes the object with the initial buffer size in UINTs
    __checkReturn HRESULT Init()
    {
        HRESULT hr = S_OK;

        m_BufferSize = 1024;

        IFCOOM((m_dwFunc = new UINT[m_BufferSize]));
        
        Reset();

Cleanup:
        m_Status = hr;
        RRETURN(hr);
    }

    UINT* GetShader()               {return m_dwFunc;}
    UINT  ShaderSizeInDWORDs()      {return m_Index;}
    HRESULT GetStatus()             {return m_Status;}
    void SetStatus(HRESULT hr)      {m_Status = hr;}

    // This function should be called to mark the start of a shader
    void StartShader(D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType, UINT vermajor,UINT verminor)
    {
        Reset();
        UINT Token = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(ShaderType, vermajor, verminor);
        OPCODE(Token);
        OPCODE(0);        // Reserve space for length
    }

    // Should be called at the end of the shader
    __checkReturn HRESULT EndShader()
    {
        if (1 < m_BufferSize)
            m_dwFunc[1] = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_LENGTH(m_Index);

        return m_Status;
    }

    // Emit a resource declaration
    void EmitResourceDecl(D3D10_SB_RESOURCE_DIMENSION Dimension, UINT TRegIndex,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
               ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) );
        EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
        FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
        ENDINSTRUCTION();
    }
    // Emit a resource declaration (multisampled)
    void EmitResourceMSDecl(D3D10_SB_RESOURCE_DIMENSION Dimension, UINT TRegIndex,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW,
                          UINT SampleCount)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
               ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) |
               ENCODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(SampleCount));
        EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
        FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
             ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
        ENDINSTRUCTION();
    }
    // Emit a sampler declaration
    void EmitSamplerDecl(UINT SRegIndex, D3D10_SB_SAMPLER_MODE Mode)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE( ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_SAMPLER) |
                ENCODE_D3D10_SB_SAMPLER_MODE(Mode) );
        EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_SAMPLER, SRegIndex));
        ENDINSTRUCTION();
    }

    // Emit an input declaration
    void EmitInputDecl(UINT RegIndex, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        ENDINSTRUCTION();
    }
    void EmitInputDecl2D(UINT RegIndex, UINT RegIndex2, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2, WriteMask));
        ENDINSTRUCTION();
    }

    // Emit an input declaration for a system interpreted value
    void EmitInputSystemInterpretedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SIV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    void EmitInputSystemInterpretedValueDecl2D(UINT RegIndex, UINT RegIndex2, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SIV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    // Emit an input declaration for a system generated value
    void EmitInputSystemGeneratedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SGV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    void EmitInputSystemGeneratedValueDecl2D(UINT RegIndex, UINT RegIndex2, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SGV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    // Emit a PS input declaration
    void EmitPSInputDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_INTERPOLATION_MODE Mode)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE( ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS) |
                ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        ENDINSTRUCTION();
    }
    // Emit a PS input declaration for a system interpreted value
    void EmitPSInputSystemInterpretedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_INTERPOLATION_MODE Mode, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE( ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV) |
                ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    // Emit a PS input declaration for a system generated value
    void EmitPSInputSystemGeneratedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_INTERPOLATION_MODE Mode, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE( ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS_SGV) |
                ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    // Emit input primitive id declaration
    void EmitInputPrimIdDecl()
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID));
        ENDINSTRUCTION();
    }
    // Emit and oDepth declaration
    void EmitODepthDecl()
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH));
        ENDINSTRUCTION();
    }
    // Emit an oMask declaration
    void EmitOMaskDecl()
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK));
        ENDINSTRUCTION();
    }
    // Emit an output declaration
    void EmitOutputDecl(UINT RegIndex, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask));
        ENDINSTRUCTION();
    }
    // Emit an output declaration for a system interpreted value
    void EmitOutputSystemInterpretedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT_SIV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }
    // Emit an output declaration for a system generated value
    void EmitOutputSystemGeneratedValueDecl(UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT_SGV));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask));
        FUNC(ENCODE_D3D10_SB_NAME(Name));
        ENDINSTRUCTION();
    }

    // Emit an input register indexing range declaration
    void EmitInputIndexingRangeDecl(UINT RegIndex, UINT Count, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
        FUNC((UINT)Count);
        ENDINSTRUCTION();
    }

    // 2D indexing range decl (indexing is for second dimension)
    void EmitInputIndexingRangeDecl2D(UINT RegIndex, UINT RegIndex2Min, UINT Reg2Count, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2Min, WriteMask));
        FUNC((UINT)Reg2Count);
        ENDINSTRUCTION();
    }

    // Emit an output register indexing range declaration
    void EmitOutputIndexingRangeDecl(UINT RegIndex, UINT Count, UINT WriteMask)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
        EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask));
        FUNC((UINT)Count);
        ENDINSTRUCTION();
    }

    // Emit a temp registers ( r0...r(n-1) ) declaration
    void EmitTempsDecl(UINT NumTemps)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_TEMPS));
        FUNC((UINT)NumTemps);
        ENDINSTRUCTION();
    }

    // Emit an indexable temp register (x#) declaration
    void EmitIndexableTempDecl(UINT TempNumber, UINT RegCount, UINT ComponentCount )
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP));
        FUNC((UINT)TempNumber);
        FUNC((UINT)RegCount);
        FUNC((UINT)ComponentCount);
        ENDINSTRUCTION();
    }

    // Emit a constant buffer (cb#) declaration
    void EmitConstantBufferDecl(UINT RegIndex, UINT Size, // size 0 means unknown/any size
                                D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN AccessPattern)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE( ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER) |
                ENCODE_D3D10_SB_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(AccessPattern));
        EmitOperand(COperand2D(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER, RegIndex, Size));
        ENDINSTRUCTION();
    }

    // Emit Immediate Constant Buffer (icb) declaration
    void EmitImmediateConstantBufferDecl(UINT Num4Tuples, const UINT* pImmediateConstantBufferData)
    {
        m_bExecutableInstruction = FALSE;
        EmitCustomData( D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER,
                        4*Num4Tuples /*2 UINTS will be added during encoding */,
                        pImmediateConstantBufferData);
    }

    // Emit a GS input primitive declaration
    void EmitGSInputPrimitiveDecl(D3D10_SB_PRIMITIVE Primitive)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE) |
               ENCODE_D3D10_SB_GS_INPUT_PRIMITIVE(Primitive));
        ENDINSTRUCTION();
    }

    // Emit a GS output topology declaration
    void EmitGSOutputTopologyDecl(D3D10_SB_PRIMITIVE_TOPOLOGY Topology)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY) |
               ENCODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(Topology));
        ENDINSTRUCTION();
    }

    // Emit GS Maximum Output Vertex Count declaration
    void EmitGSMaxOutputVertexCountDecl(UINT Count)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT));
        FUNC((UINT)Count);
        ENDINSTRUCTION();
    }
    // Emit global flags declaration
    void EmitGlobalFlagsDecl(UINT Flags)
    {
        m_bExecutableInstruction = FALSE;
        OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS) |
               ENCODE_D3D10_SB_GLOBAL_FLAGS(Flags));
        ENDINSTRUCTION();
    }

    // Emit an instruction. Custom-data is not handled by this function.
    void EmitInstruction(const CInstruction& instruction);
    // Emit an operand
    void EmitOperand(const COperandBase& operand);
    // Emit an instruction without operands
    void Emit(UINT OpCode)
    {
        OPCODE(OpCode);
        ENDINSTRUCTION();
    }

    void EmitCustomData( D3D10_SB_CUSTOMDATA_CLASS CustomDataClass,
                         UINT SizeInUINTs /*2 UINTS will be added during encoding */,
                         const UINT* pCustomData)
    {
        if (SUCCEEDED(m_Status))
        {
            if( ((m_Index + SizeInUINTs) < m_Index) ||  // wrap
                 (SizeInUINTs > 0xfffffffd) )           // need to add 2, also 0xffffffff isn't caught above
            {
                m_Status = E_OUTOFMEMORY;
            }

            UINT FullSizeInUINTs = SizeInUINTs + 2; // include opcode and size
            if( m_Index + FullSizeInUINTs >= m_BufferSize )
            {
                Reserve(FullSizeInUINTs);
            }

            if (SUCCEEDED(m_Status))
            {
                if (m_Index < m_BufferSize)
                    m_dwFunc[m_Index++] = ENCODE_D3D10_SB_CUSTOMDATA_CLASS(CustomDataClass);
                if (m_Index < m_BufferSize)
                    m_dwFunc[m_Index++] = FullSizeInUINTs;
                if (m_Index < m_BufferSize)
                    memcpy(&m_dwFunc[m_Index],pCustomData,sizeof(UINT)*SizeInUINTs);

                AssertAndAssume((m_Index + SizeInUINTs) <= m_BufferSize);
                m_Index += SizeInUINTs;
            }
        }
    }

    // Returns number of executable instructions in the current shader
    UINT GetNumExecutableInstructions() {return m_NumExecutableInstructions;}

    void EmitBinary(CONST DWORD*, DWORD);

    struct SnapShot
    {
        UINT    Index;
        UINT    StartOpIndex;
        UINT    StatementIndex;
        UINT    NumExecutableInstructions;
        bool    ExecutableInstruction;
    };

    SnapShot SnapShotState()
    {
        SnapShot Ret;

        Ret.Index                     = m_Index;
        Ret.StartOpIndex              = m_StartOpIndex;
        Ret.StatementIndex            = m_StatementIndex;
        Ret.NumExecutableInstructions = m_NumExecutableInstructions;
        Ret.ExecutableInstruction     = m_bExecutableInstruction;

        return Ret;
    }

    void RestoreState(const SnapShot& Token)
    {
        AssertAndAssume(Token.Index <= m_BufferSize);

        m_Index                     = Token.Index;
        m_StartOpIndex              = Token.StartOpIndex;
        m_StatementIndex            = Token.StatementIndex;
        m_NumExecutableInstructions = Token.NumExecutableInstructions;
        m_bExecutableInstruction    = Token.ExecutableInstruction;
    }

    UINT InstructionCount()
    {
        return m_NumExecutableInstructions;
    }

protected:
    void OPCODE(UINT x)
    {
        if (SUCCEEDED(m_Status))
        {
            if (m_Index < m_BufferSize)
            {
                m_dwFunc[m_Index] = x;
                m_StartOpIndex = m_Index++;
            }
            if (m_Index >= m_BufferSize)
                Reserve(1024);
        }
    }
    // Should be called after end of each instruction
    void ENDINSTRUCTION()
    {
        if (m_StartOpIndex < m_Index)
        {
            if (m_Index - m_StartOpIndex < MAX_INSTRUCTION_LENGTH)
                m_dwFunc[m_StartOpIndex] |= ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(m_Index - m_StartOpIndex);

            Reserve(MAX_INSTRUCTION_LENGTH);

            if (SUCCEEDED(m_Status))
            {
                m_StatementIndex++;
                if (m_bExecutableInstruction)
                    m_NumExecutableInstructions++;
                m_bExecutableInstruction = true;
            }
        }
    }
    void FUNC(UINT x)
    {
        if (SUCCEEDED(m_Status))
        {
            if (m_Index < m_BufferSize)
                m_dwFunc[m_Index++] = x;
            if (m_Index >= m_BufferSize)
                Reserve(1024);
        }
    }
    // Prepare assembler for a new shader
    void Reset()
    {
        m_Index = 0;
        m_Status = S_OK;
        m_StartOpIndex = 0;
        m_StatementIndex = 1;
        m_NumExecutableInstructions = 0;
        m_bExecutableInstruction = TRUE;
    }
    // Reserve SizeInUINTs UINTs in the m_dwFunc array
    void Reserve(UINT SizeInUINTs)
    {
        if (SUCCEEDED(m_Status))
        {
            if( m_Index + SizeInUINTs < m_Index ) // overflow (prefix)
            {
                m_Status = E_OUTOFMEMORY;
                return;
            }
            if (m_BufferSize < (m_Index + SizeInUINTs))
            {
                UINT NewSize = m_BufferSize + SizeInUINTs + 1024;
                UINT* pNewBuffer = new UINT[NewSize];
                if (pNewBuffer == NULL)
                {
                    m_Status = E_OUTOFMEMORY;
                    return;
                }
                memcpy(pNewBuffer, m_dwFunc, sizeof(UINT)*m_Index);
                delete [] m_dwFunc;
                m_dwFunc = pNewBuffer;
                m_BufferSize = NewSize;
            }
        }
    }
    // Buffer where the binary representation is built
    // Had to use xcount as it doesn't realize that when m_Index is > m_BufferSize then in OPCODE and FUNC the buffer is expanded
    __field_xcount_part(m_BufferSize, m_Index) UINT*  m_dwFunc;
    // Index where to place the next token in the m_dwFunc array
    UINT    m_Index;
    // Index of the start of the current instruction in the m_dwFunc array
    UINT    m_StartOpIndex;
    // Current buffer size in UINTs
    UINT    m_BufferSize;
    // Current statement index of the current vertex shader
    UINT    m_StatementIndex;
    // Number of executable instructions in the shader
    UINT    m_NumExecutableInstructions;
    // Current error code while writing out shader (S_OK, E_INVALIDARG or E_OUTOFMEMORY)
    HRESULT m_Status;
    // "true" when the current instruction is executable
    bool    m_bExecutableInstruction;
};

//*****************************************************************************
//
//  CShaderCodeParser
//
//*****************************************************************************

struct ParserPositionToken
{
    CShaderToken* Inst;
    UINT          InstCount;
};

class CShaderCodeParser
{
public:
    CShaderCodeParser():
        m_pShaderCode(NULL),
        m_pOriginalShaderCode(NULL),
        m_pCurrentToken(NULL),
        m_pShaderEndToken(NULL),
        m_NumParsedInstructions(0)
    {
    }
    CShaderCodeParser(CONST CShaderToken* pBuffer):
        m_pShaderCode(NULL),
        m_pOriginalShaderCode(NULL),
        m_pCurrentToken(NULL),
        m_pShaderEndToken(NULL),
        m_NumParsedInstructions(0)
    {
        SetShader(pBuffer);
    }
    ~CShaderCodeParser()    {}
    void SetShader(CONST CShaderToken* pBuffer);
    D3D10_SB_OPCODE_TYPE PeekNextInstructionOpCode();
    __checkReturn HRESULT ParseInstruction(CInstruction* pInstruction);
    __checkReturn HRESULT ParseOperand(COperandBase* pOperand);
    void ParseResourceDcl(CInstruction* pInstruction, CResourceSpaceDecl* pSpaceDcl);
    bool EndOfShader() {return m_pCurrentToken >= m_pShaderEndToken;}
    D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType();
    UINT ShaderMinorVersion();
    UINT ShaderMajorVersion();
    UINT ShaderLengthInTokens();
    ParserPositionToken GetCurrentToken();
    UINT CurrentTokenOffsetInBytes();
    void SetCurrentToken(ParserPositionToken In);
    D3D10_SB_OPCODE_TYPE CurrentOpcode(); // Returns the OpCode for the next instruction to be parsed
    DWORD CurrentInstructionLength(); // Number of DWORDS in the current instruction
    void Advance(UINT InstructionSize);
    UINT ParsedInstructionCount() {return m_NumParsedInstructions;}
    bool IsValid() { return m_pShaderCode != nullptr; }
protected:
    CShaderToken*   m_pCurrentToken;
    CShaderToken*   m_pShaderCode;
    CShaderToken*   m_pOriginalShaderCode;
    // Points to the last token of the current shader
    CShaderToken*   m_pShaderEndToken;

    UINT            m_NumParsedInstructions;
};


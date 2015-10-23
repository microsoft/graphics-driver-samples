#include "roscompiler.h"

#define COLOR_COMMENT   0//(FOREGROUND_INTENSITY)
#define COLOR_KEYWORD   1//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_TEXT      2//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_LITERAL   3//(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

HLSLDisasm::HLSLDisasm()
{
    m_cbSize        = 0;
    m_cbSizeMax     = 0;
    m_pBuf          = NULL;
    m_bColorCode    = FALSE;
    m_pFile         = NULL;
    m_pCustomCtx    = NULL;
    m_pStrPrinter   = NULL;
}

HLSLDisasm::HLSLDisasm(void *pFile, fnPrinter *pStrPrinter, void *pCustomCtx)
{
    m_cbSize        = 0;
    m_cbSizeMax     = 0;
    m_pBuf          = NULL;
    m_bColorCode    = FALSE;
    m_pFile         = pFile;
    m_pCustomCtx    = pCustomCtx;
    m_pStrPrinter   = pStrPrinter;
}

HLSLDisasm::~HLSLDisasm()
{
	delete[] m_pBuf;
}

HRESULT DisasembleShader(const UINT * pShader, HLSLDisasm * pCtx);

HRESULT
HLSLDisasm::Run(const UINT * pShader)
{
    if(pShader)
        return DisasembleShader(pShader, this);
    else
    {
        xprintf("No shader active");
        Flush(0);
        return S_OK;
    }
}

void HLSLDisasm::SetColor(WORD wColor)
{
    const char* x_rgszFontColor[4] =
    {
        "a0a0a0", // COLOR_COMMENT   0//(FOREGROUND_INTENSITY)
        "ffff40", // COLOR_KEYWORD   1//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
        "e0e0e0", // COLOR_TEXT      2//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
        "00ffff"  // COLOR_LITERAL   3//(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
    };

    if(m_bColorCode)
    {
        xprintf("<font color = \"#%s\">", x_rgszFontColor[wColor]);
    }
}

void HLSLDisasm::UnsetColor()
{
    if(m_bColorCode)
    {
        xprintf("</font>");
    }
}

void HLSLDisasm::xprintf(LPCSTR pStr, ...)
{
    char sz[512];
    va_list ap;
    va_start(ap, pStr);
    vsprintf_s(sz, sizeof(sz), pStr, ap);
    va_end(ap);

    xaddstring(sz);
}

void HLSLDisasm::Flush(int Line)
{
	if (!EnsureSize(1)) return;
	m_pBuf[m_cbSize] = '\0';
	if (m_pStrPrinter)
    {
		(m_pStrPrinter)(m_pFile, m_pBuf, Line, m_pCustomCtx);
    }
	else
    {
		OutputDebugStringA(m_pBuf);
		OutputDebugStringA("\n");
    }
    m_cbSize = 0;
}

void HLSLDisasm::xaddstring(LPCSTR sz)
{
	const size_t cbSize = strlen(sz);
    if(!EnsureSize(cbSize)) return;
    memcpy(&m_pBuf[m_cbSize], sz, cbSize);
    m_cbSize += cbSize;
}

bool HLSLDisasm::EnsureSize(const size_t cbSize)
{
    if(m_cbSizeMax < m_cbSize + cbSize)
    {
        const size_t kBufInc = 8*1024;
        char *pNewBuf = new char[m_cbSizeMax + cbSize + kBufInc];
        if(pNewBuf == NULL)
        {
            return false;
        }

        memcpy(pNewBuf, m_pBuf, m_cbSize);
		delete[] m_pBuf;
        m_pBuf = pNewBuf;
        m_cbSizeMax = m_cbSizeMax + cbSize + kBufInc;
    }

    return true;
}

//----------------------------------------------------------------------------
// Misc functions
//----------------------------------------------------------------------------
HRESULT PrintComponent(HLSLDisasm *pCtx, D3D10_SB_4_COMPONENT_NAME CompName)
{
    switch(CompName)
    {
        case D3D10_SB_4_COMPONENT_X: pCtx->xprintf("x");break;
        case D3D10_SB_4_COMPONENT_Y: pCtx->xprintf("y");break;
        case D3D10_SB_4_COMPONENT_Z: pCtx->xprintf("z");break;
        case D3D10_SB_4_COMPONENT_W: pCtx->xprintf("w");break;
        default: return E_FAIL;
    }
    return S_OK;
}

HRESULT PrintRegister(HLSLDisasm *pCtx, D3D10_SB_OPERAND_TYPE Type)
{
    switch(Type)
    {
        case D3D10_SB_OPERAND_TYPE_TEMP:                       pCtx->xprintf("r"); break;
        case D3D10_SB_OPERAND_TYPE_INPUT:                      pCtx->xprintf("v"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT:                     pCtx->xprintf("o"); break;
        case D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP:             pCtx->xprintf("x"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:                pCtx->xprintf("i32"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:                pCtx->xprintf("i64"); break;
        case D3D10_SB_OPERAND_TYPE_SAMPLER:                    pCtx->xprintf("s"); break;
        case D3D10_SB_OPERAND_TYPE_RESOURCE:                   pCtx->xprintf("t"); break;
        case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER:            pCtx->xprintf("cb"); break;
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE_CONSTANT_BUFFER:  pCtx->xprintf("icb"); break;
        case D3D10_SB_OPERAND_TYPE_LABEL:                      pCtx->xprintf("l"); break;
        case D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID:          pCtx->xprintf("primID"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:               pCtx->xprintf("oDepth"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:         pCtx->xprintf("oStencilRef"); break;
        case D3D10_SB_OPERAND_TYPE_NULL:                       pCtx->xprintf("null"); break;
        case D3D11_SB_OPERAND_TYPE_STREAM:                     pCtx->xprintf("stream"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_BODY:              pCtx->xprintf("fb"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_TABLE:             pCtx->xprintf("ft"); break;
        case D3D11_SB_OPERAND_TYPE_INTERFACE:                  pCtx->xprintf("fp"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_INPUT:             pCtx->xprintf("f_input"); break;
        case D3D11_SB_OPERAND_TYPE_FUNCTION_OUTPUT:            pCtx->xprintf("f_output"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID:    pCtx->xprintf("out_cpid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID:     pCtx->xprintf("in_fork_instID"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID:     pCtx->xprintf("in_join_instID"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT:        pCtx->xprintf("vicp"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT:       pCtx->xprintf("vocp"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_PATCH_CONSTANT:       pCtx->xprintf("in_patch_const"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT:         pCtx->xprintf("domainpt"); break;
        case D3D11_SB_OPERAND_TYPE_THIS_POINTER:               pCtx->xprintf("this"); break;
        case D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW:      pCtx->xprintf("uav"); break;
        case D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY: pCtx->xprintf("tgsm"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID:            pCtx->xprintf("thread_id"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID:      pCtx->xprintf("thread_gid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP:   pCtx->xprintf("thread_idgid"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK:        pCtx->xprintf("in_covMask"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED: pCtx->xprintf("thread_id_flatGroup"); break;
        case D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID:       pCtx->xprintf("gs_instId"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL: pCtx->xprintf("out_depthGE"); break;
        case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:    pCtx->xprintf("out_depthLE"); break;
        case D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER:              pCtx->xprintf("cycleCounter"); break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK:       pCtx->xprintf("coverageMask"); break;
        case D3D10_SB_OPERAND_TYPE_RASTERIZER:                 pCtx->xprintf("rasterizer"); break;
        case D3D11_SB_OPERAND_TYPE_INNER_COVERAGE:             pCtx->xprintf("innerCoverage"); break;
        default:
            return E_FAIL;
    }
    return S_OK;
}

HRESULT PrintOperandD3D10(HLSLDisasm *pCtx, COperandBase *pOperand, BOOL bFloat, BOOL bAmbiguous)
{
    HRESULT hr = S_OK;

    if ( pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_NEG ||
         pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG )
    {
        pCtx->xprintf("-");
    }

    if ( pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
         pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG )
    {
        pCtx->xprintf("|");
    }

    hr = PrintRegister( pCtx,pOperand->m_Type );
    if ( FAILED( hr ) )
    {
        return hr;
    }

    if ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_IMMEDIATE32 &&
         pOperand->m_Type != D3D10_SB_OPERAND_TYPE_IMMEDIATE64 )
    {
        if ( pOperand->m_IndexDimension == 1 &&
             pOperand->m_Index[0].m_RelRegType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 )
        {
                pCtx->xprintf("%d", pOperand->m_Index[0].m_RegIndex);
        }
        else
        {
            //XXX I don't think this is right
            for ( UINT i = 0; i < (UINT)pOperand->m_IndexDimension && i < 3; i++ )
            {
                if ( pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32)
                {
                    if ( ( i == 0 ) &&
                         ( ( ( 1 == pOperand->m_IndexDimension ) ||
                             ( ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INPUT ) &&
                               ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_OUTPUT ) &&
                               ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT) &&
                               ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT)
                             )
                           ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_THIS_POINTER )
                         ) ||
                         ( ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INPUT )&&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_OUTPUT )&&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW) &&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_RESOURCE) &&
                           ( pOperand->m_Type != D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INTERFACE ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_THIS_POINTER ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_INPUT_CONTROL_POINT ) &&
                           ( pOperand->m_Type != D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT )
                         )
                       )
                    {
                        pCtx->xprintf("%d", pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf("[%d]", pOperand->m_Index[i].m_RegIndex);
                    }
                }

                if ( pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_RELATIVE ||
                     pOperand->m_IndexType[i] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE )
                {
                    pCtx->xprintf("[");

                    hr = PrintRegister( pCtx, pOperand->m_Index[i].m_RelRegType );
                    if ( FAILED( hr ) )
                    {
                        return hr;
                    }

                    if ( 2 == pOperand->m_Index[i].m_IndexDimension )
                    {
                        //reminder we should disallow D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER as indices
                        assert(pOperand->m_Index[i].m_RelRegType != D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

                        pCtx->xprintf("_%d_%d.", pOperand->m_Index[i].m_RelIndex1, pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(" + %d]", pOperand->m_Index[i].m_RegIndex );
                    }
                    else
                    {
                        pCtx->xprintf("%d.", pOperand->m_Index[i].m_RelIndex);
                        PrintComponent(pCtx, pOperand->m_Index[i].m_ComponentName);
                        pCtx->xprintf(" + %d]", pOperand->m_Index[i].m_RegIndex );
                    }
                }
            }
        }

        if ( pOperand->m_NumComponents == D3D10_SB_OPERAND_0_COMPONENT ||
             pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT )
        {
            //--
        }
        else if (pOperand->m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE)
        {
            if (pOperand->m_WriteMask != D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL)
            {
                pCtx->xprintf(".");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X)
                    pCtx->xprintf("x");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y)
                    pCtx->xprintf("y");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z)
                    pCtx->xprintf("z");
                if (pOperand->m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
                    pCtx->xprintf("w");
            }
        }
        else if (pOperand->m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE)
        {
            if ( ( D3D10_SB_4_COMPONENT_X != pOperand->m_Swizzle[0] ) ||
                 ( D3D10_SB_4_COMPONENT_Y != pOperand->m_Swizzle[1] ) ||
                 ( D3D10_SB_4_COMPONENT_Z != pOperand->m_Swizzle[2] ) ||
                 ( D3D10_SB_4_COMPONENT_W != pOperand->m_Swizzle[3] ) )
            {
                pCtx->xprintf(".");
                PrintComponent(pCtx,(D3D10_SB_4_COMPONENT_NAME)pOperand->m_Swizzle[0]);
                for(UINT i = 1; i < 4; ++i)
                {
                    for(UINT j = i; j < 4; ++j)
                    {
                        if ( pOperand->m_Swizzle[j] != pOperand->m_Swizzle[i-1] )
                        {
                            PrintComponent(pCtx,(D3D10_SB_4_COMPONENT_NAME)pOperand->m_Swizzle[i]);
                            break;
                        }
                    }
                }
            }
        }
        else if(pOperand->m_Type != D3D10_SB_OPERAND_TYPE_RESOURCE )
        {
            pCtx->xprintf(".");
            PrintComponent(pCtx, pOperand->m_ComponentName);
        }
    }
    else
    {
        pCtx->SetColor(COLOR_LITERAL);
        if (bAmbiguous)
        {
            if (pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            {
                UINT uVal = pOperand->m_Value[0] & 0x7F800000;
                if( pOperand->m_Value[0] == 0x80000000 )
                    bFloat = TRUE;
                else if(uVal == 0x7F800000 || uVal == 0)
                    bFloat = FALSE;
                else
                    bFloat = TRUE;

            }
            else
            {
                bFloat = FALSE;
                for( UINT i = 0; i < 4; i++ )
                {
                    UINT uVal = pOperand->m_Value[i] & 0x7F800000;
                    if( pOperand->m_Value[i] == 0x80000000 )
                        bFloat = TRUE;
                    if(uVal != 0x7F800000 && uVal != 0)
                        bFloat = TRUE;

                }
            }
        }

        if (bFloat)
        {
            if(pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
                pCtx->xprintf("(%f)",pOperand->m_Valuef[0]);
            else
                pCtx->xprintf("(%f, %f, %f, %f)", pOperand->m_Valuef[0], pOperand->m_Valuef[1], pOperand->m_Valuef[2], pOperand->m_Valuef[3]);
        }
        else
        {
            if(pOperand->m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            {
                if(abs((INT)pOperand->m_Value[0]) > 10000)
                    pCtx->xprintf("(0x%08x)",pOperand->m_Value[0]);
                else
                    pCtx->xprintf("(%d)",pOperand->m_Value[0]);
            }
            else
            {
                pCtx->xprintf("(");
                for(UINT i = 0;i < 4; i++)
                {
                    if(abs((INT)pOperand->m_Value[i]) > 10000)
                        pCtx->xprintf("0x%08x",pOperand->m_Value[i]);
                    else
                        pCtx->xprintf("%d",pOperand->m_Value[i]);

                    if(i != 3)
                        pCtx->xprintf(", ");
                }
                pCtx->xprintf(")");
            }
        }
        pCtx->UnsetColor();
    }

    if (pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
        pOperand->m_Modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG)
    {
        pCtx->xprintf("|");
    }

    if (pOperand->m_MinPrecision != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
    {
        switch(pOperand->m_MinPrecision)
        {
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
                pCtx->xprintf("[f16]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
                pCtx->xprintf("[f2_8]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
                pCtx->xprintf("[sint16]");
                break;
            case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
                pCtx->xprintf("[uint16]");
                break;
        }
    }

    return S_OK;
}

static HRESULT PrintName(HLSLDisasm *pCtx, UINT uName)
{
    switch(uName)
    {
        case D3D10_SB_NAME_UNDEFINED:                           pCtx->xprintf("undefined"); break;
        case D3D10_SB_NAME_CLIP_DISTANCE:                       pCtx->xprintf("clip_distance"); break;
        case D3D10_SB_NAME_CULL_DISTANCE:                       pCtx->xprintf("cull_distance"); break;
        case D3D10_SB_NAME_POSITION:                            pCtx->xprintf("position"); break;
        case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:           pCtx->xprintf("rendertarget_array_index"); break;
        case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:                pCtx->xprintf("viewport_array_index"); break;
        case D3D10_SB_NAME_VERTEX_ID:                           pCtx->xprintf("vertex_id"); break;
        case D3D10_SB_NAME_PRIMITIVE_ID:                        pCtx->xprintf("primitive_id"); break;
        case D3D10_SB_NAME_INSTANCE_ID:                         pCtx->xprintf("instance_id"); break;
        case D3D10_SB_NAME_IS_FRONT_FACE:                       pCtx->xprintf("is_front_face"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_ueq0"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_veq0"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_ueq1"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:   pCtx->xprintf("final_quad_veq1"); break;
        case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:      pCtx->xprintf("final_quad_uinside"); break;
        case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:      pCtx->xprintf("final_quad_vinside"); break;
        case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_ueq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_veq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:    pCtx->xprintf("final_tri_weq0"); break;
        case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:         pCtx->xprintf("final_tri_inside"); break;
        case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:        pCtx->xprintf("final_line_detail"); break;
        case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:       pCtx->xprintf("final_line_density"); break;
        default:
            return E_FAIL;
    }

    return S_OK;
}

HRESULT
DisasembleShader(const UINT * pShader, HLSLDisasm * pCtx)
{
    HRESULT hr = S_OK;
    CShaderCodeParser Parser;

    Parser.SetShader(pShader);
    CInstruction Instruction;

    INT uIndent = 0;

    // print out target
    D3D10_SB_TOKENIZED_PROGRAM_TYPE uShaderType = Parser.ShaderType();
    UINT uMajorVersion = Parser.ShaderMajorVersion();
    UINT uMinorVersion = Parser.ShaderMinorVersion();

    for (INT i = 0; i < uIndent; i++)
    {
        pCtx->xprintf("  ");
    }

    switch(uShaderType)
    {
        case D3D10_SB_PIXEL_SHADER:
            pCtx->xprintf("ps_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_VERTEX_SHADER:
            pCtx->xprintf("vs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D10_SB_GEOMETRY_SHADER:
            pCtx->xprintf("gs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_COMPUTE_SHADER:
            pCtx->xprintf("cs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_HULL_SHADER:
            pCtx->xprintf("hs_%d_%d", uMajorVersion, uMinorVersion);
            break;
        case D3D11_SB_DOMAIN_SHADER:
            pCtx->xprintf("ds_%d_%d", uMajorVersion, uMinorVersion);
            break;
        default:
            hr = E_FAIL;
            goto Cleanup;
    }

    pCtx->Flush(0);

    int line = 0;

    while (!Parser.EndOfShader())
    {
        IFC (Parser.ParseInstruction(&Instruction));

        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_ENDIF:
        case D3D10_SB_OPCODE_ENDLOOP:
        case D3D10_SB_OPCODE_ELSE:
        case D3D10_SB_OPCODE_ENDSWITCH:
              uIndent--;
              break;
        }

        for (INT i = 0; i < uIndent; i++)
        {
            pCtx->xprintf("  ");
        }

        //print opcode, validate first
        if (Instruction.m_OpCode >= D3D10_SB_NUM_OPCODES)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        if ((g_InstructionInfo[Instruction.m_OpCode].m_NumSrcOperands +
             g_InstructionInfo[Instruction.m_OpCode].m_NumDstOperands ) !=
             (int)Instruction.m_NumOperands)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        pCtx->SetColor(COLOR_KEYWORD);

        pCtx->xprintf("%s", g_InstructionInfo[Instruction.m_OpCode].m_Name);

        if (Instruction.m_bSaturate)
        {
            switch (Instruction.m_OpCode)
            {
            case D3D10_SB_OPCODE_DCL_INPUT:
            case D3D10_SB_OPCODE_DCL_OUTPUT:
            case D3D10_SB_OPCODE_DCL_INPUT_SGV:
            case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
            case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
            case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
            case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
            case D3D10_SB_OPCODE_DCL_INPUT_PS:
            case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            case D3D10_SB_OPCODE_DCL_SAMPLER:
            case D3D10_SB_OPCODE_DCL_RESOURCE:
            case D3D10_SB_OPCODE_DCL_INPUT_SIV:
            case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
            case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            case D3D10_SB_OPCODE_DCL_TEMPS:
            case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
            case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:

            case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
            case D3D11_SB_OPCODE_DCL_STREAM:
            case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
            case D3D11_SB_OPCODE_DCL_INTERFACE:

            case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
            case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
            case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
            case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
            case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
            case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
            case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
            case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:

            case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
            case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
            case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
            case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
            case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
                break;
            default:
                pCtx->xprintf("_sat");
            }
        }

        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_DISCARD:
        case D3D10_SB_OPCODE_IF:
        case D3D10_SB_OPCODE_CONTINUEC:
        case D3D10_SB_OPCODE_BREAKC:
        case D3D10_SB_OPCODE_CALLC:
        case D3D10_SB_OPCODE_RETC:
            switch (Instruction.m_Test)
            {
            case D3D10_SB_INSTRUCTION_TEST_ZERO:
                pCtx->xprintf("_z");
                break;

            case D3D10_SB_INSTRUCTION_TEST_NONZERO:
                pCtx->xprintf("_nz");
                break;
            }
            break;
        }

        pCtx->UnsetColor();

        BOOL bOperands = TRUE;

        if (Instruction.m_bExtended)
        {
            switch (Instruction.m_OpCodeEx[0])
            {
            case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS:
                pCtx->xprintf("_O(%i,%i,%i) ",
                    Instruction.m_TexelOffset[0],
                    Instruction.m_TexelOffset[1],
                    Instruction.m_TexelOffset[2]);
                break;
            }
        }

        // special things to do
        switch (Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_RESINFO:
            switch (Instruction.m_ResInfoReturnType)
            {
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_RCPFLOAT:
                    pCtx->xprintf("_rcpfloat ");
                    break;
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_UINT:
                    pCtx->xprintf("_uint ");
                    break;
                default:
                case D3D10_SB_RESINFO_INSTRUCTION_RETURN_FLOAT:
                    pCtx->xprintf(" ");
                    break;
            }
            break;
        case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
            {
                int count = 0;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_X)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Y)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_Z)
                    count++;
                if(Instruction.m_IndexableTempDecl.Mask & D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
                    count++;

                pCtx->xprintf(" x%i[%i], %d", Instruction.m_IndexableTempDecl.IndexableTempNumber,
                                                 Instruction.m_IndexableTempDecl.NumRegisters,
                                                 count);
                bOperands = FALSE;
                break;
            }
        case D3D10_SB_OPCODE_DCL_INDEX_RANGE:

            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));

            pCtx->xprintf(" %i", Instruction.m_IndexRangeDecl.RegCount);
            bOperands = FALSE;

            break;
        case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
            {
                bool hasFlags = false;
                pCtx->xprintf(" ");
                if( Instruction.m_GlobalFlagsDecl.Flags & D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED )
                {
                    hasFlags = true;
                    pCtx->xprintf("refactoringAllowed" );
                }

                if( Instruction.m_GlobalFlagsDecl.Flags & D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL )
                {
                    hasFlags = true;
                    pCtx->xprintf("forceEarlyDepthStencil " );
                }

                if(!hasFlags)
                {
                    pCtx->xprintf("none" );
                }
                bOperands = FALSE;
            }
            break;
        case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
            switch(Instruction.m_OutputTopologyDecl.Topology)
            {
            case D3D10_PRIMITIVE_TOPOLOGY_POINTLIST: pCtx->xprintf(" pointlist");break;
            case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP: pCtx->xprintf(" linestrip"); break;
            case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: pCtx->xprintf(" triangestrip"); break;
            case D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED: pCtx->xprintf(" undefined"); break;
            default: pCtx->xprintf(" ???"); break;
            }
            break;
        case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
            pCtx->xprintf(" %u", Instruction.m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount); break;
        case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
            switch(Instruction.m_InputPrimitiveDecl.Primitive)
            {
            case D3D10_PRIMITIVE_POINT: pCtx->xprintf(" point"); break;
            case D3D10_PRIMITIVE_LINE: pCtx->xprintf(" line"); break;
            case D3D10_PRIMITIVE_TRIANGLE: pCtx->xprintf(" triangle"); break;
            case D3D10_PRIMITIVE_LINE_ADJ: pCtx->xprintf(" lineadj"); break;
            case D3D10_PRIMITIVE_TRIANGLE_ADJ: pCtx->xprintf(" triangleadj"); break;
            case D3D11_SB_PRIMITIVE_1_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint1"); break;
            case D3D11_SB_PRIMITIVE_2_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint2"); break;
            case D3D11_SB_PRIMITIVE_3_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint3"); break;
            case D3D11_SB_PRIMITIVE_4_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint4"); break;
            case D3D11_SB_PRIMITIVE_5_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint5"); break;
            case D3D11_SB_PRIMITIVE_6_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint6"); break;
            case D3D11_SB_PRIMITIVE_7_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint7"); break;
            case D3D11_SB_PRIMITIVE_8_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint8"); break;
            case D3D11_SB_PRIMITIVE_9_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint9"); break;
            case D3D11_SB_PRIMITIVE_10_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint10"); break;
            case D3D11_SB_PRIMITIVE_11_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint11"); break;
            case D3D11_SB_PRIMITIVE_12_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint12"); break;
            case D3D11_SB_PRIMITIVE_13_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint13"); break;
            case D3D11_SB_PRIMITIVE_14_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint14"); break;
            case D3D11_SB_PRIMITIVE_15_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint15"); break;
            case D3D11_SB_PRIMITIVE_16_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint16"); break;
            case D3D11_SB_PRIMITIVE_17_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint17"); break;
            case D3D11_SB_PRIMITIVE_18_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint18"); break;
            case D3D11_SB_PRIMITIVE_19_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint19"); break;
            case D3D11_SB_PRIMITIVE_20_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint20"); break;
            case D3D11_SB_PRIMITIVE_21_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint21"); break;
            case D3D11_SB_PRIMITIVE_22_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint22"); break;
            case D3D11_SB_PRIMITIVE_23_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint23"); break;
            case D3D11_SB_PRIMITIVE_24_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint24"); break;
            case D3D11_SB_PRIMITIVE_25_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint25"); break;
            case D3D11_SB_PRIMITIVE_26_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint26"); break;
            case D3D11_SB_PRIMITIVE_27_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint27"); break;
            case D3D11_SB_PRIMITIVE_28_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint28"); break;
            case D3D11_SB_PRIMITIVE_29_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint29"); break;
            case D3D11_SB_PRIMITIVE_30_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint30"); break;
            case D3D11_SB_PRIMITIVE_31_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint31"); break;
            case D3D11_SB_PRIMITIVE_32_CONTROL_POINT_PATCH: pCtx->xprintf(" controlPoint32"); break;
            case D3D10_PRIMITIVE_UNDEFINED: pCtx->xprintf(" undefined"); break;
            default: pCtx->xprintf(" ???");
            }
            break;

        case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
                pCtx->xprintf(" %i", Instruction.m_HSForkPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
                pCtx->xprintf(" %i", Instruction.m_HSJoinPhaseDecl.InstanceCount);
                bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
            switch(Instruction.m_TessellatorDomainDecl.TessellatorDomain)
            {
            case D3D11_SB_TESSELLATOR_DOMAIN_ISOLINE: pCtx->xprintf(" isoline"); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_TRI: pCtx->xprintf(" tri"); break;
            case D3D11_SB_TESSELLATOR_DOMAIN_QUAD: pCtx->xprintf(" quad"); break;
                default: pCtx->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
            switch(Instruction.m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive)
            {
            case D3D11_SB_TESSELLATOR_OUTPUT_POINT: pCtx->xprintf(" point"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_LINE: pCtx->xprintf(" line"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CCW: pCtx->xprintf(" tri_ccw"); break;
            case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CW: pCtx->xprintf(" tri_cw"); break;
                default: pCtx->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
            switch(Instruction.m_TessellatorPartitioningDecl.TessellatorPartitioning)
            {
            case D3D11_SB_TESSELLATOR_PARTITIONING_POW2: pCtx->xprintf(" pow2"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_INTEGER: pCtx->xprintf(" int"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN: pCtx->xprintf(" fract-even"); break;
            case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD: pCtx->xprintf(" fract-odd"); break;
                default: pCtx->xprintf(" ???");
            }
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
            pCtx->xprintf(" %i", Instruction.m_InputControlPointCountDecl.InputControlPointCount);
            bOperands = false;
            break;
        case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
            pCtx->xprintf(" %i", Instruction.m_OutputControlPointCountDecl.OutputControlPointCount);
            bOperands = false;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                pCtx->xprintf(", ");
                PrintName(pCtx, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDeclSIV.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: pCtx->xprintf(", undefined"); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  pCtx->xprintf(", constant"); break;
                case D3D10_SB_INTERPOLATION_LINEAR: pCtx->xprintf(", linear"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: pCtx->xprintf(", linear centroid"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: pCtx->xprintf(", linear noperspective"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: pCtx->xprintf(", linear noperspective centroid"); break;
                default : pCtx->xprintf(" ???"); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS:

            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));

            if(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV == Instruction.m_OpCode)
            {
                pCtx->xprintf(", ");
                PrintName(pCtx, Instruction.m_OutputDeclSIV.Name);
            }

            switch(Instruction.m_InputPSDecl.InterpolationMode)
            {
                case D3D10_SB_INTERPOLATION_UNDEFINED: pCtx->xprintf(", undefined"); break;
                case D3D10_SB_INTERPOLATION_CONSTANT:  pCtx->xprintf(", constant"); break;
                case D3D10_SB_INTERPOLATION_LINEAR: pCtx->xprintf(", linear"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_CENTROID: pCtx->xprintf(", linear centroid"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE: pCtx->xprintf(", linear noperspective"); break;
                case D3D10_SB_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID: pCtx->xprintf(", linear noperspective centroid"); break;
                default : pCtx->xprintf(" ???"); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            pCtx->xprintf(", ");

            PrintName(pCtx, Instruction.m_OutputDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            pCtx->xprintf(", ");

            PrintName(pCtx, Instruction.m_OutputDeclSIV.Name);
            bOperands = FALSE;
            break;

       case D3D10_SB_OPCODE_DCL_INPUT_SIV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            pCtx->xprintf(", ");

            PrintName(pCtx, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            pCtx->xprintf(", ");
            PrintName(pCtx, Instruction.m_InputPSDeclSGV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_INPUT_SGV:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            pCtx->xprintf(", ");
            PrintName(pCtx, Instruction.m_InputDeclSIV.Name);
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf("cb%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                pCtx->xprintf("cb%d[%d]", Instruction.m_Operands[0].m_Index[0].m_RegIndex, Instruction.m_Operands[0].m_Index[1].m_RegIndex);
            }

            if (Instruction.m_ResourceDecl.CBInfo.AccessPattern == D3D10_SB_CONSTANT_BUFFER_DYNAMIC_INDEXED)
                pCtx->xprintf(", dynamicIndexed");
            else
                pCtx->xprintf(", immediateIndexed");

            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_DCL_TEMPS:
            pCtx->xprintf(" %d",  Instruction.m_TempsDecl.NumTemps);
            break;

        case D3D10_SB_OPCODE_DCL_RESOURCE:

            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch( Instruction.m_ResourceDecl.SRVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: pCtx->xprintf(", buffer"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: pCtx->xprintf(", texture1d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: pCtx->xprintf(", texture2d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: pCtx->xprintf(", texture2dms"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: pCtx->xprintf(", texture3d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: pCtx->xprintf(", texturecube"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: pCtx->xprintf(", texture1darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: pCtx->xprintf(", texture2darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: pCtx->xprintf(", texture2dmsarray"); break;
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS) ||
                (Instruction.m_ResourceDecl.SRVInfo.Dimension == D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY))
            {
                pCtx->xprintf("[%d]", Instruction.m_ResourceDecl.SRVInfo.SampleCount);
            }

            if ((Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[1]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[2]) &&
                (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0] == Instruction.m_ResourceDecl.SRVInfo.ReturnType[3]))
            {
                switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[0])
                {
                case D3D10_SB_RETURN_TYPE_MIXED: pCtx->xprintf(", mixed"); break;
                case D3D10_SB_RETURN_TYPE_UNORM: pCtx->xprintf(", unorm"); break;
                case D3D10_SB_RETURN_TYPE_SNORM: pCtx->xprintf(", snorm"); break;
                case D3D10_SB_RETURN_TYPE_SINT:  pCtx->xprintf(", sint"); break;
                case D3D10_SB_RETURN_TYPE_UINT:  pCtx->xprintf(", uint"); break;
                case D3D10_SB_RETURN_TYPE_FLOAT: pCtx->xprintf(", float"); break;
                }
            }
            else
            {
                pCtx->xprintf(", (");
                for(UINT i = 0;i < 4; i++)
                {
                    switch (Instruction.m_ResourceDecl.SRVInfo.ReturnType[i])
                    {
                    case D3D10_SB_RETURN_TYPE_MIXED: pCtx->xprintf(" mixed"); break;
                    case D3D10_SB_RETURN_TYPE_UNORM: pCtx->xprintf(" unorm"); break;
                    case D3D10_SB_RETURN_TYPE_SNORM: pCtx->xprintf(" snorm"); break;
                    case D3D10_SB_RETURN_TYPE_SINT:  pCtx->xprintf(" sint"); break;
                    case D3D10_SB_RETURN_TYPE_UINT:  pCtx->xprintf(" uint"); break;
                    case D3D10_SB_RETURN_TYPE_FLOAT: pCtx->xprintf(" float"); break;
                    }
                    if(i != 3)
                        pCtx->xprintf(",");
                }
                pCtx->xprintf(" )");
            }
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf("t%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            pCtx->xprintf(", stride=%u", Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf(" uav%d[%d-%d], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf(" uav%d[%d-%d], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            pCtx->xprintf(", stride=%u", Instruction.m_ResourceDecl.UAVInfo.Stride);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
            pCtx->xprintf(" ");

            if (Instruction.m_Operands[0].OperandIndexDimension() == D3D10_SB_OPERAND_INDEX_3D)
            {
                pCtx->xprintf(" uav%d[%u-%u], space %d", Instruction.m_ResourceDecl.Space.SetIdx, Instruction.m_ResourceDecl.Space.MinShaderRegister, Instruction.m_ResourceDecl.Space.MaxShaderRegister,
                    Instruction.m_ResourceDecl.Space.Space);
            }
            else
            {
                IFC(PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], TRUE, FALSE));
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Dimension)
            {
                case D3D10_SB_RESOURCE_DIMENSION_BUFFER: pCtx->xprintf(", buffer"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D: pCtx->xprintf(", texture1d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D: pCtx->xprintf(", texture2d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS: pCtx->xprintf(", texture2dms"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D: pCtx->xprintf(", texture3d"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE: pCtx->xprintf(", texturecube"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY: pCtx->xprintf(", texture1darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY: pCtx->xprintf(", texture2darray"); break;
                case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY: pCtx->xprintf(", texture2dmsarray"); break;
            }

            switch (Instruction.m_ResourceDecl.UAVInfo.Type)
            {
            case D3D10_SB_RETURN_TYPE_MIXED: pCtx->xprintf(", mixed"); break;
            case D3D10_SB_RETURN_TYPE_UNORM: pCtx->xprintf(", unorm"); break;
            case D3D10_SB_RETURN_TYPE_SNORM: pCtx->xprintf(", snorm"); break;
            case D3D10_SB_RETURN_TYPE_SINT:  pCtx->xprintf(", sint"); break;
            case D3D10_SB_RETURN_TYPE_UINT:  pCtx->xprintf(", uint"); break;
            case D3D10_SB_RETURN_TYPE_FLOAT: pCtx->xprintf(", float"); break;
            }
            bOperands = FALSE;
            break;

        case D3D10_SB_OPCODE_CUSTOMDATA:
            {
                switch (Instruction.m_CustomData.Type )
                {
                    case D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER:
                    {
                        pCtx->SetColor(COLOR_KEYWORD);
                        pCtx->xprintf("dcl_immediateConstantBuffer");
                        pCtx->UnsetColor();
                        pCtx->xprintf(" { ");
                        for (UINT uNumVecs = 0; uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16;)
                        {
                            pCtx->xprintf("{ ");
                            for(UINT i = 0;i < 4; i++)
                            {
                                UINT uVal;
                                float *pfVal = (float*)&uVal;
                                uVal = ((UINT*)Instruction.m_CustomData.pData)[uNumVecs*4 + i];
                                UINT uMasked = uVal & 0x7F800000;
                                if(uMasked == 0x7F800000 || uMasked == 0)
                                {
                                    if(abs((INT)uVal) > 10000)
                                        pCtx->xprintf("0x%08x",uVal);
                                    else
                                        pCtx->xprintf("%d",(INT)uVal);
                                } else
                                {
                                    pCtx->xprintf("%f",*pfVal);
                                }

                                if(i != 3)
                                    pCtx->xprintf(", ");
                            }
                            pCtx->xprintf("}");
                            uNumVecs++;
                            if (uNumVecs < Instruction.m_CustomData.DataSizeInBytes / 16)
                            {
                                pCtx->xprintf(",\n                              ");
                            }
                        }
                        pCtx->xprintf(" }");
                    }
                    break;

                    case D3D10_SB_CUSTOMDATA_COMMENT:
                        pCtx->xprintf(" %d bytes of comment custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_DEBUGINFO:
                        pCtx->xprintf(" %d bytes of debuginfo custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D10_SB_CUSTOMDATA_OPAQUE:
                        pCtx->xprintf(" %d bytes of opaque custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    case D3D11_SB_CUSTOMDATA_SHADER_MESSAGE:
                        pCtx->xprintf(" %d bytes of shader message custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;

                    default:
                        pCtx->xprintf(" %d bytes of unknown custom data\n", Instruction.m_CustomData.DataSizeInBytes);
                        break;
                }
            }
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
            pCtx->xprintf(" fb%u", Instruction.m_FunctionBodyIdx);
            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
            pCtx->xprintf(" ft%u = {", Instruction.m_FunctionTable.FtIdx);
            for(UINT i = 0; i < Instruction.m_FunctionTable.FbCount; i++)
            {
                pCtx->xprintf("fb%u", Instruction.m_FunctionTable.pFbStartToken[i]);
                if(i < Instruction.m_FunctionTable.FbCount-1) pCtx->xprintf(", ");
            }
            pCtx->xprintf("}");

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_DCL_INTERFACE:
            pCtx->xprintf(" fp%u[%u][%u] = {", Instruction.m_InterfaceTable.FpIdx,
                Instruction.m_InterfaceTable.FpArraySize, Instruction.m_InterfaceTable.NumCallSites);
            for(UINT i = 0; i < Instruction.m_InterfaceTable.FtCount; i++)
            {
                pCtx->xprintf("ft%u", Instruction.m_InterfaceTable.pFtStartToken[i]);
                if(i < Instruction.m_InterfaceTable.FtCount-1) pCtx->xprintf(", ");
            }
            pCtx->xprintf("}");

            bOperands = FALSE;
            break;

        case D3D11_SB_OPCODE_INTERFACE_CALL:
            pCtx->xprintf(" ");
            IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[0], false, false));
            pCtx->xprintf("[%u]", Instruction.m_InterfaceCallSiteIdx);

            bOperands = FALSE;
            break;

        default:
            pCtx->xprintf(" ");
            break;
        }

        if(bOperands)
        {
            BOOL bFloat;
            BOOL bAmbiguous = FALSE;
            switch(Instruction.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD          :
            case D3D10_SB_OPCODE_DERIV_RTX    :
            case D3D10_SB_OPCODE_DERIV_RTY    :
            case D3D10_SB_OPCODE_DIV          :
            case D3D10_SB_OPCODE_DP2          :
            case D3D10_SB_OPCODE_DP3          :
            case D3D10_SB_OPCODE_DP4          :
            case D3D10_SB_OPCODE_EQ           :
            case D3D10_SB_OPCODE_EXP          :
            case D3D10_SB_OPCODE_FRC          :
            case D3D10_SB_OPCODE_FTOI         :
            case D3D10_SB_OPCODE_FTOU         :
            case D3D10_SB_OPCODE_GE           :
//            case D3D10_SB_OPCODE_LABEL        :
            case D3D10_SB_OPCODE_LOG          :
            case D3D10_SB_OPCODE_LT           :
            case D3D10_SB_OPCODE_MAD          :
            case D3D10_SB_OPCODE_MIN          :
            case D3D10_SB_OPCODE_MAX          :
            case D3D10_SB_OPCODE_MUL          :
            case D3D10_SB_OPCODE_NE           :
            case D3D10_SB_OPCODE_ROUND_NE     :
            case D3D10_SB_OPCODE_ROUND_NI     :
            case D3D10_SB_OPCODE_ROUND_PI     :
            case D3D10_SB_OPCODE_ROUND_Z      :
            case D3D10_SB_OPCODE_RSQ          :
            case D3D10_SB_OPCODE_SAMPLE       :
            case D3D10_SB_OPCODE_SAMPLE_B     :
            case D3D10_SB_OPCODE_SAMPLE_C     :
            case D3D10_SB_OPCODE_SAMPLE_C_LZ  :
            case D3D10_SB_OPCODE_SAMPLE_L     :
            case D3D10_SB_OPCODE_SAMPLE_D     :
            case D3D10_SB_OPCODE_SQRT         :
//            case D3D10_SB_OPCODE_SUB          :
            case D3D10_SB_OPCODE_SINCOS       :
                bFloat = TRUE;
                break;

            case D3D10_SB_OPCODE_MOV          :

            case D3D10_SB_OPCODE_MOVC         :
                bAmbiguous = TRUE;
                bFloat = FALSE;
                break;

            default:
                bFloat = FALSE;
                break;
            }
            for(UINT i = 0; i < Instruction.m_NumOperands;i++)
            {

                IFC (PrintOperandD3D10(pCtx, &Instruction.m_Operands[i], bFloat, bAmbiguous));

                if(i != Instruction.m_NumOperands-1)
                    pCtx->xprintf(", ");

            }
        }

        switch(Instruction.m_OpCode)
        {
        case D3D10_SB_OPCODE_IF:
        case D3D10_SB_OPCODE_LOOP:
        case D3D10_SB_OPCODE_ELSE:
        case D3D10_SB_OPCODE_SWITCH:
              uIndent++;
              break;
        case D3D10_SB_OPCODE_DCL_SAMPLER:
            switch(Instruction.m_ResourceDecl.SamplerInfo.SamplerMode)
            {
                case D3D10_SB_SAMPLER_MODE_DEFAULT:
                    pCtx->xprintf(", mode_default");
                    break;
                case D3D10_SB_SAMPLER_MODE_COMPARISON:
                    pCtx->xprintf(", mode_comparison");
                    break;
                case D3D10_SB_SAMPLER_MODE_MONO:
                    pCtx->xprintf(", mode_mono");
                    break;
            }
            break;
        }

        pCtx->Flush(++line);
    }

Cleanup:
    RRETURN(hr);
}


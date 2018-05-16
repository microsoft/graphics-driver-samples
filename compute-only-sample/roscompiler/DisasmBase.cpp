#include "precomp.h"
#include "roscompiler.h"

BaseDisasm::BaseDisasm()
{
    m_cSize = 0;
    m_cSizeMax = 0;
    m_pBuf = NULL;
    m_bColorCode = FALSE;
    m_pFile = NULL;
    m_pCustomCtx = NULL;
    m_pStrPrinter = NULL;
}

BaseDisasm::~BaseDisasm()
{
    delete[] m_pBuf;
}

void BaseDisasm::SetColor(WORD wColor)
{
    const TCHAR* x_rgszFontColor[4] =
    {
        TEXT("a0a0a0"), // COLOR_COMMENT   0//(FOREGROUND_INTENSITY)
        TEXT("ffff40"), // COLOR_KEYWORD   1//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
        TEXT("e0e0e0"), // COLOR_TEXT      2//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
        TEXT("00ffff")  // COLOR_LITERAL   3//(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
    };

    if (m_bColorCode)
    {
        xprintf(TEXT("<font color = \"#%s\">"), x_rgszFontColor[wColor]);
    }
}

void BaseDisasm::UnsetColor()
{
    if (m_bColorCode)
    {
        xprintf(TEXT("</font>"));
    }
}

void BaseDisasm::xprintf(const TCHAR *pStr, ...)
{
    TCHAR sz[512];
    va_list ap;
    va_start(ap, pStr);
    _vstprintf_s(sz, _countof(sz), pStr, ap);
    va_end(ap);

    xaddstring(sz);
}

void BaseDisasm::Flush(int Line)
{
    if (!EnsureSize(1)) return;
    m_pBuf[m_cSize] = TEXT('\0');
    if (m_pStrPrinter)
    {
        (m_pStrPrinter)(m_pFile, m_pBuf, Line, m_pCustomCtx);
    }
    else
    {
        OutputDebugString(m_pBuf);
        OutputDebugString(TEXT("\n"));
    }
    m_cSize = 0;
}

void BaseDisasm::xaddstring(const TCHAR* sz)
{
    const size_t cSize = _tcslen(sz);
    if (!EnsureSize(cSize)) return;
    memcpy(&m_pBuf[m_cSize], sz, cSize * sizeof(TCHAR));
    m_cSize += cSize;
}

bool BaseDisasm::EnsureSize(const size_t cbSize)
{
    if (m_cSizeMax < m_cSize + cbSize)
    {
        const size_t kBufInc = 8 * 1024;
        BYTE *pNewBuf = new BYTE[(m_cSizeMax + cbSize + kBufInc) * sizeof(TCHAR)];
        if (pNewBuf == NULL)
        {
            return false;
        }

        memcpy(pNewBuf, m_pBuf, m_cSize * sizeof(TCHAR));
        delete[] m_pBuf;
        m_pBuf = (TCHAR*) pNewBuf;
        m_cSizeMax = m_cSizeMax + cbSize + kBufInc;
    }

    return true;
}

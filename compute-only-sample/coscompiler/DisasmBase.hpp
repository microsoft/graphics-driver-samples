#pragma once

#define COLOR_COMMENT   0//(FOREGROUND_INTENSITY)
#define COLOR_KEYWORD   1//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_TEXT      2//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_LITERAL   3//(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

typedef void (fnPrinter)(void *pFile, const TCHAR* szStr, int Line, void* m_pCustomCtx);

class BaseDisasm
{
public:
    BaseDisasm();
    ~BaseDisasm();

    void SetCustomCtx(void *pCustomCtx) { m_pCustomCtx = pCustomCtx; }
    void SetFile(void * pFile) { m_pFile = pFile; }
    void SetPrinter(fnPrinter *pStrPrinter) { m_pStrPrinter = pStrPrinter; }

    void SetColor(WORD wColor);
    void UnsetColor();
    void xprintf(const TCHAR *pStr, ...);
    void Flush(int Line);

protected:
    size_t m_cSize;
    size_t m_cSizeMax;
    TCHAR* m_pBuf;
    bool   m_bColorCode;
    void*  m_pFile;
    void*  m_pCustomCtx;
    fnPrinter* m_pStrPrinter;

    void xaddstring(const TCHAR *sz);
    bool EnsureSize(const size_t cSize);
};
#include <roscompiler.h>

BaseDisasm::BaseDisasm()
{
	m_cbSize = 0;
	m_cbSizeMax = 0;
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
	const char* x_rgszFontColor[4] =
	{
		"a0a0a0", // COLOR_COMMENT   0//(FOREGROUND_INTENSITY)
		"ffff40", // COLOR_KEYWORD   1//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
		"e0e0e0", // COLOR_TEXT      2//(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
		"00ffff"  // COLOR_LITERAL   3//(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
	};

	if (m_bColorCode)
	{
		xprintf("<font color = \"#%s\">", x_rgszFontColor[wColor]);
	}
}

void BaseDisasm::UnsetColor()
{
	if (m_bColorCode)
	{
		xprintf("</font>");
	}
}

void BaseDisasm::xprintf(LPCSTR pStr, ...)
{
	char sz[512];
	va_list ap;
	va_start(ap, pStr);
	vsprintf_s(sz, sizeof(sz), pStr, ap);
	va_end(ap);

	xaddstring(sz);
}

void BaseDisasm::Flush(int Line)
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

void BaseDisasm::xaddstring(LPCSTR sz)
{
	const size_t cbSize = strlen(sz);
	if (!EnsureSize(cbSize)) return;
	memcpy(&m_pBuf[m_cbSize], sz, cbSize);
	m_cbSize += cbSize;
}

bool BaseDisasm::EnsureSize(const size_t cbSize)
{
	if (m_cbSizeMax < m_cbSize + cbSize)
	{
		const size_t kBufInc = 8 * 1024;
		char *pNewBuf = new char[m_cbSizeMax + cbSize + kBufInc];
		if (pNewBuf == NULL)
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

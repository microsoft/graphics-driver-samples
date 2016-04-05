#pragma once

#include "d3dumddi_.h"

class RosCompilerException : public std::exception
{
public:

    RosCompilerException(HRESULT hr, char *fileName, unsigned lineNumber, char *Message = NULL ) :
        std::exception(Message), 
        m_hr(hr),
        m_fileName(fileName),
        m_lineNumber(lineNumber),
        m_Message(Message)
    {
        // do nothing
    }

    HRESULT GetError()
    {
        return m_hr;
    }

private:

    HRESULT m_hr;
    char *m_fileName;
    unsigned m_lineNumber;
    char *m_Message;
};

#if VC4

#define VC4_THROW( hr )  if (FAILED(hr)) throw RosCompilerException( hr, __FILE__, __LINE__, "RosCompilerException: Error" )

#if DBG
#define VC4_ASSERT( _exp ) assert(_exp)
#else
#define VC4_ASSERT( _exp ) ( (_exp) ? true : throw RosCompilerException( E_NOTIMPL, __FILE__, __LINE__, "RosCompilerException: Assert" ) ); __assume( _exp )
#endif // DBG

#endif // VC4

#pragma once

#define assert( _exp ) ( ( _exp ) ? true : (\
    OutputDebugStringW( L"Assertion Failed\n" ),\
    OutputDebugStringW( #_exp L"\n" ),\
    DebugBreak() ) ); __assume( _exp )
 
class CosUmdException : public std::exception
{
public:

    CosUmdException(HRESULT hr = E_FAIL) : std::exception("CosUmdException") , m_hr(hr)
    {
        // do nothing
    }

    HRESULT m_hr;

};

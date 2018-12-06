#pragma once

#define assert( _exp ) ( ( _exp ) ? true : (\
    OutputDebugStringW( L"Assertion Failed\n" ),\
    OutputDebugStringW( #_exp L"\n" ),\
    DebugBreak() ) ); __assume( _exp )
 
class RosUmdException : public std::exception
{
public:

    RosUmdException(HRESULT hr = E_FAIL) : std::exception("RosUmdException") , m_hr(hr)
    {
        // do nothing
    }

    HRESULT m_hr;

};

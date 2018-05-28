#pragma once

#include "CosUmd12.h"

class CosUmd12Descriptor
{
public:

    enum Type { kUnknown };

    CosUmd12Descriptor(Type type, void * ptr)
    {
        m_type = type;
        m_ptr = ptr;
    }

private:

    
    Type    m_type;
    void *  m_ptr;

};

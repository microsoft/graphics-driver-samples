#pragma once

//
// Header used to include <d3dumddi.h>
//
// This header takes care of the various dependencies needed by d3dumddi.h
//
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef ROUND_TO_PAGES
#define ROUND_TO_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#endif

//
// Must include windef.h for POINT used by wingdi.h
//

#include <windef.h>

//
// Must include wingdi.h to get definition of PALETTEENTRY used by d3d10umddi.h
//

#include <wingdi.h>

//
// Must define NTSTATUS due to d3dkmddi.h which is included by d3d10umddi.h
//

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;


//
// Must turn off warning 4201 due to dxgiddi.h which is included as part of d3d10umddi.h
//
#pragma warning(push)
#pragma warning(disable : 4201)


//
// Must include KM WDK include path due to inclusion of d3dkmthk.h in d3d10umddi.h
//


#include <d3d10umddi.h>
#include <d3d11.h>

//
// Pop off our warning state
//
#pragma warning(pop)


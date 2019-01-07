#pragma once

#ifndef _STDINT
typedef unsigned char      uint8_t;
typedef unsigned short	   uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef signed char        int8_t;
typedef short			   int16_t;
typedef int				   int32_t;
typedef long long		   int64_t;
#endif

#define kVpuSegmentUndefined 0
#define kVpuSegmentGlobal 1
#define kVpuSegmentGroupShared 2
#define kVpuSegmentLocal 3
#define kVpuSegmentGlobalConstant 4

typedef int32_t VpuSegment;

#define kVpuResourceClassSRV 0
#define kVpuResourceClassUAV 1
#define kVpuResourceClassCBV 2
#define kVpuResourceClassSampler 3

typedef int32_t VpuResourceClass;

typedef struct {
    int8_t * m_base;
    int32_t m_elementSize;
} VpuResourceDescriptor;

#define kVpuMaxUAVs 4

typedef struct {
    int32_t m_id;
    VpuResourceDescriptor m_uavs[kVpuMaxUAVs];
} VpuThreadLocalStorage;

typedef VpuResourceDescriptor * VpuResourceHandle;

struct VpuRelocation {
	uint32_t	m_type;
	uint64_t	m_fixupOffset; // offset from base to fixup
	uint64_t    m_referenceOffset; // offset from base to reference
};

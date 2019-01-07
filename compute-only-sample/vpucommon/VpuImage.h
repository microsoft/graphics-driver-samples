#pragma once

#include "Vpu.h"

struct VpuImageHeader
{
public:

	uint64_t m_codeSize;
	uint64_t m_entryOffset;
	uint64_t m_tlsSize;
	uint16_t m_relocationCount;

	uint64_t GetSerializationSize(void)
	{
		return sizeof(VpuImageHeader) + (m_relocationCount * sizeof(VpuRelocation)) + m_codeSize;
	}

	uint64_t GetTlsOffset(void)
	{
		return AlignUp(m_codeSize, kVpuImageCodeAlignment);
	}

	uint64_t GetImageSize()
	{
		return AlignUp(m_codeSize, kVpuImageCodeAlignment) +
			AlignUp(m_tlsSize, kVpuImagekDataAlignment);
	}

	uint64_t GetEntryOffset()
	{
		return m_entryOffset;
	}

	bool Load(uint8_t * base, uint64_t size)
	{
		if (size < GetImageSize())
			return false;

		VpuRelocation * relocations = (VpuRelocation *)(this + 1);
		uint8_t * code = (uint8_t *)&relocations[m_relocationCount];

		memset(base, 0, size);
		memcpy(base, code, m_codeSize);

		for (int i = 0; i < m_relocationCount; i++)
			if (!ApplyRelocation(base, &relocations[i]))
				return false;

		return true;
	}

	const uint32_t kIMAGE_REL_I386_REL32 = 0x0014;  // PC-relative 32-bit reference to the symbols virtual address
	const uint32_t kIMAGE_REL_AMD64_REL32 = 0x0004;  // 32-bit relative address from byte following reloc
	const uint32_t kIMAGE_REL_I386_DIR32 = 0x0006;  // Direct 32-bit reference to the symbols virtual address

	bool ApplyRelocation(uint8_t * base, VpuRelocation * relocation)
	{
		uint32_t * loc = (uint32_t *)(base + relocation->m_fixupOffset);

		if (relocation->m_type == kIMAGE_REL_I386_REL32 || relocation->m_type == kIMAGE_REL_AMD64_REL32)
			*loc = (uint32_t) (relocation->m_referenceOffset - relocation->m_fixupOffset - 4);
#ifdef _M_IX86
		else if (relocation.m_type == kIMAGE_REL_I386_DIR32)
			*loc = relocation.m_referenceOffset + (uint32_t)base;
#endif
		else
			return false;
		return true;
	}

private:

	static const uint32_t kVpuImageCodeAlignment = 4096;
	static const uint32_t kVpuImagekDataAlignment = 4096;

	uint64_t AlignUp(uint64_t value, uint64_t alignment)
	{
		uint64_t mask = alignment - 1;
		return (value + mask) & ~mask;
	}

};

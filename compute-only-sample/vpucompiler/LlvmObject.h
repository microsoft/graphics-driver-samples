#pragma once

#include "Vpu.h"

#include "llvm/Support/Error.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/ELF.h"

#include <map>
#include <vector>

struct VpuObjRelocation {
	uint32_t	m_type;
	uint64_t	m_fixupOffset; // offset from base to fixup
	std::string m_symbolName;
};

class VpuObject
{
public:

	VpuObject() { m_loaded = false; }
	~VpuObject() { assert(!m_loaded);  }

	bool Open(const char * filePath);
	void Close();

	bool GetSymbolValue(const char * name, uint64_t & value)
	{
		assert(m_loaded);
		if (m_symbolTable.count(name) == 0)
			return false;
		value = m_symbolTable[name];
		return true;
	}

	const std::vector<VpuObjRelocation> & GetRelocations(void)
	{
		assert(m_loaded);
		return m_relocations;
	}

	uint64_t GetCodeSize() { assert(m_loaded); return m_code.getSize(); }
	bool GetCode(uint8_t * buffer, uint64_t size);

private:

	bool LoadSymbolTable();
	bool LoadRelocations();

	bool m_loaded;

	llvm::object::OwningBinary<llvm::object::Binary> m_binary;
	llvm::object::ObjectFile * m_obj;
	const llvm::object::COFFObjectFile * m_coff;
	llvm::object::SectionRef m_code;

	std::map<std::string, uint64_t> m_symbolTable;
	std::vector<VpuObjRelocation> m_relocations;

};
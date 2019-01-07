#include "LlvmObject.h"

#include <windows.h>

using namespace llvm;
using namespace object;

bool VpuObject::LoadSymbolTable(void)
{
	for (uint32_t i = 0; i < m_coff->getNumberOfSymbols(); i++)
	{
		Expected<COFFSymbolRef> symbolOrErr = m_coff->getSymbol(i);

		if (!symbolOrErr)
			return false;

		auto symbol = *symbolOrErr;

		StringRef testName;
		if (m_coff->getSymbolName(symbol, testName))
			return false;

		if (m_symbolTable.count(testName) != 0)
			return false;

		m_symbolTable[testName] = symbol.getValue();

		i += symbol.getNumberOfAuxSymbols();
	}

	return true;
}

bool VpuObject::LoadRelocations(void)
{
	for (auto r : m_code.relocations()) {
		VpuObjRelocation objrel;

		objrel.m_fixupOffset = r.getOffset();
		objrel.m_type = r.getType();

		auto symbol = r.getSymbol();

		Expected<SymbolRef::Type> typeOrErr = symbol->getType();
		if (!typeOrErr) return false;
		SymbolRef::Type type = *typeOrErr;

		Expected<StringRef> nameOrErr = symbol->getName();

		if (!nameOrErr) return false;

		objrel.m_symbolName = *nameOrErr;

		m_relocations.push_back(objrel);
	}

	return true;
}

bool VpuObject::Open(const char * filePath)
{
	StringRef file(filePath);

	Expected<OwningBinary<Binary>> binaryOrErr = createBinary(file);

	if (!binaryOrErr)
		return false;

	m_binary = std::move(*binaryOrErr);

	Binary * binary = m_binary.getBinary();
	m_obj = dyn_cast<ObjectFile>(binary);

	if (m_obj == nullptr)
		return false;

	m_coff = dyn_cast<const COFFObjectFile>(m_obj);

	if (m_coff == nullptr)
		return false;

	if (!LoadSymbolTable())
		return false;

	for (const SectionRef &section : m_obj->sections())
	{
		StringRef name;
		if (section.getName(name))
			return false;

		if (name == ".text") {
			m_code = section;
		}
	}

	if (m_code == SectionRef())
		return false;

	if (!LoadRelocations())
		return false;

	m_loaded = true;
	return true;
}

bool VpuObject::GetCode(uint8_t * buffer, uint64_t size)
{
	StringRef contents;

	if (m_code.getContents(contents))
		return false;

	if (contents.size() > size)
		return false;

	memset(buffer, 0, size);
	memcpy(buffer, contents.data(), contents.size());

	return true;
}

void VpuObject::Close()
{
	m_symbolTable.clear();
	m_loaded = false;
}
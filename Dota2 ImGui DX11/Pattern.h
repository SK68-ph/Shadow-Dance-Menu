#pragma once
#include "PatternUtil.h"


class Pattern
{

protected:

	uintptr_t address_;
	uint8_t* voida;
public:
	uint8_t* PatternScan(void* module, const char* signature);
	Pattern(void* getmodulehandlea, const char* sigida);

	uintptr_t GetAbsoluteAddress(const uintptr_t address, const size_t offset, const size_t size)
	{
		return address + *reinterpret_cast<PDWORD>(address + offset) + size;
	}
	Pattern& getAbsAddress(const size_t offset, const size_t size, const bool dRef);
	uintptr_t GetAdress();
};


#pragma once
#include "includes.h"
#include "IAppSystem.h"

namespace utilities {
	std::uint8_t* PatternScan(void* module, const char* signature);
	std::uintptr_t ReadMultiLevelPointer(uintptr_t ptr, std::vector<uint32_t>& offsets);
	bool isPtrReadable(uintptr_t base);
	void* GetInterface(const char* dllname, const char* interfacename);

}
#pragma once
#include "includes.h"
#include "IAppSystem.h"

namespace utilities {
	std::uint8_t* pattern_scan(const char* module_name, const char* signature) noexcept;
	std::uintptr_t ReadMultiLevelPointer(const uintptr_t ptr, const std::vector<uint32_t>& offsets);
	bool isPtrReadable(uintptr_t base);
	void* GetInterface(const char* dllname, const char* interfacename);
}
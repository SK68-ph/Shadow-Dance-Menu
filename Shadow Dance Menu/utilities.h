#pragma once
#include "includes.h"

namespace utilities {
	std::uint8_t* PatternScan(void* module, const char* signature);
	std::uintptr_t ReadMultiLevelPointer(uintptr_t ptr, std::vector<uint32_t>& offsets);
	bool validateAddr(uintptr_t base);
}
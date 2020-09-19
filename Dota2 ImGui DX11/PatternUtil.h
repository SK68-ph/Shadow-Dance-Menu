#pragma once
#include <Windows.h>
#include <vector>
#include <math.h>

#define INLINE __forceinline 

namespace Util {

	template
		<typename T>
		T GetVirtual(void* object, int index) {
		return (*(T**)object)[index];
	}
	template <typename T>
	T GetFunc(uintptr_t adress, DWORD index) {
		return *(T*)((uintptr_t)adress + index);
	}

	template <size_t index, typename ReturnType = void, typename... Args>
	__forceinline ReturnType CallVirtual(void* instance, Args&&... args) {
		using Fn_t = ReturnType(__fastcall*)(void* instance, Args&&... args);
		auto func = (*static_cast<Fn_t**>(instance))[index];
		return func(instance, std::forward<Args>(args)...);
	}

	INLINE DWORD_PTR GetAbsoluteAddresss(DWORD_PTR pInstruction, int iOffset, int iSize)
	{
		DWORD offset = *(DWORD*)(pInstruction + iOffset);
		DWORD_PTR ptr = pInstruction + offset + iSize;
		if (offset > 0xF0000000)
		{
			ptr -= 0x100000000;
		}
		return ptr;
	}

}


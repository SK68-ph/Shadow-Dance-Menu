#pragma once
#include <string.h>

#define cc const char*
#define u64 unsigned long long

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef void* (*InstantiateInterfaceFn) ();


inline void**& getvtable(void* inst, size_t offset = 0)
{
	return *reinterpret_cast<void***>((size_t)inst + offset);
}

inline const void** getvtable(const void* inst, size_t offset = 0)
{
	return *reinterpret_cast<const void***>((size_t)inst + offset);
}

template <typename T, typename ... args_t>
inline constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
{
	using VirtualFn = T(__thiscall*)(void*, decltype(argList)...);
	return (*(VirtualFn**)thisptr)[nIndex](thisptr, argList...);
}

template< typename T >
T getvfunc(void* vTable, int iIndex)
{
	return (*(T**)vTable)[iIndex];
}

template<typename T, typename ...Args>
constexpr auto callVirtualMethod(void* classBase, int index, Args... args) noexcept
{
	return ((*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[index])(classBase, args...);
}


void* GetInterface(const char* zmodule, const char* name)
{
	CreateInterfaceFn CreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(GetModuleHandle(zmodule), "CreateInterface"));

	int returnCode = 0;
	void* Interface = CreateInterface(name, &returnCode);

	return Interface;
}

inline uintptr_t GetAbsoluteAddress(uintptr_t instruction_ptr, int offset, int size)
{
	return instruction_ptr + *reinterpret_cast<uint32_t*>(instruction_ptr + offset) + size;
};

template<typename T, typename Z>
bool StringsMatch(T a, Z b) {
	return !strcmp((cc)a, (cc)b);
}
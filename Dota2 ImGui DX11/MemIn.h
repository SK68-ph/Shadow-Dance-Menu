//Every address parameter is in the context of the virtual address space of the current process unless explicity stated otherwise.
#pragma once

#ifndef MEMIN_H
#define MEMIN_H

#include <Windows.h>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <atomic>
#include <TlHelp32.h>
#include <thread>
#include <algorithm>

//CPU STATES(for use in the saveCpuStateMask parameter on the Hook() function)
//Even though the upper portions of YMM0-15 and ZMM0-15 are volatile, there's no mechanism to save them. 
#define GPR 0x01
#define FLAGS 0x02
#define XMMX 0x04

enum class SCAN_BOUNDARIES
{
	RANGE,
	MODULE,
	ALL_MODULES
}; 

struct ScanBoundaries
{
	const SCAN_BOUNDARIES scanBoundaries;
	union
	{
		struct { uintptr_t start, end; };
		const TCHAR* const moduleName;
	};

	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries, const uintptr_t start, const uintptr_t end);
	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries, const TCHAR* const moduleName);
	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries);
};

enum class SCAN_TYPE
{
	EXACT,
	BETWEEN,
	GREATER_THAN,
	LESS_THAN,
	UNCHANGED,
	CHANGED,
	INCREASED,
	INCREASED_BY,
	DECREASED,
	DECREASED_BY,
	UNKNOWN
};

enum class VALUE_TYPE
{
	ONE_BYTE    = 0x01,
	TWO_BYTES   = 0x02,
	FOUR_BYTES  = 0x04,
	EIGTH_BYTES = 0x08,
	FLOAT       = 0x10,
	DOUBLE      = 0x20,
	ALL         = (ONE_BYTE | TWO_BYTES | FOUR_BYTES | EIGTH_BYTES | FLOAT | DOUBLE)
};

enum class FLOAT_ROUNDING
{
	NONE,
	ROUND,
	TRUNCATE
};

template<typename T>
struct Scan
{
	SCAN_TYPE scanType;

	T value, value2;
	FLOAT_ROUNDING floatRounding;

	Scan(const SCAN_TYPE scanType, T value, const FLOAT_ROUNDING floatRounding = FLOAT_ROUNDING::NONE)
		: scanType(scanType),
		value(value),
		value2(T()),
		floatRounding(floatRounding) {}

	Scan(const SCAN_TYPE scanType, T value, T value2, const FLOAT_ROUNDING floatRounding = FLOAT_ROUNDING::NONE)
		: scanType(scanType),
		value(value),
		value2(value2),
		floatRounding(floatRounding) {}

	Scan(const SCAN_TYPE scanType)
		: scanType(scanType),
		value(T()),
		value2(T()),
		floatRounding(FLOAT_ROUNDING::NONE) {}
};

template<typename T>
struct Value
{
	uintptr_t address;
	T value;

	Value(uintptr_t address, T& value)
		: address(address),
		value(value) {}

	//For use in std::sort()
	bool operator<(const Value& other) const { return address < other.address; }
};

struct AOB
{
	char* aob;
	size_t size;

	AOB() : aob(nullptr), size(0) {}

	AOB(const char* _aob) : aob(const_cast<char*>(_aob)), size(0) {}

	AOB(const AOB& other)
		:size(other.size)
	{
		if (size)
		{
			aob = new char[size];
			memcpy(aob, other.aob, size);
		}
		else
			aob = other.aob;
	}

	AOB(const char* _aob, size_t size)
		: size(size)
	{
		aob = new char[size];
		memcpy(aob, _aob, size);
	}

	~AOB()
	{
		if(size)
			delete[] aob;
	}
};

template<typename T>
struct ThreadData
{
	std::thread thread;
	std::vector<Value<T>> values;
};

class MemIn
{
	class ProtectRegion
	{
		uintptr_t m_Address;
		const size_t m_Size;
		DWORD m_Protection;
		BOOL m_Success;
	public:
		ProtectRegion(const uintptr_t address, const SIZE_T size, const bool m_Protect = true);
		~ProtectRegion();

		inline bool Success() { return m_Success; }
	};

	struct NopStruct
	{
		std::unique_ptr<uint8_t[]> buffer;
		SIZE_T size = 0;
	};

	struct HookStruct
	{
		uintptr_t buffer = 0;
		uint8_t bufferSize = 0;
		uint8_t numReplacedBytes = 0;
		bool useCodeCaveAsMemory = true;
		uint8_t codeCaveNullByte = 0;
	};

	//Store addresses/bytes which the user nopped so they can be restored later with Patch()
	static std::unordered_map<uintptr_t, NopStruct> m_Nops;

	static std::unordered_map<uintptr_t, HookStruct> m_Hooks;

	static std::unordered_map<uintptr_t, size_t> m_Pages;
public:
	//Returns a copy of the data at 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be read from.
	template <typename T>
	static inline T Read(const uintptr_t address)
	{
		T t;
		Read(address, &t, sizeof(T));
		return t;
	}

	//Copies 'size' bytes from 'address' to 'buffer'.
	//Parameters:
	//  address [in]  The address where the bytes will be copied from.
	//  buffer  [out] The buffer where the bytes will be copied to.
	//  size    [in]  The number of bytes to be copied.
	static void Read(const uintptr_t address, void* const buffer, const SIZE_T size);

	//Copies 'value' to 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  value   [in] The value where the bytes will be copied from.
	template <typename T>
	static inline bool Write(const uintptr_t address, const T& value) { return Write(address, &value, sizeof(T)); }

	//Copies 'size' bytes from 'buffer' to 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  buffer  [in] The buffer where the bytes will be copied from.
	//  size    [in] The number of bytes to be copied.
	static bool Write(const uintptr_t address, const void* const buffer, const SIZE_T size);

	//Patches 'address' with 'size' bytes stored on 'buffer'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  buffer  [in] The buffer where the bytes will be copied from.
	//  size    [in] The number of bytes to be copied.
	static bool Patch(const uintptr_t address, const char* bytes, const size_t size);

	//Writes 'size' 0x90 bytes at address.
	//Parameters:
	//  address   [in] The address where the bytes will be nopped.
	//  size      [in] The number of bytes to be written.
	//  saveBytes [in] If true, save the original bytes located at 'address'
	//                 where they can be later restored by calling Restore().
	static bool Nop(const uintptr_t address, const size_t size, const bool saveBytes = true);

	//Restores the bytes that were nopped at 'address'.
	//Parameters:
	//  address   [in] The address where the bytes will be restored.
	static bool Restore(const uintptr_t address);

	//Copies 'size' bytes from 'sourceAddress' to 'destinationAddress'.
	//Parameters:
	//  destinationAddress [in] The destination buffer's address.
	//  sourceAddress      [in] The souce buffer's address.
	//  size               [in] The number of bytes to be copied.
	static bool Copy(const uintptr_t destinationAddress, const uintptr_t sourceAddress, const size_t size);

	//Sets 'size' 'value' bytes at 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be written to.
	//  value   [in] The byte to be set.
	//  size    [in] The nmber of bytes to be set.
	static bool Set(const uintptr_t address, const int value, const size_t size);

	//Compares the first 'size' bytes of 'address1' and 'address2'.
	//Parameters:
	//  address1 [in] the address where the first buffer is located.
	//  address2 [in] the address where the second buffer is located.
	//  size     [in] The number of bytes to be compared.
	static bool Compare(const uintptr_t address1, const uintptr_t address2, const size_t size);

	//Calculates the MD5 hash of a memory region of the attached process.
	//Parameters:
	//  address [in]  The address where the hash will be calculated.
	//  size    [in]  The size of the region.
	//  outHash [out] A buffer capable of holding a MD5 hash which is 16 bytes.
	static bool HashMD5(const uintptr_t address, const size_t size, uint8_t* const outHash);

	//Scans the address space according to 'scanBoundaries' for a pattern & mask.
	//Parameters:
	//  pattern        [in] A buffer containing the pattern. An example of a
	//                      pattern is "\x68\xAB\x00\x00\x00\x00\x4F\x90\x00\x08".
	//  mask           [in] A string that specifies how the pattern should be 
	//                      interpreted. If mask[i] is equal to '?', then the
	//                      byte pattern[i] is ignored. A example of a mask is
	//                      "xx????xxxx".
	//  scanBoundaries [in] See definition of the ScanBoundaries class.
	//  protect        [in] Specifies a mask of memory protection constants
	//                      which defines what memory regions will be scanned.
	//                      The default value(-1) specifies that pages with any
	//                      protection between 'start' and 'end' should be scanned.
	//  numThreads     [in] The number of threads to be used. Thr default argument
	//                      uses the number of CPU cores as the number of threads.
	//  firstMatch     [in] If true, the address returned(if any) is guaranteed to
	//                      be the first match(i.e. the lowest address on the virtual
	//                      address space that is a match) according to scanBoundaries.
	static uintptr_t PatternScan(const char* const pattern, const char* const mask, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = -1, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false);
	
	//Scans the address space according to 'scanBoundaries' for an AOB.
	//Parameters:
	//  AOB            [in] The array of bytes(AOB) in string form. To specify
	//                      a byte that should be ignore use the '?' character.
	//                      An example of AOB is "68 AB ?? ?? ?? ?? 4F 90 00 08".
	//  scanBoundaries [in] See definition of the ScanBoundaries class.
	//  protect        [in] Specifies a mask of memory protection constants
	//                      which defines what memory regions will be scanned.
	//                      The default value(-1) specifies that pages with any
	//                      protection between 'start' and 'end' should be scanned.
	//  patternSize    [out] A pointer to a variable that receives the size of the
	//                       size of the pattern in bytes. This parameter can be NULL.
	//  numThreads     [in] The number of threads to be used. Thr default argument
	//                      uses the number of CPU cores as the number of threads.
	//  firstMatch     [in] If true, the address returned(if any) is guaranteed to
	//                      be the first match(i.e. the lowest address on the virtual
	//                      address space that is a match) according to scanBoundaries.
	static uintptr_t AOBScan(const char* const AOB, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = -1, size_t* const patternSize = nullptr, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false);

	//Reads a multilevel pointer.
	//Parameters:
	//  base    [in] The base address.
	//  offsets [in] A vector specifying the offsets.
	static uintptr_t ReadMultiLevelPointer(const uintptr_t base, const std::vector<uint32_t>& offsets);

	//Hooks an address.
	//Parameters:
	//  address             [in]  The address to be hooked.
	//  callback            [in]  The callback to be executed when the CPU executes 'address'.
	//  trampoline          [out] An optional pointer to a variable that receives the address
	//                            of the trampoline. The trampoline contains the original replaced
	//                            instructions of the 'address' and a jump back to 'address'.
	//  saveCpuStateMask    [in]  A mask containing a bitwise OR combination of one or more of
	//                            the following macros: GPR(general purpose registers),
	//                            FLAGS(eflags/rflags), XMMX(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5).
	//                            Push the CPU above states to the stack before executing callback.
	//                            You should use this parameter if you perform a mid function hook.
	//                            By default no CPU state is saved.
	static bool Hook(const uintptr_t address, const void* const callback, uintptr_t* const trampoline = nullptr, const DWORD saveCpuStateMask = 0);

	//Removes a previously placed hook at 'address'.
	//Parameters:
	//  address [in] The address to be unhooked.
	static bool Unhook(const uintptr_t address);

	//Scans the address space according to 'scanBoundaries' for a nullByte.
	//Parameters:
	//  size           [in]  The size of the code cave.
	//  nullByte       [in]  The byte of the code cave. If -1 is specified,
	//                       the null byte is any byte, that is, FindCodeCave()
	//                       will return any sequence of the same byte.
	//  scanBoundaries [in]  See definition of the ScanBoundaries class.
	//  codeCaveSize   [out] If not NULL, the variable pointed by this argument
	//                       receives the size of the code cave found. If no code
	//                       cave is found, 0(zero) is set.
	//  protection     [in]  Specifies a mask of memory protection constants
	//                       which defines what memory regions will be scanned.
	//                       The default value(-1) specifies that pages with any
	//                       protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]  The number of threads to be used. Thr default argument
	//                       uses the number of CPU cores as the number of threads.
	//  firstMatch     [in]  If true, the address returned(if any) is guaranteed to
	//                       be the first match(i.e. the lowest address on the virtual
	//                       address space that is a match) according to scanBoundaries.
	static uintptr_t FindCodeCave(const size_t size, const uint32_t nullByte = 0x00, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), size_t* const codeCaveSize = nullptr, const DWORD protection = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false);

	//Scans the address space according to 'scanBoundaries' for nullBytes.
	//Parameters:
	//  size           [in]  The size of the code cave.
	//  nullBytes      [in]  The byte of the code cave.
	//  pNullByte      [in]  If a codecave is found and pNullByte is not NULL,
	//                       the byte that the codecave contains is written to
	//                       the variable pointed by pNullByte.
	//  scanBoundaries [in]  See definition of the ScanBoundaries class.
	//  codeCaveSize   [out] If not NULL, the variable pointed by this argument
	//                       receives the size of the code cave found. If no code
	//                       cave is found, 0(zero) is set.
	//  protection     [in]  Specifies a mask of memory protection constants
	//                       which defines what memory regions will be scanned.
	//                       The default value(-1) specifies that pages with any
	//                       protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]  The number of threads to be used. Thr default argument
	//                       uses the number of CPU cores as the number of threads.
	//  firstMatch     [in]  If true, the address returned(if any) is guaranteed to
	//                       be the first match(i.e. the lowest address on the virtual
	//                       address space that is a match) according to scanBoundaries.
	static uintptr_t FindCodeCaveBatch(const size_t size, const std::vector<uint8_t>& nullBytes, uint8_t* const pNullByte = nullptr, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), size_t* const codeCaveSize = nullptr, const DWORD protection = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false);

	//TODO: add string as type.
	//  values         [in/out]	The values. If there're no elements on the set it's 
	//                          considered to be the 'first scan', otherwise it's a
	//                          'next scan'.
	//  scan           [in]     A reference to a Scan struct which specifies how the
	//                          scan should be performed.
	//  alignment      [in]     The address will only be scanned if it's divisible
	//                          by the alignment value.
	//  scanBoundaries [in]     See definition of the ScanBoundaries class.
	//  protection     [in]     Specifies a mask of memory protection constants
	//                          which defines what memory regions will be scanned.
	//                          The default value(-1) specifies that pages with any
	//                          protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]     The number of threads to be used. Thr default argument
	//                          uses the number of CPU cores as the number of threads.
	template<typename T>
	static bool ValueScan(std::vector<Value<T>>& values, Scan<T>& scan, const size_t alignment = 4, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = PAGE_READONLY | PAGE_READWRITE, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()))
	{
		if (alignment == 0)
			return false;

		uintptr_t start = 0, end = 0;
		if (values.empty())
		{
			switch (scanBoundaries.scanBoundaries)
			{
			case SCAN_BOUNDARIES::RANGE:
				start = scanBoundaries.start, end = scanBoundaries.end;
				break;
			case SCAN_BOUNDARIES::MODULE:
				DWORD moduleSize;
				if (!(start = GetModuleBase(scanBoundaries.moduleName, &moduleSize)))
					return 0;
				end = start + moduleSize;
				break;
			case SCAN_BOUNDARIES::ALL_MODULES:
			{
				struct ValueScanInfo
				{
					std::vector<Value<T>>& values;
					Scan<T>& scan;
					const size_t alignment;
					const DWORD protection;
					const size_t numThreads;
					bool success;
				};

				ValueScanInfo vsi = { values, scan, alignment, protect, numThreads, true };

				EnumModules(GetCurrentProcessId(),
					[](MODULEENTRY32& me, void* param)
					{
						ValueScanInfo* vsi = static_cast<ValueScanInfo*>(param);
						std::vector<Value<T>> values;

						if (!(vsi->success = MemIn::ValueScan(values, vsi->scan, vsi->alignment, ScanBoundaries(SCAN_BOUNDARIES::RANGE, reinterpret_cast<uintptr_t>(me.modBaseAddr), reinterpret_cast<uintptr_t>(me.modBaseAddr) + me.modBaseSize), vsi->protection, vsi->numThreads)))
							return false;

						vsi->values.insert(vsi->values.end(), values.begin(), values.end());
						return true;
					}, &vsi);

				return vsi.success;
			}
			default:
				return 0;
			}
		}
		size_t chunkSize = (end - start) / numThreads;
		std::vector<ThreadData<T>> threads(numThreads);
		
		if (!values.empty())
		{
			size_t quota = values.size() / numThreads;
			for (size_t i = 0; i < threads.size(); i++)
				threads[i].values.insert(threads[i].values.end(), values.begin() + i * quota, values.begin() + (i + 1) * quota);

			values.clear();
		}

		for (size_t i = 0; i < numThreads; i++)
			threads[i].thread = std::thread(&MemIn::ValueScanImpl<T>, std::ref(threads[i].values), std::ref(scan), alignment, start + chunkSize * i, start + chunkSize * (static_cast<size_t>(i) + 1), protect);

		for (auto& thread : threads)
			thread.thread.join();

		size_t newCapacity = 0;
		for (auto& thread : threads)
			newCapacity += thread.values.size();

		values.reserve(newCapacity);

		for (auto& thread : threads)
			values.insert(values.end(), thread.values.begin(), thread.values.end());

		return true;
	}

	//Returns the PID of the specified process.
	//Parameters:
	//  processName [in] The name of the process.
	static DWORD GetProcessIdByName(const TCHAR* const processName);

	//Returns the PID of the window's owner.
	//Parameters:
	//  windowName [in] The window's title. If NULL, all window 
	//                  names match.
	//  className  [in] The class name. If NULL, any window title
	//                  matching windowName is considered.
	static DWORD GetProcessIdByWindow(const TCHAR* const windowName, const TCHAR* const className = nullptr);

	//If moduleName is NULL, GetModuleBase() returns the base of the module created by the file used to create the process specified (.exe file)
	//Returns a module's base address on the attached process.
	//Parameters:
	//  moduleName  [in]  The name of the module.
	//  pModuleSize [out] An optional pointer that if provided, receives the size of the module.
	static uintptr_t GetModuleBase(const TCHAR* const moduleName = nullptr, DWORD* const pModuleSize = nullptr);

	//Returns the size of first parsed instruction on the buffer at 'address'.
	//Parameters:
	//  address [in] The address of the buffer containing instruction.
	static size_t GetInstructionLength(const void* const address);

	//Loops through all modules of a process passing its information to a callback function.
	//Parameters:
	//  processId [in] The PID of the process which the modules will be looped.
	//  callback  [in] A function pointer to a callback function.
	//  param     [in] An optional pointer to be passed to the callback.
	static void EnumModules(const DWORD processId, bool (*callback)(MODULEENTRY32& me, void* param), void* param);

	//Converts an AOB in string form into pattern & mask form.
	//Parameters:
	//  AOB     [in]  The array of bytes(AOB) in string form.
	//  pattern [out] The string that will receive the pattern.
	//  mask    [out] The string that will receive th mask.
	static void AOBToPattern(const char* const AOB, std::string& pattern, std::string& mask);

	//Converts a pattern and mask into an AOB.
	//Parameters:
	//  pattern [in]  The pattern.
	//  mask    [in]  The mask.
	//  AOB     [out] The array of bytes(AOB) in string form.
	static void PatternToAOB(const char* const pattern, const char* const mask, std::string& AOB);
private:
	static void PatternScanImpl(std::atomic<uintptr_t>& returnValue, const uint8_t* const pattern, const char* const mask, uintptr_t start, const uintptr_t end, const DWORD protect, const bool firstMatch);

	static void FindCodeCaveImpl(std::atomic<uintptr_t>& returnValue, const size_t size, uintptr_t start, const uintptr_t end, const DWORD protect, const bool firstMatch);

	template<typename T>
	struct ValueScanRegionData
	{
		std::vector<Value<T>>& values;
		Scan<T>& scan;
		size_t alignment;
		uintptr_t start, end;
		Value<T>* value;

		ValueScanRegionData(std::vector<Value<T>>& values, Scan<T>& scan, size_t alignment, uintptr_t start, uintptr_t end, Value<T>* value = nullptr)
			: values(values),
			scan(scan),
			alignment(alignment),
			start(start),
			end(end),
			value(value) {}
	};

	template<typename T>
	static T ProcessValue(ValueScanRegionData<T>& vsrd) { return *reinterpret_cast<const T*>(vsrd.start); }

	template<>
	static float ProcessValue(ValueScanRegionData<float>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return roundf(*reinterpret_cast<float*>(vsrd.start));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return truncf(*reinterpret_cast<float*>(vsrd.start));
		else
			return *reinterpret_cast<float*>(vsrd.start);
	}

	template<>
	static double ProcessValue(ValueScanRegionData<double>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return round(*reinterpret_cast<double*>(vsrd.start));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return trunc(*reinterpret_cast<double*>(vsrd.start));
		else
			return *reinterpret_cast<double*>(vsrd.start);
	}

	template<typename T>
	static void ValueScanRegionEquals(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) == vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.start, *reinterpret_cast<T*>(vsrd.start));
		}
	}
	
	template<typename T>
	static void ValueScanRegionGreater(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) > vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.start, *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionLess(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) < vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.start, *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionBetween(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) > vsrd.scan.value && *reinterpret_cast<T*>(vsrd.start) < vsrd.scan.value2)
				vsrd.values.emplace_back(vsrd.start, *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionUnknown(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
			vsrd.values.emplace_back(vsrd.start, *reinterpret_cast<T*>(vsrd.start));
	}

	template<typename T>
	static T NextScanProcessValue(ValueScanRegionData<T>& vsrd) { return *reinterpret_cast<const T*>(vsrd.value->address); }

	template<>
	static float NextScanProcessValue(ValueScanRegionData<float>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return roundf(*reinterpret_cast<float*>(vsrd.value->address));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return truncf(*reinterpret_cast<float*>(vsrd.value->address));
		else
			return *reinterpret_cast<float*>(vsrd.value->address);
	}

	template<>
	static double NextScanProcessValue(ValueScanRegionData<double>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return round(*reinterpret_cast<double*>(vsrd.value->address));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return trunc(*reinterpret_cast<double*>(vsrd.value->address));
		else
			return *reinterpret_cast<double*>(vsrd.value->address);
	}

	template<typename T>
	static bool NextValueScanEquals(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) == vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanGreater(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) > vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanLess(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) < vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanBetween(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value > vsrd.scan.value && value < vsrd.scan.value2;
	}

	template<typename T>
	static bool NextValueScanIncreased(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) > vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanIncreasedBy(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value == value + vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanDecreased(ValueScanRegionData<T>& vsrd)
	{
		return *reinterpret_cast<T*>(vsrd.value->address) < vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanDecreasedBy(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value == value - vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanChanged(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) != vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanUnchanged(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) == vsrd.value->value;
	}

	template<typename T>
	static void PerformFloatRounding(Scan<T>& scan) {}

	template<>
	static void PerformFloatRounding(Scan<float>& scan)
	{
		if (scan.floatRounding == FLOAT_ROUNDING::ROUND)
			scan.value = roundf(scan.value), scan.value2 = roundf(scan.value2);
		else if (scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			scan.value = truncf(scan.value), scan.value2 = truncf(scan.value2);
	}

	template<>
	static void PerformFloatRounding(Scan<double>& scan)
	{
		if (scan.floatRounding == FLOAT_ROUNDING::ROUND)
			scan.value = round(scan.value), scan.value2 = round(scan.value2);
		else if (scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			scan.value = trunc(scan.value), scan.value2 = trunc(scan.value2);
	}

	template<typename T>
	static void ValueScanImpl(std::vector<Value<T>>& values, Scan<T>& scan, const size_t alignment, const uintptr_t start, const uintptr_t end, const DWORD protect)
	{
		PerformFloatRounding<T>(scan);

		ValueScanRegionData<T> vsrd(values, scan, alignment, start, end);
		MEMORY_BASIC_INFORMATION mbi;
		if (values.empty())
		{
			void(*firstScan)(ValueScanRegionData<T>&);
			switch (scan.scanType)
			{
			case SCAN_TYPE::EXACT: firstScan = ValueScanRegionEquals; break;
			case SCAN_TYPE::GREATER_THAN: firstScan = ValueScanRegionGreater; break;
			case SCAN_TYPE::LESS_THAN: firstScan = ValueScanRegionLess; break;
			case SCAN_TYPE::BETWEEN: firstScan = ValueScanRegionBetween; break;
			case SCAN_TYPE::UNKNOWN: firstScan = ValueScanRegionUnknown; break;
			default: return;
			}

			while (vsrd.start < end && VirtualQuery(reinterpret_cast<LPCVOID>(vsrd.start), &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				if (!(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) && (mbi.Protect & protect))
				{
					const uintptr_t pageEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
					vsrd.end = end > pageEnd ? pageEnd : end;

					if (((vsrd.end - vsrd.start) % sizeof(T)) > 0)
						vsrd.end -= ((vsrd.end - vsrd.start) % sizeof(T));

					firstScan(vsrd);
				}

				vsrd.start = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
			}
		}
		else
		{
			bool(*nextScan)(ValueScanRegionData<T>&);
			switch (scan.scanType)
			{
			case SCAN_TYPE::EXACT: nextScan = NextValueScanEquals; break;
			case SCAN_TYPE::GREATER_THAN: nextScan = NextValueScanGreater; break;
			case SCAN_TYPE::LESS_THAN: nextScan = NextValueScanLess; break;
			case SCAN_TYPE::BETWEEN: nextScan = NextValueScanBetween; break;
			case SCAN_TYPE::INCREASED: nextScan = NextValueScanIncreased; break;
			case SCAN_TYPE::INCREASED_BY: nextScan = NextValueScanIncreasedBy; break;
			case SCAN_TYPE::DECREASED: nextScan = NextValueScanDecreased; break;
			case SCAN_TYPE::DECREASED_BY: nextScan = NextValueScanDecreasedBy; break;
			case SCAN_TYPE::CHANGED: nextScan = NextValueScanChanged; break;
			case SCAN_TYPE::UNCHANGED: nextScan = NextValueScanUnchanged; break;
			default: return;
			}

			std::sort(values.begin(), values.end());

			uintptr_t regionEnd = 0;

			for (size_t i = 0; i < values.size();)
			{
				if (values[i].address < regionEnd || VirtualQuery(reinterpret_cast<void*>(values[i].address), &mbi, sizeof(MEMORY_BASIC_INFORMATION)) && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) && (regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize))
				{
					vsrd.value = &values[i];
					if (nextScan(vsrd))
					{
						i++;
						continue;
					}
				}
				
				values.erase(values.begin() + i);
			}
		}
	}

	template<>
	static void ValueScanImpl(std::vector<Value<AOB>>& values, Scan<AOB>& scan, const size_t alignment, const uintptr_t start, const uintptr_t end, const DWORD protect)
	{
		if (values.empty())
		{
			uintptr_t aobStart = start;
			size_t patternSize;
			
			while ((aobStart = AOBScan(scan.value.aob, ScanBoundaries(SCAN_BOUNDARIES::RANGE, aobStart, end), protect, &patternSize, 1)))
			{
				if (std::find_if(std::begin(values), std::end(values), [aobStart](Value<AOB>& value) { return aobStart == value.address; } ) == std::end(values))
				{
					AOB aob(reinterpret_cast<const char*>(aobStart), patternSize);
					Value<AOB> value(aobStart++, aob);
					values.push_back(value);
				}
			}
		}
		else
		{
			std::sort(values.begin(), values.end());
			
			uintptr_t regionEnd = 0;
			
			MEMORY_BASIC_INFORMATION mbi;
			for (size_t i = 0; i < values.size();)
			{
				if (values[i].address < regionEnd || VirtualQuery(reinterpret_cast<void*>(values[i].address), &mbi, sizeof(MEMORY_BASIC_INFORMATION)) && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) && (regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize))
				{	
					bool unchanged = true;
					for (size_t j = 0; j < values[i].value.size; j++)
					{
						if (reinterpret_cast<uint8_t*>(values[i].address)[j] != values[i].value.aob[j])
						{
							unchanged = false;
							break;
						}
					}

					if (unchanged)
					{
						i++;
						continue;
					}
				}
			
				values.erase(values.begin() + i);
			}
		}
	}

};

#endif // MEMIN_H
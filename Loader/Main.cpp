#include "injector.h"


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

bool IsCorrectTargetArchitecture(HANDLE hProc) {
	BOOL bTarget = FALSE;
	if (!IsWow64Process(hProc, &bTarget)) {
		printf("Can't confirm target process architecture: 0x%X\n", GetLastError());
		return false;
	}

	BOOL bHost = FALSE;
	IsWow64Process(GetCurrentProcess(), &bHost);

	return (bTarget == bHost);
}

DWORD GetProcId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));

		}
	}
	CloseHandle(hSnap);
	return procId;
}

wchar_t* getCurrentDir() {
	wchar_t* buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf); 
	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return NULL;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return NULL;
	}
	return buf;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	DWORD PID = GetProcId(L"dota2.exe");
	const wchar_t* dllPath = dllPath = L"ShadowDanceMenu.dll";
	

	if (PID == 0) {
		printf("Process not found\n");
		system("pause");
		return -1;
	}
	



	printf("Process pid: %d\n", PID);

	TOKEN_PRIVILEGES priv = { 0 };
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		priv.PrivilegeCount = 1;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
			AdjustTokenPrivileges(hToken, FALSE, &priv, 0, NULL, NULL);

		CloseHandle(hToken);
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProc) {
		DWORD Err = GetLastError();
		printf("OpenProcess failed: 0x%X\n", Err);
		system("PAUSE");
		return -2;
	}

	if (!IsCorrectTargetArchitecture(hProc)) {
		printf("Invalid Process Architecture.\n");
		CloseHandle(hProc);
		system("PAUSE");
		return -3;
	}

	if (GetFileAttributes(dllPath) == INVALID_FILE_ATTRIBUTES) {
		printf("Dll file doesn't exist\n");
		CloseHandle(hProc);
		system("PAUSE");
		return -4;
	}

	std::ifstream File(dllPath, std::ios::binary | std::ios::ate);

	if (File.fail()) {
		printf("Opening the file failed: %X\n", (DWORD)File.rdstate());
		File.close();
		CloseHandle(hProc);
		system("PAUSE");
		return -5;
	}

	auto FileSize = File.tellg();
	if (FileSize < 0x1000) {
		printf("Filesize invalid.\n");
		File.close();
		CloseHandle(hProc);
		system("PAUSE");
		return -6;
	}

	BYTE* pSrcData = new BYTE[(UINT_PTR)FileSize];
	if (!pSrcData) {
		printf("Can't allocate dll file.\n");
		File.close();
		CloseHandle(hProc);
		system("PAUSE");
		return -7;
	}

	File.seekg(0, std::ios::beg);
	File.read((char*)(pSrcData), FileSize);
	File.close();

	printf("Mapping...\n");
	if (!ManualMapDll(hProc, pSrcData, FileSize)) {
		delete[] pSrcData;
		CloseHandle(hProc);
		printf("Error while mapping.\n");
		system("PAUSE");
		return -8;
	}
	delete[] pSrcData;

	CloseHandle(hProc);
	printf("OK\n");
	return 0;
}

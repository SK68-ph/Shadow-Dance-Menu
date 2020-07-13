#pragma once
#include "CVars.h"
#include <sstream>  

typedef void(_fastcall* ExecuteCommand)(int, const char*, ...);


class UnrestrictedCMD
{
public:
    bool bSv_Cheats = false;

    uintptr_t getEngine2BaseAddress() {
        return (uintptr_t)GetModuleHandle(TEXT("engine2.dll"));
    }
    //Populate vars
    void Init() {
        engine2ModBase = getEngine2BaseAddress();
        cmdOffset = 0x46FA0;            // feature pattern scan soon.
        sv_cheatOffsets = 0x540CD8;     // 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 20 0B 7B CB 69 01 00 00 04 00 00 00 00 00 00 00 = 48bytes
        SV_CHEATS(true);
    }

    //Push command to console
    void ExecuteCmd(const char* command)
    {
        //input_forceuser
        //engine2.dll + 46F80  = 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 20 57 48 81 EC 40 01 00 00
        ExecuteCommand conMsg = (ExecuteCommand)(engine2ModBase + cmdOffset);
        conMsg(0, (const char*)command);
    }

    //Enable SV_CHEATS
    bool SV_CHEATS(bool bSet)
    {
        uintptr_t consoleAddr = (engine2ModBase + sv_cheatOffsets);
        bool *command = (bool*)consoleAddr;
        *command = bSet;
        return *command;
    }

    std::string stringBuild(std::string arg1, int number) {
        std::string s = std::to_string(number);
        return arg1 + s;
    }

private:
    unsigned long long cmdOffset;
    unsigned long long sv_cheatOffsets;
    uintptr_t engine2ModBase;
};
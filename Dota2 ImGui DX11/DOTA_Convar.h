#pragma once
#include "CVars.h"
#include <sstream>  
#include "Pattern.h"
#include "MemIn.h"

class UnrestrictedCMD
{
public:

    //Populate vars
    int Init() {
        engine2ModBase = (uintptr_t)GetModuleHandleA("engine2.dll");
        if (engine2ModBase == NULL) return -1;
        
        ScanModules();
        if (unrestictedCmdAddr == NULL || svcheatAddr == NULL) return -1;

        bInit = true;
        return 1;
    }

    bool isPtrReadable(uintptr_t base) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(reinterpret_cast<LPCVOID>(base), &mbi, sizeof(MEMORY_BASIC_INFORMATION)) || mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return 0;
        return 1;
    }

    int getVBE() {
        // Start AOB scan if pattern matched ingame.
        if (vbeBaseAddr == 0)
        {
            Pattern* vbeAOB = new Pattern(GetModuleHandleA("engine2.dll"), "? ? ? ? ? ? 00 00 10 00 00 00 00 00 00 00 25 00 00 00 00 00 00 00 00 ? ? ? ? ? 00 00");
            vbeBaseAddr = vbeAOB->GetAdress();
        }
        // Check
        if (vbeAddr == 0)// check PTR is initialized
            vbeAddr = memeIn.ReadMultiLevelPointer(vbeBaseAddr, vbeOffsets);

        if (isPtrReadable(vbeAddr) == false) {
            vbeAddr = 0;
            return 0;
        }
            

        int* VBE = (int*)vbeAddr;
        if (*VBE == 0) 
        {
            VBE = NULL;
            vbeAddr = NULL;
            return 0;
        }
        else if (*VBE == 6)
        {
            return 6;
        }
        else if (*VBE == 10)
        {
            return 10;
        }
        else if (*VBE == 14)
        {
            return 14;
        }
    }

    //Push command to Unrestricted Console
    int ExecuteCmd(const char* command)
    {
        if (bInit == false)
        {
            ExecuteCommand conMsg = (ExecuteCommand)unrestictedCmdAddr;
            conMsg(0, (const char*)command);
            return 1;
        }
        SV_CHEATS(true);
        if (bSV_Cheats == true && strlen(command) != 0)
        {
            ExecuteCommand conMsg = (ExecuteCommand)unrestictedCmdAddr;
            conMsg(0, (const char*)command);
            return 1;
        }
        return -1;
    }

    //Enable SV_CHEATS
    void SV_CHEATS(bool bSet)
    {
            bool* command = (bool*)svcheatAddr;
            *command = bSet;
            bSV_Cheats = *command;
    }

    // convert int then concatinate to command.
    std::string stringBuild(std::string arg1, int number) {
        std::string s = std::to_string(number);
        return arg1 + s;
    }

private:
    //Prototype
    typedef void(_fastcall* ExecuteCommand)(int, const char*, ...);
    //Vars
    MemIn memeIn;
    uintptr_t svcheatAddr;
    uintptr_t engine2ModBase;
    uintptr_t unrestictedCmdAddr;
    uintptr_t vbeBaseAddr;
    uintptr_t vbeAddr;
    std::vector<uint32_t> vbeOffsets = { 0x40, 0x98, 0x170, 0x0, 0x418, 0x20, 0xE04 };
    bool bSV_Cheats;
    bool bInit;

    // AOB SV_CHEATS 1
    // 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? ? 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    
    // AOB VBE
    // 80 13 94 1C 7D 01 00 00 10 00 00 00 00 00 00 00 25 00 00 00 00 00 00 00 00 04 AE F3 7C 01 00 00
    // "engine2.dll"+00580560 = 40, 98, 170, 0, 418, 20, E04;
    
    // AOB Unrestricted_CMD
    // input_forceuser 00007FF891669690
    // engine2.dll + 467D0  = 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 20 57 48 81 EC 40 01 00 00
    void ScanModules() {
        Pattern* cmdAOB = new Pattern(GetModuleHandleA("engine2.dll"), "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 20 57 48 81 EC 40 01 00 00");
        unrestictedCmdAddr = cmdAOB->GetAdress();
        //std::cout <<  "unrestictedCmdAddr = " << std::hex << unrestictedCmdAddr << std::endl;
        ExecuteCmd("sv_cheats 1");
        Pattern* cheatAOB = new Pattern(GetModuleHandleA("engine2.dll"), "01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? ? 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
        svcheatAddr = cheatAOB->GetAdress();
        //std::cout << "svcheatAddr = " << std::hex << svcheatAddr << std::endl;
    }

};
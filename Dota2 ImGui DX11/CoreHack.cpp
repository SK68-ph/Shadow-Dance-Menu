#include "CoreHack.h"

uintptr_t vbeBaseAddr;
uintptr_t vbeAddr;
std::vector<uint32_t> vbeOffsets = { 0x0, 0x30, 0x38, 0x98, 0x170, 0x0, 0xAC0 };


int Hack::getVBE() {
    
    if (vbeBaseAddr == NULL)
    {
        vbeBaseAddr = (uintptr_t)(utilities::PatternScan(GetModuleHandleA("engine2.dll"), "? ? ? ? ? 01 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 01 00 00"));
        if (vbeBaseAddr == NULL)
            vbeBaseAddr = (uintptr_t)(utilities::PatternScan(GetModuleHandleA("engine2.dll"), "? ? ? ? ? 02 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 02 00 00"));
        if (vbeBaseAddr == NULL) { 
            return -1; 
        } 
    }

    if (vbeAddr == NULL){
        vbeAddr = utilities::ReadMultiLevelPointer(vbeBaseAddr, vbeOffsets);
        if (vbeAddr == NULL)
            return -1;

    }
    if (utilities::isPtrReadable(vbeAddr) == false) {
        vbeAddr = NULL;
        return -1;
    }

    int* VBE = (int*)vbeAddr;
    if (*VBE == 0)
    {
        return 0;
    }
    return 1;
}

void Hack::ConVars::InitConvars() {
    ICvar* cvar = reinterpret_cast<ICvar*>(utilities::GetInterface("tier0.dll", "VEngineCvar007"));
    this->camera_distance = cvar->FindCommandBase("dota_camera_distance");
    this->range_display = cvar->FindCommandBase("dota_range_display");
    this->r_farz = cvar->FindCommandBase("r_farz");
    this->fog_enable = cvar->FindCommandBase("fog_enable");
    this->weather = cvar->FindCommandBase("cl_weather");
    this->particle_hack = cvar->FindCommandBase("dota_use_particle_fow");
}

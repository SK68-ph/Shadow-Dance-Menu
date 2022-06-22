#include "CoreHack.h"

uintptr_t vbeBaseAddr;
uintptr_t vbeAddr;
std::vector<uint32_t> vbeOffsets = { 0x0, 0x30, 0x38, 0x98, 0x170, 0x0, 0xAC0 };

int Hack::InitVbe() {

}

int Hack::getVBE() {

    if (vbeBaseAddr == NULL)
    {
        vbeBaseAddr = (uintptr_t)utilities::pattern_scan("engine2.dll", "? ? ? ? ? 01 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 01 00 00");
        if (vbeBaseAddr == NULL)
        {
            vbeBaseAddr = (uintptr_t)utilities::pattern_scan("engine2.dll", "? ? ? ? ? 02 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 02 00 00");
        }
        if (vbeBaseAddr == NULL) { 
            return -1; 
        } 
    }

    if (vbeAddr == NULL){
        vbeAddr = utilities::ReadMultiLevelPointer(vbeBaseAddr, vbeOffsets);
        if (vbeAddr == NULL)
            return -1;

        std::cout << "VBE ADDR = " << std::hex << vbeAddr << std::endl;
    }
    if (utilities::isPtrReadable(vbeAddr) == false) {
        vbeAddr = 0;
        return -1;
    }

    int* VBE = (int*)vbeAddr;
    if (*VBE == 0)
    {
        return 0;
    }
    return 1;
}

void Hack::ConVars::FindConVars() {
    ICvar* cvar = reinterpret_cast<ICvar*>(utilities::GetInterface("tier0.dll", "VEngineCvar007"));
    this->sv_cheats = cvar->FindCommandBase("sv_cheats");
    this->camera_distance = cvar->FindCommandBase("dota_camera_distance");
    this->range_display = cvar->FindCommandBase("dota_range_display");
    this->r_farz = cvar->FindCommandBase("r_farz");
    this->fog_enable = cvar->FindCommandBase("fog_enable");
    this->cl_weather = cvar->FindCommandBase("cl_weather");
    this->particle_hack = cvar->FindCommandBase("dota_use_particle_fow");
}


//void Hack::PrepareCommand(std::string command, int value) {
//    std::string tempCmnd = command + std::to_string(value);
//    const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
//    ExecuteUnrestrictedCmd(chrCommand);
//}
//void Hack::PrepareCommand(std::string command, const char* value) {
//    std::string val = value;
//    std::string tempCmnd = command + val;
//    const char* chrCommand = const_cast<char*>(tempCmnd.c_str());
//    std::cout << chrCommand << std::endl;
//    ExecuteUnrestrictedCmd(chrCommand);
//}

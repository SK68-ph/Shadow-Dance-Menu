#include "CoreHack.h"
#include <map>

uintptr_t vbeBaseAddr;
uintptr_t vbeAddr;
std::vector<unsigned int> vbeOffsets;
std::vector<uintptr_t> vbeScanAddr;
std::vector<unsigned int> vbeScanOffsets;

std::vector<const char*> patterns = { 
    "? ? ? ? ? 01 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 01 00 00" ,
    "? ? ? ? ? 02 00 00 02 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 ? ? ? ? ? 02 00 00" 
};

int Hack::LoadOffsetConfig() {

    std::fstream file;
    std::string word;
    std::vector<std::string> offsets;
    std::vector<unsigned int> offsetsInt;

    std::cout << "Loading offsets.ini" << std::endl;
    file.open("offsets.ini", std::ios::out | std::ios::in);
    if (file.fail()) {
        std::cout << "Failed to load offsets.init, loading defaults (If vbe not working try to rescan)" << std::endl;
        return -1;
    }
    int i = 0;
    while (file >> word)
    {
        if (i >= 8 || word.empty()) break;

        offsets.push_back(word);
        i++;
    }
    for (size_t i = 0; i < offsets.size(); i++)
    {
        std::istringstream buffer(offsets[i]);
        unsigned long long value;
        buffer >> std::hex >> value;
        offsetsInt.push_back(value);
    }
    vbeOffsets = offsetsInt;
    return 1;
}

int Hack::SaveOffsetConfig() {
    std::ofstream file("offsets.ini");

    std::cout << "Saving to offsets.ini" << std::endl;
    for (size_t i = 0; i < vbeOffsets.size(); i++)
    {
        std::stringstream sstream;
        sstream << std::hex << vbeOffsets[i];
        std::string result = sstream.str();
        file << result << std::endl;
    }
    file.close();
}

void Hack::InitHack() {
    if (LoadOffsetConfig() == -1)
    {
        vbeOffsets = { 0x0, 0x30, 0x38, 0x98, 0x170, 0x0, 0xAB0 };
        SaveOffsetConfig();
    }
}


void ScanVBEBase() {
    for (size_t i = 0; i < patterns.size(); i++)
    {
        vbeBaseAddr = (uintptr_t)(utilities::PatternScan(GetModuleHandleA("engine2.dll"), patterns[i]));
        if (vbeBaseAddr != NULL)
            break;
    }
    if (vbeBaseAddr != NULL)
    {
        std::cout << "Found vbe base address = " << std::hex << vbeBaseAddr << std::endl;
    }
}


int Hack::ScanVbeOffset(bool firstScan) {
    if (vbeBaseAddr == NULL)
    {
        ScanVBEBase();
        if (vbeBaseAddr == NULL)
            return 0;
    }
    if (firstScan)
    {
        auto tempOffsets = vbeOffsets;
        for (int i = 0x100; i < 0xFFF; i = i + 0x4)
        {
            tempOffsets[tempOffsets.size() - 1] = i;
            vbeAddr = utilities::ReadMultiLevelPointer(vbeBaseAddr, tempOffsets);
            if (*(float*)vbeAddr > 0.0001 && *(float*)vbeAddr < 1.0)
            {
                vbeScanAddr.push_back(vbeAddr);
                vbeScanOffsets.push_back(i);
                std::cout << "Scanned vbe addr = " << std::hex << vbeAddr << " Offset = " << i << std::endl;
            }
        }
        std::cout << "Scan complete, Make your self visible to the enemy and hit Scan button again" << std::endl;
        return 1;
    }
    else
    {
        for (size_t i = 0; i < vbeScanAddr.size(); i++)
        {
            if (*(int*)(vbeScanAddr[i]) == 0) {
                vbeOffsets[vbeOffsets.size() - 1] = vbeScanOffsets[i];
                std::cout << "Success Vbe can now be enabled. Found correct vbe addr = " << std::hex << vbeScanAddr[i] << " Offset = " << vbeScanOffsets[i] << std::endl;
                vbeAddr = NULL;
                SaveOffsetConfig();
                return 1;
            }
        }
    }
    return 0;
}


int Hack::getVBE() {
    
    if (vbeBaseAddr == NULL)
    {
        ScanVBEBase();
        if (vbeBaseAddr == NULL)
            return -1;
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
    std::cout << "Found cvar address = " << cvar << std::endl;
    this->sv_cheats = cvar->FindCommandBase("sv_cheats");
    this->camera_distance = cvar->FindCommandBase("dota_camera_distance");
    this->drawrange = cvar->FindCommandBase("dota_range_display");
    this->r_farz = cvar->FindCommandBase("r_farz");
    this->fog_enable = cvar->FindCommandBase("fog_enable");
    this->weather = cvar->FindCommandBase("cl_weather");
    this->particle_hack = cvar->FindCommandBase("dota_use_particle_fow");
}

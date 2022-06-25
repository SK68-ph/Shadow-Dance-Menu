#include "CoreHack.h"

CGameEntitySystem* entity;
std::vector<CEntityInstance*> Heroes;
VMT* entityVMT;

CEntityInstance* OnAddEntity(CGameEntitySystem* ecx, CEntityInstance* ptr, EntityHandle index)
{
    auto ret = entityVMT->GetOriginalMethod(OnAddEntity)(ecx, ptr, index);
    const char* typeName = ptr->Schema_DynamicBinding()->bindingName;

    if (strstr(typeName, "DOTA_Unit_Hero")) {

        auto alreadyExists = false;
        for (auto hero : Heroes)
        {
            if (typeName == hero->Schema_DynamicBinding()->bindingName)
            {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists)
        {
            Heroes.push_back(ptr);
        }
    }

    return ret;
}

CEntityInstance* OnRemoveEntity(CGameEntitySystem* ecx, CEntityInstance* ptr, EntityHandle index)
{
    const char* typeName = ptr->Schema_DynamicBinding()->bindingName;

    if (strstr(typeName, "DOTA_Unit_Hero")) {
        for (size_t i = Heroes.size(); i-- > 0; ) {
            if (Heroes[i] == ptr) {
                Heroes.erase(Heroes.begin() + i);
                break;
            }
        }
    }

    return entityVMT->GetOriginalMethod(OnRemoveEntity)(ecx, ptr, index);
}

void InitHack() {
    // Init CGameEntitySystem
    void* client = utilities::GetInterface("client.dll", "Source2Client002");
    uintptr_t* vmt_slot = *(uintptr_t**)client + 25; //25th function in Source2Client vtable
    uintptr_t addr_start = *vmt_slot + 3; //stores the relative address portion of the mov rax, [rip + 0x2512059] instruction
    entity = *(CGameEntitySystem**)(addr_start + *(uint32_t*)(addr_start)+4); //pointer to CGameEntitySystem is at 2512059 + addr_start + 4
    std::cout << std::hex <<  "Entity Addr = " << (uintptr_t)entity << std::endl;
    // Hook our entity
    entityVMT = new VMT(entity); //loads CGameEntitySystem VMT into vmt.entity
    entityVMT->HookVMT(OnAddEntity, 14);
    entityVMT->HookVMT(OnRemoveEntity, 15);
    entityVMT->ApplyVMT(entity);
}

void ExitHack()
{
    // Unhook entity
    entityVMT->RevertVMT(entity);
}

int getVBE() {
    if (Heroes.size() == 0) // check if entity is populated
    {
        return -1;
    }

    auto VBE = *(int*)(((uintptr_t)Heroes[0]) + 0x16B0); // vbeoffset = CEntityInstance+0x16B0
    if (VBE == 0)
    {
        return 0;
    }
    return 1;
}

void ConVars::InitConvars() {
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

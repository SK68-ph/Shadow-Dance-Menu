#pragma once
#include "CoreHack.h"
#include "common.h"
#include "Schema.h"
#include "sdk/CGameEntitySystem.h"
#include "sdk/IAppSystem.h"
#include "sdk/ICVar.h"

class IEngineClient;

VMT* entityVMT;
CGameEntitySystem* entity;
IEngineClient* engine;
std::vector<C_BaseEntity*> Heroes;
ICvar* cvar;
ConCommandBase* sv_cheats;
ConCommandBase* camera_distance;
ConCommandBase* drawrange;
ConCommandBase* r_farz;
ConCommandBase* fog_enable;
ConCommandBase* weather;
ConCommandBase* particle_hack;
SchemaNetvarCollection* Netvars = 0;


C_BaseEntity* OnAddEntity(CGameEntitySystem* ecx, C_BaseEntity* ptr, EntityHandle index)
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
                int EntityIndex = index & 0x7FFF;
                Heroes.emplace_back(ptr);
        }
    }

    return ret;
}

C_BaseEntity* OnRemoveEntity(CGameEntitySystem* ecx, C_BaseEntity* ptr, EntityHandle index)
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

void InitConvars() {
    cvar = (ICvar*)GetInterface("tier0.dll", "VEngineCvar007");
    sv_cheats = cvar->FindCommandBase("sv_cheats");
    camera_distance = cvar->FindCommandBase("dota_camera_distance");
    drawrange = cvar->FindCommandBase("dota_range_display");
    r_farz = cvar->FindCommandBase("r_farz");
    fog_enable = cvar->FindCommandBase("fog_enable");
    weather = cvar->FindCommandBase("cl_weather");
    particle_hack = cvar->FindCommandBase("dota_use_particle_fow");
}

void InitEntity() {
    void* client = GetInterface("client.dll", "Source2Client002");
    uintptr_t* vmt_slot = *(uintptr_t**)client + 25; //25th function in Source2Client vtable
    uintptr_t addr_start = *vmt_slot + 3; //stores the relative address portion of the mov rax, [rip + 0x2512059] instruction
    entity = *(CGameEntitySystem**)(addr_start + *(uint32_t*)(addr_start)+4); //pointer to CEntitySystem is at 2512059 + addr_start + 4
    // Init CEngine
    engine = (IEngineClient*)GetInterface("engine2.dll", "Source2EngineToClient001");
    // Hook our entity
    entityVMT = new VMT(entity); //loads CEntitySystem VMT into vmt.entity
    entityVMT->HookVMT(OnAddEntity, 14);
    entityVMT->HookVMT(OnRemoveEntity, 15);
    entityVMT->ApplyVMT(entity);
}
void InitSchema() {
    CMsg = (ConMsg)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg");
    SchemaSystem = (u64)GetInterface("schemasystem.dll", "SchemaSystem_001");
    Netvars = new SchemaNetvarCollection;
    Netvars->Add("C_BaseEntity", "client.dll");
    Netvars->Add("CGameSceneNode", "client.dll");
    Netvars->Add("C_DOTA_BaseNPC", "client.dll");
    Netvars->Add("C_DOTA_BaseNPC_Hero", "client.dll");
    Netvars->Add("C_DOTABaseAbility", "client.dll");
    Netvars->Add("C_DOTAGamerules", "client.dll");
    Netvars->Add("C_BaseModelEntity", "client.dll");
    Netvars->Add("CGlowProperty", "client.dll");
    Netvars->Add("C_EconEntity", "client.dll");
    Netvars->Add("CAttributeContainer", "client.dll");
    Netvars->Add("CEconItemView", "client.dll");
    Netvars->Add("C_DOTAWearableItem", "client.dll");
    Netvars->Add("C_BaseCombatCharacter", "client.dll");
    m_iTeamNum = Netvars->Get((u64)"m_iTeamNum")->offset;
    m_hOwnerEntity = Netvars->Get((u64)"m_hOwnerEntity")->offset;
    m_clrRender = Netvars->Get((u64)"m_clrRender")->offset;
    m_flStartSequenceCycle = Netvars->Get((u64)"m_flStartSequenceCycle")->offset;
    m_fGameTime = Netvars->Get((u64)"m_fGameTime")->offset;
    m_nGameState = Netvars->Get((u64)"m_nGameState")->offset;
    m_iGameMode = Netvars->Get((u64)"m_iGameMode")->offset;
    m_hReplicatingOtherHeroModel = Netvars->Get((u64)"m_hReplicatingOtherHeroModel")->offset;
    m_lifeState = Netvars->Get((u64)"m_lifeState")->offset;
}

void InitHack() {
    InitConvars();
    InitEntity();
    InitSchema();
}

void RemoveVmtHooks()
{
    entityVMT->RevertVMT(entity); // Unhook entity
}

bool isEntityPopulated()
{
    return (Heroes.size() > 0);
}

int localHero = -1;
int localPlayerIndex = -1;
int& GetLocalPlayer(int& = localPlayerIndex, int screen = 0) {
    typedef int& (*Fn)(void*, int&, int);
    return getvfunc<Fn>(engine, 22)(engine, localPlayerIndex, screen);
}


int getVBE() {
    if (!isEntityPopulated()) // check if entity is populated
    {
        localPlayerIndex = -1;
        localHero = -1;
        return -1;
    }
    if (localPlayerIndex == -1)
    {
        GetLocalPlayer(localPlayerIndex);
        localPlayerIndex++;
        if (localPlayerIndex == -1)
        {
            return -1;
        }
    }
    if (localHero == -1)
    {
        for (size_t i = 0; i < Heroes.size(); i++)
        {
            if (localPlayerIndex == Heroes[i]->OwnerIndex())
            {
                localHero = i;
                break;
            }

        }
        if (localHero == -1)
        {
            return -1;
        }
    }

    auto VBE = Heroes[localHero]->IsVisibleByEnemy(); // vbeoffset = C_BaseEntitye+0x16B0
    if (VBE)
    {
        return 0;
    }
    return 1;
}

void ResetConvars()
{
    if (cvar != nullptr)
    {
        weather->SetValue(0);
        camera_distance->SetValue(1200);
        drawrange->SetValue(0);
        r_farz->SetValue(-1);
        fog_enable->SetValue(false);
        particle_hack->SetValue(false);
        sv_cheats->SetValue(0);
    }
}

void SetWeather(int val) {
    weather->SetValue(val);
}
void SetDrawRange(int val) {
    sv_cheats->SetValue(1);
    drawrange->SetValue(val);
}
void SetParticleHack(int val) {
    particle_hack->SetValue(val);
}
void SetNoFog(int val) {
    fog_enable->SetValue(val);
}
void SetCamDistance(int val) {
    camera_distance->SetValue(val);			
    r_farz->SetValue(val * 2);
}

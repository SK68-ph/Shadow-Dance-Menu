#pragma once
// dllmain.cpp : Defines the entry point for the DLL application.
#include "includes.h"
#define ui unsigned long long
#define cc const char*
#pragma once
#include <ctime>
#include <string>
#include <Psapi.h>
#define InRange(x, a, b) (x >= a && x <= b)
#define getBit(x) (InRange((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (InRange(x, '0', '9') ? x - '0': 0))
#define getByte(x) (getBit(x[0]) << 4 | getBit(x[1]))
#define VT_METHOD(region, index) (*(ui*)(*(ui*)region + index * 8))
#define getEntityIndexclient(ent) (*(signed short*)(*(ui*)(ent + 0x10)+ 0x10) & 0x7FFF)
#define n2hex(x) ((ui)n2hexstr(x).c_str())
ui m_iHealth, m_iMaxHealth, m_flMana, m_flMaxMana, m_iCurrentLevel, m_hAbilities,
m_lifeState, m_hReplicatingOtherHeroModel, m_iTeamNum, m_hOwnerEntity,
m_clrRender, m_iGlowType,
m_glowColorOverride, a_m_iLevel, a_m_iManaCost, a_m_flCooldownLength, a_m_fCooldown,
a_m_bHidden, m_Glow, m_iHealthBarOffset, m_pGameSceneNode, m_vecAbsOrigin, m_fGameTime, m_nGameState;

enum GameState : int
{
    DOTA_GAMERULES_STATE_INIT = 0,
    DOTA_GAMERULES_WAIT_FOR_PLAYERS_TO_LOAD,
    DOTA_GAMERULES_HERO_SELECTION,
    DOTA_GAMERULES_STRATEGY_TIME,
    DOTA_GAMERULES_PREGAME,
    DOTA_GAMERULES_GAME_IN_PROGRESS,
    DOTA_GAMERULES_POSTGAME,
    DOTA_GAMERULES_DISCONNECT,
    DOTA_GAMERULES_TEAM_SHOWCASE,
    DOTA_GAMERULES_CUSTOM_GAME_SETUP,
    DOTA_GAMERULES_WAIT_FOR_MAP_TO_LOAD
};
template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len, '0');
    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
        rc[i] = digits[(w >> j) & 0x0f];
    return rc;
}
int vstrcmp(ui a, ui b) {
    return strcmp((cc)a, (cc)b);
}
int vstrcmp(cc a, ui b) {
    return strcmp(a, (cc)b);
}
int vstrcmp(ui a, cc b) {
    return strcmp((cc)a, b);
}
int vstrcmp(cc a, cc b) {
    return strcmp(a, b);
}
typedef void(__fastcall* ConMsg)(cc, ui, ui, ui);
ConMsg CMsg = 0;
void CMSG(cc pure) {

    CMsg(pure, 0, 0, 0);

}

void CMSG(cc format, ui p1) {

    CMsg(format, p1, 0, 0);

}

void CMSG(cc format, ui p1, ui p2) {

    CMsg(format, p1, p2, 0);

}

void CMSG(cc format, ui p1, ui p2, ui p3) {

    CMsg(format, p1, p2, p3);

}
ui FPat(const ui& start_address, const ui& end_address, const char* target_pattern) {
    const char* pattern = target_pattern;

    ui first_match = 0;

    for (ui position = start_address; position < end_address; position++) {
        if (!*pattern)
            return first_match;

        const unsigned char pattern_current = *reinterpret_cast<const unsigned char*>(pattern);
        const unsigned char memory_current = *reinterpret_cast<const unsigned char*>(position);

        if (pattern_current == '\?' || memory_current == getByte(pattern)) {
            if (!first_match)
                first_match = position;

            if (!pattern[2])
                return first_match;

            pattern += pattern_current != '\?' ? 3 : 2;
        }
        else {
            pattern = target_pattern;
            first_match = 0;
        }
    }

    return NULL;
}

ui FPat(const char* module, const char* target_pattern) {
    MODULEINFO module_info = { 0 };

    if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(module), &module_info, sizeof(MODULEINFO)))
        return NULL;

    const ui start_address = ui(module_info.lpBaseOfDll);
    const ui end_address = start_address + module_info.SizeOfImage;

    return FPat(start_address, end_address, target_pattern);
}
inline ui GetAbsoluteAddress(ui instruction_ptr, int offset, int size)
{
    return instruction_ptr + *reinterpret_cast<int32_t*>(instruction_ptr + offset) + size;
}

typedef ui(__fastcall* fNextEnt)(ui, ui);
fNextEnt NextEnt;
ui CGameEntSystem;
ui ent_find_pat;
typedef ui(__fastcall* fGLocalPlayerentindex)(ui self, ui ptrtoint, ui zero);

fGLocalPlayerentindex GLocalPlayer;

typedef unsigned char(__fastcall* IsInGame)(ui);

IsInGame InGame;
typedef void* (*oCreateInterface)(const char*, int);
oCreateInterface pCreateInterface;
ui Panorama, Panorama2;
ui oRunFrame;
ui CreateInterface(const char* szModule, const char* szInterface) {


    pCreateInterface = (oCreateInterface)GetProcAddress(GetModuleHandleA(szModule), "CreateInterface");


    return (ui)pCreateInterface(szInterface, 0);


}
bool IsInMatch = false;
ui CEngineClient = 0;
int LocalPlayerIndex = -1;
ui LocalEnt = 0;
ui LocalPlayer;
unsigned char LocalTeam = 0;
ui framecounter = 0;//он нам нужен чисто ради теста тип каждые 600 кадров выводим в консоль нашу инфу.
bool msged = false;
void ExitedMatch() {
    IsInMatch = false;
    LocalPlayerIndex = -1;
    LocalEnt = 0;
    LocalPlayer = 0;
    LocalTeam = 0;
    framecounter = 0;
    msged = false;
    CMSG("exited match!\n");
}
ui GameRules, GameRulesPtr;
float GameTime() {
    return *(float*)(GameRules + m_fGameTime);
}
int GameState() {
    return *(int*)(GameRules + m_nGameState);
}

void EnteredMatch() {
    IsInMatch = true;
    GameRules = *(ui*)GameRulesPtr;
    CMSG("entered match!\n");

}
bool inited = false;

const char* getBaseClass(ui ent) {
    if (
        *(ui*)(ent + 0x10) == NULL
        ||
        *(ui*)(*(ui*)(ent + 0x10) + 0x8) == NULL
        ) return 0;
    return ***(const char****)(
        *(ui*)(ent + 0x10) + 0x8
        );
}

enum LifeState : char
{
    UnitAlive = 0, KillCam = 1, UnitDead = 2
};
void hkRunFrame() {
    ((void(__fastcall*)(ui))oRunFrame)(Panorama2);
    if (!inited) return;
    if (InGame(CEngineClient)) {
        if (!IsInMatch) EnteredMatch();
    }
    else if (IsInMatch) ExitedMatch();
    if (IsInMatch) {
        if (GameState() == DOTA_GAMERULES_GAME_IN_PROGRESS || GameState() == DOTA_GAMERULES_PREGAME) {
            if (!LocalTeam) {
                if (LocalPlayerIndex == -1) {
                    GLocalPlayer(CEngineClient, (ui)&LocalPlayerIndex, 0);
                    if (LocalPlayerIndex == -1) return;
                }
                if (!LocalPlayer) {
                    ui ent = NextEnt(CGameEntSystem, 0);

                    while (ent) {

                        if (*(ui*)(ent + 0x10))
                        {
                            if (getEntityIndexclient(ent) == LocalPlayerIndex) {
                                LocalPlayer = ent;
                                break;
                            }
                        }
                        *(ui*)(*(ui*)(ent + 0x10) + 0x58) == 0 ? ent = 0 : ent = *(ui*)(*(ui*)(*(ui*)(ent + 0x10) + 0x58));

                    }
                    if (!LocalPlayer) return;
                }
                if (!LocalTeam) {
                    LocalTeam = *(unsigned char*)(LocalPlayer + m_iTeamNum);
                    if (!LocalTeam) return;
                }
            }
            else {
                if (!LocalEnt) {
                    ui ent = NextEnt(CGameEntSystem, 0);

                    while (ent) {

                        if (*(ui*)(ent + 0x10))
                        {
                            if ((*(int*)(ent + m_hOwnerEntity) & 0x7FFF) == LocalPlayerIndex) {
                                if (!vstrcmp(getBaseClass(ent), "C_DOTA_BaseNPC_Hero")) {
                                    LocalEnt = ent;
                                    break;
                                }
                            }
                        }
                        *(ui*)(*(ui*)(ent + 0x10) + 0x58) == 0 ? ent = 0 : ent = *(ui*)(*(ui*)(*(ui*)(ent + 0x10) + 0x58));

                    }
                }
                else {
                    if (!msged) {
                        CMSG("local hero name: %s\n", *(ui*)(*(ui*)(LocalEnt + 0x10) + 0x18));
                        CMSG("local player index: %d\n", LocalPlayerIndex);
                        CMSG("local team: %d\n", LocalTeam);
                        msged = true;
                    }
                    framecounter++;
                    if (framecounter == 600) {
                        framecounter = 0;
                        CMSG("local hero health: %d\n", *(int*)(LocalEnt + m_iHealth));
                        CMSG("local hero level: %d\n", *(int*)(LocalEnt + m_iCurrentLevel));
                        CMSG("local hero lifestate: %d\n", *(char*)(LocalEnt + m_lifeState));//enum LifeState чекаем
                        CMSG("local hero mana: %s\n", (ui)std::to_string(*(float*)(LocalEnt + m_flMana)).c_str());

                    }
                }
            }
        }
    }
}
ui SchemaSystem;
template<typename T, ui SIZE>
class CArray {
public:
    T* elms[SIZE] = { 0 };
    ui count = 0;
    void operator=(T* elem) {
        elms[count++] = elem;
    };
    bool operator!() {
        return count == 0;
    }
    explicit operator bool() const
    {
        return count != 0;
    }
    T** begin() {
        return &elms[0];
    }
    T** end() {
        return &elms[count];
    }
    __forceinline T* last() {
        return elms[count - 1];
    }
    __forceinline T* first() {
        return elms[0];
    }
    void RemoveAndShift(T* elm) {
        for (ui i = 0; i < count; i++) {
            if (elms[i] == elm) {
                T* aa = elms[i];
                for (ui j = i; j < count; j++) {
                    if (elms[j + 1]) elms[j] = elms[j + 1];
                    else {
                        elms[j] = 0;
                        break;
                    }
                }
                count--;

            }
        }
    }
    void Destroy() {
        for (ui i = 0; i < count; i++) {
            delete elms[i];
        }
    }
};
struct ClassDescription;
struct SchemaParent {
    ui idk;
    ClassDescription* parent;
};
struct ClassDescription {
    ui idk;//0
    ui classname;//8
    ui modulename;//10
    int sizeofclass;//18
    short memberstoiterate;//1c
    char pad[6];//20
    ui MemberInfo;//28
    ui idk2;//30
    SchemaParent* parent;//38
};

struct SchemaTypeDescription {
    ui idk;
    ui name;
    ui idk2;
};
struct MemberDescription {
    ui name;
    SchemaTypeDescription* schematypeptr;
    int offset;
    int idk;
    ui idk2;
};
class schemanetvar {
public:
    ui classname;
    ui name;
    ui _typename;
    int offset;
    schemanetvar(ui a, ui b, ui c, ui d) {
        classname = a;
        name = b;
        _typename = c;
        offset = d;
    }
};
class SchemaNetvarCollection {
public:
    CArray<schemanetvar, 1000> Netvars;
    void Add(cc _class, cc _module) {
        ui Scope = ((ui(__fastcall*)(ui schemasys, const char* _mod))
            (*(ui*)(*(ui*)(SchemaSystem)+0x68)))(SchemaSystem, _module);
        if (!Scope) { CMSG("No such scope %s!\n", (ui)_module); return; }
        ui Class = ((ui(__fastcall*)(ui scope, const char* _class))
            (*(ui*)(*(ui*)(Scope)+0x10)))(Scope, _class);
        if (!Class) { CMSG("No such class %s!\n", (ui)_class); return; }
        ClassDescription* a = (ClassDescription*)Class;
        for (ui i = 0; i < a->memberstoiterate; i++) {
            MemberDescription* z = (MemberDescription*)(a->MemberInfo + i * 0x20);
            Netvars = new schemanetvar(a->classname, z->name, z->schematypeptr->name, (ui)z->offset);
        }
    }
    schemanetvar* Get(ui name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);
    }
    schemanetvar* Get(ui _class, ui name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name) && !vstrcmp(netvar->classname, _class)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);

    }
};
SchemaNetvarCollection* Netvars = 0;

void OnInject() {
    CMsg = (ConMsg)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg");

    ent_find_pat = FPat("client.dll", "48 83 EC 28 8B 02 83 F8 02 7D 12 48 8D 0D ? ? ? ? 48 83 C4 28 48 FF 25 ? ? ? ? 48 89 6C");
    if (ent_find_pat == 0) {
        CMSG("ent_find_pat is NULL :(");
        return;
    }
    NextEnt = (fNextEnt)GetAbsoluteAddress(ent_find_pat + 0x5a, 1, 5);
    CGameEntSystem = *(ui*)(GetAbsoluteAddress(ent_find_pat + 0x51, 3, 7));

    ui GameRulesXREF = FPat("client.dll", "48 8b 05 ? ? ? ? 48 85 c0 75 ? 0f 57 c0 c3 f3 0f 10 80 ? ? ? ? c3");
    if (!GameRulesXREF) { CMSG("pattern GameRulesXREF not found\n"); return; }

    GameRulesPtr = GetAbsoluteAddress(GameRulesXREF, 3, 7);

    CEngineClient = CreateInterface("engine2.dll", "Source2EngineToClient001");
    InGame = (IsInGame)VT_METHOD(CEngineClient, 26);
    GLocalPlayer = (fGLocalPlayerentindex)VT_METHOD(CEngineClient, 22);


    SchemaSystem = CreateInterface("schemasystem.dll", "SchemaSystem_001");
    Netvars = new SchemaNetvarCollection;
    Netvars->Add("C_BaseEntity", "client.dll");
    Netvars->Add("CGameSceneNode", "client.dll");
    Netvars->Add("C_DOTA_BaseNPC", "client.dll");
    Netvars->Add("C_DOTA_BaseNPC_Hero", "client.dll");
    Netvars->Add("C_DOTABaseAbility", "client.dll");
    Netvars->Add("C_DOTAGamerules", "client.dll");
    Netvars->Add("C_BaseModelEntity", "client.dll");
    Netvars->Add("CGlowProperty", "client.dll");
    m_iHealth = Netvars->Get((ui)"m_iHealth")->offset;
    m_iMaxHealth = Netvars->Get((ui)"m_iMaxHealth")->offset;
    m_flMana = Netvars->Get((ui)"m_flMana")->offset;
    m_flMaxMana = Netvars->Get((ui)"m_flMaxMana")->offset;
    m_iCurrentLevel = Netvars->Get((ui)"m_iCurrentLevel")->offset;
    m_hAbilities = Netvars->Get((ui)"m_hAbilities")->offset;
    m_lifeState = Netvars->Get((ui)"m_lifeState")->offset;
    m_hReplicatingOtherHeroModel = Netvars->Get((ui)"m_hReplicatingOtherHeroModel")->offset;
    m_iTeamNum = Netvars->Get((ui)"m_iTeamNum")->offset;
    m_hOwnerEntity = Netvars->Get((ui)"m_hOwnerEntity")->offset;
    m_clrRender = Netvars->Get((ui)"m_clrRender")->offset;
    m_Glow = Netvars->Get((ui)"m_Glow")->offset;
    m_iHealthBarOffset = Netvars->Get((ui)"m_iHealthBarOffset")->offset;
    m_iGlowType = Netvars->Get((ui)"m_iGlowType")->offset;
    m_glowColorOverride = Netvars->Get((ui)"m_glowColorOverride")->offset;
    a_m_iLevel = Netvars->Get((ui)"m_iLevel")->offset;
    a_m_iManaCost = Netvars->Get((ui)"m_iManaCost")->offset;
    a_m_flCooldownLength = Netvars->Get((ui)"m_flCooldownLength")->offset;
    a_m_fCooldown = Netvars->Get((ui)"m_fCooldown")->offset;
    a_m_flCooldownLength = Netvars->Get((ui)"m_flCooldownLength")->offset;
    a_m_bHidden = Netvars->Get((ui)"m_bHidden")->offset;
    m_pGameSceneNode = Netvars->Get((ui)"m_pGameSceneNode")->offset;
    m_vecAbsOrigin = Netvars->Get((ui)"m_vecAbsOrigin")->offset;
    m_fGameTime = Netvars->Get((ui)"m_fGameTime")->offset;
    m_nGameState = Netvars->Get((ui)"m_nGameState")->offset;

    Panorama = CreateInterface("panorama.dll", "PanoramaUIEngine001");

    Panorama2 = *(ui*)(Panorama + 0x28);
    MEMORY_BASIC_INFORMATION mbi;

    VirtualQuery((LPCVOID) * (ui*)Panorama2, &mbi, sizeof(mbi));

    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
    oRunFrame = VT_METHOD(Panorama2, 6);
    VT_METHOD(Panorama2, 6) = (ui)&hkRunFrame;

    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

    CMSG("injection completed successfuly at %s, runframe at %s !\n", (ui)std::to_string(std::time(0)).c_str(), n2hex((ui)&hkRunFrame));
    inited = true;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == 1)  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)OnInject, 0, 0, 0);
    return TRUE;
}
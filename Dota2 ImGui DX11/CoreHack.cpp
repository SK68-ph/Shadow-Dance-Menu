#include "CoreHack.h"

CGameEntitySystem* entity;
IEngineClient* engine;
std::vector<CEntityInstance*> Heroes;
std::vector<unsigned int> Indexes;
VMT* entityVMT;
#define cc const char*
#define ui unsigned long long
#define u64 unsigned long long
#define VT_METHOD(region, index) (*(ui*)(*(ui*)region + index * 8))
typedef ui(__fastcall* fGLocalPlayerentindex)(ui self, ui ptrtoint, ui zero);
fGLocalPlayerentindex GLocalPlayer;
int LocalPlayerIndex = -1;
ui SchemaSystem;
ui m_iTeamNum,m_hOwnerEntity, m_flStartSequenceCycle, m_fGameTime, m_nGameState, m_lifeState,
m_iGameMode, m_clrRender, m_hReplicatingOtherHeroModel;
template<typename T, typename Z>
bool StringsMatch(T a, Z b) {
    return !strcmp((cc)a, (cc)b);
}
class ENTITY;
class CHANDLE {
public:
    int handle;
    short Index() {
        return handle & 0x7fff;
    }
};
class CENTITYIDENTITY {
public:
    ENTITY* entity;//0
    u64 baseinfo;//8
    CHANDLE handle;//10
private:
    int unk;//14
public:
    cc name;//18
    cc designer_name;
    char pad[0x30];
    CENTITYIDENTITY* m_pNext;//20
};

class ENTITY {
public:
    u64 unk;
    CENTITYIDENTITY* identity;
    ENTITY* Next() {
        if (!identity->m_pNext) return 0;
        return identity->m_pNext->entity;
    }
    int CHandle() {
        return identity->handle.handle;
    }
    short Index() {
        return identity->handle.Index();
    }
    cc BaseClass() {
        if (
            !identity || !identity->baseinfo
            ) return 0;
        return **(const char***)(
            identity->baseinfo
            );
    }
    bool OfBaseClass(cc bc) {
        return StringsMatch(BaseClass(), bc);
    }
    bool IsHero() {
        return OfBaseClass("C_DOTA_BaseNPC_Hero");
    }
    cc name() {
        return identity->name;
    }
    cc designername() {
        return identity->designer_name;
    }
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
                int EntityIndex = index & 0x7FFF;
                Heroes.emplace_back(ptr);
                std::cout << typeName << std::hex << " Addr = " << (uintptr_t)ptr << " " << std::dec << EntityIndex << std::endl;
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
    CMsg = (ConMsg)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg");
    // Init CEntitySystem
    void* client = utilities::GetInterface("client.dll", "Source2Client002");
    uintptr_t* vmt_slot = *(uintptr_t**)client + 25; //25th function in Source2Client vtable
    uintptr_t addr_start = *vmt_slot + 3; //stores the relative address portion of the mov rax, [rip + 0x2512059] instruction
    entity = *(CGameEntitySystem**)(addr_start + *(uint32_t*)(addr_start)+4); //pointer to CEntitySystem is at 2512059 + addr_start + 4
    // Init CEngine
    engine = static_cast<IEngineClient*>(utilities::GetInterface("engine2.dll", "Source2EngineToClient001"));
    // Hook our entity
    entityVMT = new VMT(entity); //loads CEntitySystem VMT into vmt.entity
    entityVMT->HookVMT(OnAddEntity, 14);
    entityVMT->HookVMT(OnRemoveEntity, 15);
    entityVMT->ApplyVMT(entity);
    GLocalPlayer = (fGLocalPlayerentindex)VT_METHOD(engine, 22);
    std::cout << std::hex << "Entity Addr = " << (uintptr_t)entity << std::endl; 


    SchemaSystem = (ui)utilities::GetInterface("schemasystem.dll", "SchemaSystem_001");
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
    m_iTeamNum = Netvars->Get((ui)"m_iTeamNum")->offset;
    m_hOwnerEntity = Netvars->Get((ui)"m_hOwnerEntity")->offset;
    m_clrRender = Netvars->Get((ui)"m_clrRender")->offset;
    m_flStartSequenceCycle = Netvars->Get((ui)"m_flStartSequenceCycle")->offset;
    m_fGameTime = Netvars->Get((ui)"m_fGameTime")->offset;
    m_nGameState = Netvars->Get((ui)"m_nGameState")->offset;
    m_iGameMode = Netvars->Get((ui)"m_iGameMode")->offset;
    m_hReplicatingOtherHeroModel = Netvars->Get((ui)"m_hReplicatingOtherHeroModel")->offset;
    m_lifeState = Netvars->Get((ui)"m_lifeState")->offset;
    std::cout  << m_flStartSequenceCycle << std::endl;
    std::cout  << m_iTeamNum << std::endl;
}

void ExitHack()
{
    // Unhook entity
    entityVMT->RevertVMT(entity);
}

bool isEntityPopulated()
{
    return (Heroes.size() > 0);
}

int localIndex = -1;
int& GetLocalPlayer(int& = localIndex, int screen = 0) {
    typedef int& (*Fn)(void*, int&, int);
    return getvfunc<Fn>(engine, 22)(engine, localIndex, screen);
}
std::vector<ENTITY*> testEnt;
//int testt2 = 0;
void test()
{
    GetLocalPlayer(localIndex);
    short localEnt = localIndex + 1;

    if (testEnt.size()==0)
    {
        for (size_t i = 0; i < Heroes.size(); i++)
        {
            testEnt.push_back((ENTITY*)((u64)Heroes[i]));
            int handle = *(int*)((u64)testEnt[i] + m_hOwnerEntity);
            handle = handle & 0x7FFF;
            std::cout << "FOUND handle = " << handle << " size = " << i << std::endl;
        }
    }
    std::cout << "FOUND localindex = " << localEnt << " size = " << testEnt.size() << std::endl;
    //for (size_t i = 0; i < testEnt.size(); i++)
    //{
    //    CHANDLE handle = *(CHANDLE*)((u64)testEnt[i] + m_hOwnerEntity);
    //    localEnt = & 0x7fff;
    //    std::cout << "LocalENT = " << localEnt << " size = " << i << std::endl;
    //    if (testEnt[i]->Index() == localEnt) {
    //        std::cout << "FOUND !" << (u64)testEnt[i] << std::endl;
    //        continue;
    //    }
    //}
    
}

int getVBE() {
    if (Heroes.size() == 0) // check if entity is populated
    {
        return -1;
    }

    auto VBE = *(int*)(1); // vbeoffset = CEntityInstance+0x16B0
    if (VBE == 0)
    {
        return 0;
    }
    return 1;
}

ICvar* cvar = reinterpret_cast<ICvar*>(utilities::GetInterface("tier0.dll", "VEngineCvar007"));
void ConVars::InitConvars() {
    std::cout << "Found cvar address = " << cvar << std::endl;
    this->sv_cheats = cvar->FindCommandBase("sv_cheats");
    this->camera_distance = cvar->FindCommandBase("dota_camera_distance");
    this->drawrange = cvar->FindCommandBase("dota_range_display");
    this->r_farz = cvar->FindCommandBase("r_farz");
    this->fog_enable = cvar->FindCommandBase("fog_enable");
    this->weather = cvar->FindCommandBase("cl_weather");
    this->particle_hack = cvar->FindCommandBase("dota_use_particle_fow");
}

void ConVars::ResetConvars()
{
    if (cvar != nullptr)
    {
        this->weather->SetValue(0);
        this->camera_distance->SetValue(1200);
        this->drawrange->SetValue(0);
        this->r_farz->SetValue(-1);
        this->fog_enable->SetValue(false);
        this->particle_hack->SetValue(false);
        this->sv_cheats->SetValue(0);
    }
}

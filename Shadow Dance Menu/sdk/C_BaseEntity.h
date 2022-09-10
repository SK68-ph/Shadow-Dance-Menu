#pragma once

class CEntityInstance;
u64 m_iTeamNum, m_hOwnerEntity, m_flStartSequenceCycle, m_fGameTime, m_nGameState, m_lifeState,
m_iGameMode, m_clrRender, m_hReplicatingOtherHeroModel;

struct SchemaClassBinding {
    SchemaClassBinding* parent; // I THINK
    const char* bindingName; // ex: C_World
    const char* dllName; // ex: libclient.so
    const char* libName; // ex: client
};

class CHANDLE {
public:
    int handle;
    short Index() {
        return handle & 0x7fff;
    }
};
class CENTITYIDENTITY {
public:
    CEntityInstance* entity;//0
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

class CEntityInstance
{
public:
    virtual SchemaClassBinding* Schema_DynamicBinding(void);
    CENTITYIDENTITY* identity;
    CEntityInstance* Next() {
        if (!identity->m_pNext) return 0;
        return identity->m_pNext->entity;
    }
    int CHandle() {
        return identity->handle.handle;
    }
    short Index() {
        return identity->handle.Index();
    }
    int OwnerIndex()
    {
        return (*(int*)((u64)this + m_hOwnerEntity)) & 0x7FFF;
    }
    int IsAlive() {
        //enum LifeState : int
        //{
        //    UnitAlive = 0, KillCam = 1, UnitDead = 2
        //};
        return *(int*)((u64)this + m_lifeState);
    }
    float IsVisibleByEnemy() {
        return *(float*)((u64)this + m_flStartSequenceCycle);
    }
};


// vbe = CEntityInstance = 16B0

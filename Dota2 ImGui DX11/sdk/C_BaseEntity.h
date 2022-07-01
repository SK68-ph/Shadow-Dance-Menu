#pragma once
#include "utilities.h"

class C_BaseEntity;
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
    C_BaseEntity* entity;//0
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

class C_BaseEntity
{
public:
    virtual SchemaClassBinding* Schema_DynamicBinding(void);
    CENTITYIDENTITY* identity;
    C_BaseEntity* Next() {
        if (!identity->m_pNext) return 0;
        return identity->m_pNext->entity;
    }
    int CHandle() {
        return identity->handle.handle;
    }
    short Index() {
        return identity->handle.Index();
    }
    int Team() {
        return *(int*)((u64)this + m_iTeamNum);
    }
    cc BaseClass() {
        if (
            !identity || !identity->baseinfo
            ) return 0;
        return **(const char***)(
            identity->baseinfo
            );
    }
    short OwnerIndex()
    {
        return (*(short*)((u64)this + m_hOwnerEntity)) & 0x7FFF;
    }
    bool OfBaseClass(cc bc) {
        return StringsMatch(BaseClass(), bc);
    }
    bool IsHero() {
        return OfBaseClass("C_DOTA_BaseNPC_Hero");
    }
    bool IsIllusion() {
        return *(int*)((u64)this + m_hReplicatingOtherHeroModel) != -1;
    }
    bool IsAlive() {
        enum LifeState : int
        {
            UnitAlive = 0, KillCam = 1, UnitDead = 2
        };
        return *(int*)((u64)this + m_lifeState) == UnitAlive;
    }
    bool IsVisibleByEnemy() {
        float vbe = *(float*)((u64)this + m_flStartSequenceCycle);
        if (vbe >= 0.001 && vbe <= 1)
        {
            return false;
        }
        return true;
    }
    cc name() {
        return identity->name;
    }
    cc designername() {
        return identity->designer_name;
    }
};


// vbe = CEntityInstance = 16B0

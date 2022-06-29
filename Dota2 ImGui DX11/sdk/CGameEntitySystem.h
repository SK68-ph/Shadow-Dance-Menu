#pragma once

#include "CEntityInstance.h"


#define EntityHandle unsigned int
#define MAX_ENTITIES_IN_LIST 512
#define MAX_ENTITY_LISTS 64
#define MAX_TOTAL_ENTITIES MAX_ENTITIES_IN_LIST * MAX_ENTITY_LISTS


class CEntityIdentity {

public:
	CEntityInstance* entity;
	void* dunno;
	int64_t  unk0;
	int64_t  unk1;
	const char* internalName;
	const char* entityName;
	void* unk2;
	void* unk3;
	void* unk4;
	void* unk5;
	CEntityIdentity* prevValid;
	CEntityIdentity* nextValid;
	void* unkptr;
	void* unkptr2;
	void* unkptr3;
};

class CEntityIdentities
{
public:
	CEntityIdentity m_pIdentities[MAX_ENTITIES_IN_LIST];
};

class EntityIdentityList
{
public:
	CEntityIdentities* m_pIdentityList;
};

#define ClearEntityDatabaseMode_t char
#define CEntityHandle unsigned int
#define EntitySpawnInfo_t char

class CGameEntitySystem {
public:
	virtual void n_0();
	virtual void BuildResourceManifest(void); // 01
	virtual void n_2();
	virtual void n_3();
	virtual void n_4();
	virtual void n_5();
	virtual void n_6();
	virtual void AddRefKeyValues(); // 7
	virtual void ReleaseKeyValues(); // 8
	virtual void n_9();
	virtual void n_10();
	virtual void ClearEntityDatabase(void); // 11
	virtual CEntityInstance* FindEntityProcedural(const char *...);
	virtual CEntityInstance* OnEntityParentChanged(CEntityInstance*, CEntityInstance*); //13
	virtual CEntityInstance* OnAddEntity(CEntityInstance*, CEntityHandle); // 14
	virtual CEntityInstance* OnRemoveEntity(CEntityInstance*, CEntityHandle); // 15
	virtual void n_16();
	virtual void SortEntities(int, EntitySpawnInfo_t*, int*, int*); // 17
	virtual void n_18();
	virtual void n_19();
	virtual void n_20();
	virtual void n_21();

	void* unk;
	CEntityIdentities* m_pEntityList[MAX_ENTITY_LISTS];

	CEntityInstance* GetBaseEntity(int index)
	{
		if (index <= -1 || index >= MAX_TOTAL_ENTITIES)
			return nullptr;

		int listToUse = (index / MAX_ENTITIES_IN_LIST);
		if (!m_pEntityList[listToUse])
			return nullptr;

		if (m_pEntityList[listToUse]->m_pIdentities[index % MAX_ENTITIES_IN_LIST].entity)
			return m_pEntityList[listToUse]->m_pIdentities[index % MAX_ENTITIES_IN_LIST].entity;
		else
			return nullptr;
	}

	int GetHighestEntityIndex()
	{
		// 33 FF xor edi, edi
		// 8B 81 (? ? ? ?) 
		// FF C0 inc eax
		// 85 C0 test eax, eax
		// "Ent %3d: %s class %s name %s\n"
		/*
		v0 = g_pGameEntitySystem_0;
		v1 = 0;
		result = (unsigned int)(*(_DWORD *)(g_pGameEntitySystem_0 + 0x2080) + 1);
		if ( (signed int)result > 0 )
		*/
		return *(int*)((uintptr_t)this + 0x2084);
	}
};
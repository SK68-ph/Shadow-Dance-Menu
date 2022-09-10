#pragma once
#include "C_BaseEntity.h"


#define EntityHandle unsigned int
#define MAX_ENTITIES_IN_LIST 512
#define MAX_ENTITY_LISTS 64
#define MAX_TOTAL_ENTITIES MAX_ENTITIES_IN_LIST * MAX_ENTITY_LISTS
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

};
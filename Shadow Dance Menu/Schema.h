#pragma once
#include "common.h"
u64 SchemaSystem;

int vstrcmp(u64 a, u64 b) {
    return strcmp((cc)a, (cc)b);
}
int vstrcmp(cc a, u64 b) {
    return strcmp(a, (cc)b);
}
int vstrcmp(u64 a, cc b) {
    return strcmp((cc)a, b);
}
int vstrcmp(cc a, cc b) {
    return strcmp(a, b);
}
typedef void(__fastcall* ConMsg)(cc, u64, u64, u64);
ConMsg CMsg = 0;
void CMSG(cc pure) {

    CMsg(pure, 0, 0, 0);

}

void CMSG(cc format, u64 p1) {

    CMsg(format, p1, 0, 0);

}

void CMSG(cc format, u64 p1, u64 p2) {

    CMsg(format, p1, p2, 0);

}

template<typename T, u64 SIZE>
class CArray {
public:
    T* elms[SIZE] = { 0 };
    u64 count = 0;
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
        for (u64 i = 0; i < count; i++) {
            if (elms[i] == elm) {
                T* aa = elms[i];
                for (u64 j = i; j < count; j++) {
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
        for (u64 i = 0; i < count; i++) {
            delete elms[i];
        }
    }
};
struct ClassDescription;
struct SchemaParent {
    u64 idk;
    ClassDescription* parent;
};
struct ClassDescription {
    u64 idk;//0
    u64 classname;//8
    u64 modulename;//10
    int sizeofclass;//18
    short memberstoiterate;//1c
    char pad[6];//20
    u64 MemberInfo;//28
    u64 idk2;//30
    SchemaParent* parent;//38
};

struct SchemaTypeDescription {
    u64 idk;
    u64 name;
    u64 idk2;
};
struct MemberDescription {
    u64 name;
    SchemaTypeDescription* schematypeptr;
    int offset;
    int idk;
    u64 idk2;
};
class schemanetvar {
public:
    u64 classname;
    u64 name;
    u64 _typename;
    int offset;
    schemanetvar(u64 a, u64 b, u64 c, u64 d) {
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
        u64 Scope = ((u64(__fastcall*)(u64 schemasys, const char* _mod))
            (*(u64*)(*(u64*)(SchemaSystem)+0x68)))(SchemaSystem, _module);
        if (!Scope) { CMSG("No such scope %s!\n", (u64)_module); return; }
        u64 Class = ((u64(__fastcall*)(u64 scope, const char* _class))
            (*(u64*)(*(u64*)(Scope)+0x10)))(Scope, _class);
        if (!Class) { CMSG("No such class %s!\n", (u64)_class); return; }
        ClassDescription* a = (ClassDescription*)Class;
        for (u64 i = 0; i < a->memberstoiterate; i++) {
            MemberDescription* z = (MemberDescription*)(a->MemberInfo + i * 0x20);
            Netvars = new schemanetvar(a->classname, z->name, z->schematypeptr->name, (u64)z->offset);
        }
    }
    schemanetvar* Get(u64 name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);
    }
    schemanetvar* Get(u64 _class, u64 name) {
        for (schemanetvar* netvar : Netvars) {
            if (!vstrcmp(netvar->name, name) && !vstrcmp(netvar->classname, _class)) return netvar;
        }
        CMSG("no such netvar found in manager: %s\n", name);

    }
};
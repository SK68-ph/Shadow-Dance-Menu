#pragma once
#include "color.h"

class ConCommandBase {
public:
    virtual void DESTROY1() = 0;
    virtual void unk() = 0;
    virtual bool IsCommand(void) = 0;
    virtual bool IsBoundedVar(void) = 0;
    virtual bool IsFlagSet(long long) = 0;
    virtual void AddFlags(long long) = 0;
    virtual void RemoveFlags(long long) = 0;
    virtual long long GetFlags(void) = 0;
    virtual const char* GetName(void) = 0;
    virtual const char* GetHelpText(void) = 0;
    virtual bool IsRegistered(void) = 0;
    virtual void* GetDLLIdentifier(void) = 0;
    virtual void Create(char const*, char const*, long long) = 0;
    virtual void Init(void) = 0;
    virtual const char* GetBaseName(void) = 0;
    virtual int GetSplitScreenPlayerSlot(void) = 0;
    virtual void SetValue(char const*) = 0; // 16
    virtual void SetValue(float) = 0; // 17
    virtual void SetValue(int) = 0; // 18
    virtual void SetValue(ColorRGBA) = 0;
    virtual float GetFloat(void) = 0;
    virtual int GetInt(void) = 0;
    virtual bool GetBool(void) = 0;
    virtual void InternalSetValue(char const*) = 0;
    virtual void InternalSetFloatValue(float) = 0;
    virtual void InternalSetIntValue(int) = 0;
    virtual void InternalSetColorValue(ColorRGBA) = 0;
    virtual void ClampValue(float&) = 0;
    virtual void ChangeStringValue(char const*, float) = 0;
    virtual void Create() = 0;
};

class ICvar {
public:
    ConCommandBase* FindCommandBase(char const* convar) {
        typedef ConCommandBase* (*Fn)(void*, char const*);
        return getvfunc<Fn>(this, 16)(this, convar); // 16 in vtable
    }
};
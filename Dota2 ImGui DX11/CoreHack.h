#pragma once
#include "includes.h"
#include "utilities.h"
#include "common.h"
#include "sdk/ICVar.h"
#include "sdk/CGameEntitySystem.h"
#include "sdk/IAppSystem.h"
#include "sdk/IEngineClient.h"
#include "vmt.h"


int getVBE();
void InitHack();
void ExitHack();
bool isEntityPopulated();
void test();

class ConVars {
public:
    void InitConvars();
    void ResetConvars();
    ConCommandBase* sv_cheats;
    ConCommandBase* camera_distance;
    ConCommandBase* drawrange;
    ConCommandBase* r_farz;
    ConCommandBase* fog_enable;
    ConCommandBase* weather;
    ConCommandBase* particle_hack;
};

#pragma once
#include "includes.h"
#include "utilities.h"
#include "common.h"
#include "ICVar.h"


namespace Hack
{
	int getVBE();
    void InitHack();
    int SaveOffsetConfig();
    int LoadOffsetConfig();
    int ScanVbeOffset(bool firstScan);

    class ConVars {
    public:
        void InitConvars();

        ConCommandBase* sv_cheats;
        ConCommandBase* camera_distance;
        ConCommandBase* drawrange;
        ConCommandBase* r_farz;
        ConCommandBase* fog_enable;
        ConCommandBase* weather;
        ConCommandBase* particle_hack;
    };

}
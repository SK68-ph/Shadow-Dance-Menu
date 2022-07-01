#pragma once
#include "includes.h"
#include "utilities.h"
#include "vmt.h"


int getVBE();
void InitHack();
void ExitHack();
bool isEntityPopulated();
void InitConvars();
void ResetConvars();


void SetWeather(int val);
void SetDrawRange(int val);
void SetParticleHack(int val);
void SetNoFog(int val);
void SetCamDistance(int val);
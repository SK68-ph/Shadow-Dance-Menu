#pragma once
#include "includes.h"


int getVBE();
void InitHack();
void RemoveVmtHooks();
bool isEntityPopulated();
void InitConvars();
void ResetConvars();


void SetWeather(int val);
void SetDrawRange(int val);
void SetParticleHack(int val);
void SetNoFog(int val);
void SetCamDistance(int val);
#pragma once
#include "offsets.hpp"
#include "config.h"
#include <Windows.h>
#include <iostream>

struct vec3 { float x = 0, y = 0, z = 0; }; // x - yaw, y - pitch, z - roll

extern DWORD Client;
extern DWORD Engine;
extern DWORD LocalPlayer;
extern DWORD GlowObj;
extern DWORD EntityList;
extern vec3* viewAngles;
extern int myTeam;


namespace Hack
{
	void Glow();
	void Aimbot();
}
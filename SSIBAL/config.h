#pragma once
#include <Windows.h>

namespace Config
{
	extern bool bMenu;
	extern bool bGlow;
	extern bool bTestAimbot;
	extern int bShooth;
	extern bool bESPBox;
	extern bool bRectAngle;

	void Option_FALSE();
	void Vkey_Input(BYTE vk);
}
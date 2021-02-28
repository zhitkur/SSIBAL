#include "config.h"

bool Config::bMenu = false;
bool Config::bGlow = false;
bool Config::bTestAimbot = false;
bool Config::bRectAngle = false;
bool Config::bESPBox = false;
int Config::bShooth = 0;

void Config::Option_FALSE()
{
	Config::bMenu = false;
	Config::bGlow = false;
	Config::bTestAimbot = false;
	Config::bRectAngle = false;
	Config::bShooth = 0;
}

void Config::Vkey_Input(BYTE vk)
{
	INPUT input;
	ZeroMemory(&input, sizeof(INPUT));
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;

	SendInput(1, &input, sizeof(INPUT));

	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}
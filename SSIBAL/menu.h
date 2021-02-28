#pragma once

#include "hook.h"
#include "config.h"
#include "utils.h"
#include "ImGui/imgui_internal.h"

extern ImFont*		g_pDefaultFont;
extern ImFont*		g_pSecondFont;
inline ImGuiStyle		style;

namespace Menu
{
	void GetFonts();
	void Menu();
	void CreateStyle();
	void Shutdown();
}
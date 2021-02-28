#pragma once

#include <iostream>
#include <Windows.h>

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx9.h"

#include "Patternscaning.h"
#include "detours.h"
#pragma comment(lib, "detours.lib")

#include "config.h"
#include "fonts/fonts.hpp"
#include "utils.h"
#include "menu.h"
#include "hook.h"

inline ImFont* g_pDefaultFont = nullptr;
inline ImFont* g_pSecondFont = nullptr;

inline IDirect3D9* pD3D = nullptr;
inline IDirect3DDevice9* Device = nullptr;

inline ID3DXFont* default_font = nullptr;

static HWND window;

typedef HRESULT(__stdcall* f_EndScene)(IDirect3DDevice9* pDevice);
inline f_EndScene oEndScene;	// Original EndScene;

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

inline WNDPROC oWndProc = nullptr;

inline int windowHeight, windowWidth;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void drawText(ID3DXFont* font, LPCSTR text, int i, int t, int r, int b, D3DCOLOR color);

BOOL WINAPI InitD3D9();

void d3d9_shutdown();
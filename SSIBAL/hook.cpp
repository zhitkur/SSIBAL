#include "hook.h"

const char* windowName = "Counter-Strike: Global Offensive";

void drawText(ID3DXFont* font, LPCSTR text, int i, int t, int r, int b, D3DCOLOR color)
{
	RECT font_rect{ 0 };
	SetRect(&font_rect, i, t, r, b);

	font->DrawText(NULL, text, -1, &font_rect, DT_LEFT | DT_NOCLIP, color);
}

void DrawFilledRect(int x, int y, int w, int h)
{
	RECT rect = { x, y, x + w, y + h };
	
}

void draw_render(IDirect3DDevice9* pDevice)
{
	if (Config::bRectAngle)
	{
		D3DRECT BarRect = { 25, 55, 300, 300 };
		pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 255, 0), 0.0f, 0);
		drawText(default_font, "digitalplumber009", 10, 10, 150, 150, D3DCOLOR_ARGB(255, 255, 255, 255));
	}
}


// 메시지 핸들을 처리해서 GUI에 마우스를 사용할 수 있도록 함
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (Config::bMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
{
	static bool init = true;
	if (init)
	{
		D3DXCreateFont(pDevice, 19, 0, FW_BOLD, 1, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			(LPCSTR)"Tahoma", &default_font);

		init = false;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplWin32_Init(FindWindowA(NULL, windowName));
		ImGui_ImplDX9_Init(pDevice);
	}

	drawText(default_font, "SSIBAL v0.1", 3, 3, 150, 150, D3DCOLOR_ARGB(255, 127, 255, 212));
	draw_render(pDevice);

	Menu::Menu();
	Menu::CreateStyle();

	//ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);		// Call Original EndScene so the game can draw
}

BOOL WINAPI InitD3D9()
{
	window = FindWindowA(NULL, windowName);
	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));	// 창을 이동시킬 수 있도록 함

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp{ 0 };
	d3dpp.hDeviceWindow = window, d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD, d3dpp.Windowed = TRUE;

	if (FAILED(pD3D->CreateDevice(0, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &Device)))
		return false;

	if (pD3D)
		pD3D->Release(), pD3D = nullptr;



	void** pVTable = *reinterpret_cast<void***>(Device);
	oEndScene = (f_EndScene)DetourFunction((PBYTE)pVTable[42], (PBYTE)Hooked_EndScene);

	if(Device)
		Device->Release(), Device = nullptr;

	return false;
}

void d3d9_shutdown()
{
	SetWindowLongPtr(window, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
	DetourRemove((PBYTE)oEndScene, (PBYTE)Hooked_EndScene);
}
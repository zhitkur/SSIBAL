#include <Windows.h>
#include "utils.h"
#include "hook.h"
#include "hack.hpp"

BOOL WINAPI OnDllDetach();

DWORD WINAPI MainThread(LPVOID param)
{
    while (!GetModuleHandleA("serverbrowser.dll"))
        Sleep(1000);

#ifdef _DEBUG
    Console::Init();
    Utils::set_random_title();
    Console::Setsize();
#endif

    try {
        Console::MessageStatus("Thank you for purchasing SSIBAL v0.1\n", Good);
        Console::MessageStatus("Initializing...\n\n");

        Console::Message(LightBlue, "[+]", White, "Client Address     : "); printf("0x%X\n", Client);
        Console::Message(LightBlue, "[+]", White, "Engine Address     : "); printf("0x%X\n", Engine);
        Console::Message(LightBlue, "[+]", White, "ViewAngle Address  : "); printf("0x%X\n", *viewAngles);
        Console::Message(LightBlue, "[+]", White, "EntityList Address : "); printf("0x%X\n", EntityList);
        Console::Message(LightBlue, "[+]", White, "GlowObject Address : "); printf("0x%X\n\n", GlowObj);
    }

    catch (const char* e) {
        Console::MessageStatus(e, Error);
    }

    // Init Something...
    HWND console = GetConsoleWindow();
    InitD3D9();

    Console::MessageStatus("Finished. \n", Good);
    Console::MessageStatus("Built on: "); printf("%s %s\n\n", __DATE__, __TIME__);
    Console::Message("If you want to turn off this window, press the", Turqoise, " Home ", White, "key\n\n");
    Console::Message(Red, "[*]", White, "Enjoy! :)");

    while (!(GetAsyncKeyState(VK_END) & 1))
    {
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            Config::bMenu = !Config::bMenu;
        }

        if (GetAsyncKeyState(VK_HOME) & 1)
        {
            ShowWindow(console, SW_HIDE);
        }
        Hack::Aimbot();
        Hack::Glow();
        Sleep(1);
    }

    if(console != SW_HIDE)
        ShowWindow(console, SW_HIDE); 

    FreeLibraryAndExitThread(static_cast<HMODULE>(param), 0);
    return false;   
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Console::Detach();
#endif

    Config::Option_FALSE();
    Menu::Shutdown();
    d3d9_shutdown();
    
    return TRUE;
}

BOOL APIENTRY DllMain(
    _In_         HMODULE     hModule, 
    _In_         DWORD      dwReason,
    _In_opt_     LPVOID     lpReserved
)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_DETACH)
    {
        return OnDllDetach();
    }

    return TRUE;
}


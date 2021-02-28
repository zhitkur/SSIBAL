#include <Windows.h>
#include <iostream>
#include <fstream>
#include <TlHelp32.h>
#include <string>
#include <shlobj.h> // IsUserAnAdmin()
#include <conio.h>
#include "termcolor.h"

const int WindowWidth = GetSystemMetrics(SM_CXSCREEN);
const int WindowHeight = GetSystemMetrics(SM_CYSCREEN);

class tool
{
public:
    void set_location_console(int w, int h)
    {
        HWND console = GetConsoleWindow();

        MoveWindow(console, WindowWidth / 2 - 200, WindowHeight / 2 - 100, w, h, TRUE);
    }

    void setup_console(std::string consoletitle, HWND console, int w = 400, int h = 200)
    {
        SetConsoleTitleA(consoletitle.c_str());
        //SetWindowPos(console, HWND_TOPMOST, 0, 0, w, h, SWP_NOMOVE);
        set_location_console(w, h);
        std::cout << termcolor::green << "        //---SSIBAL LOADER---\\\\" << termcolor::white << std::endl << std::endl;
    }

    void HideText()
    {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;

        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    }
};

// 해당 dll 파일이 존재하는지 확인
bool file_exists(const char* name)
{
    std::ifstream infile{ name };
    return infile.good();
}

// 해당 프로세스가 존재하는지 확인하여 해당 프로세스의 pid를 획득하는 함수
bool process_exists(const char* name, uint32_t& pid)
{
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    auto entry = PROCESSENTRY32{ sizeof(PROCESSENTRY32) };

    if (Process32First(snapshot, &entry)) {
        do {
            if (!strcmp(entry.szExeFile, name)) {
                pid = entry.th32ProcessID;
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return false;
}

// 해당 프로세스에 대한 권한 획득
bool enable_debug_privilege(HANDLE process)
{
    LUID luid;
    HANDLE token;
    TOKEN_PRIVILEGES newPrivileges;

    // 현재 프로세스의 권환을 변경하기 위한 액세스 토큰의 핸들을 가져온다
    if (!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token))
        return false;

    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid))
        return false;

    newPrivileges.PrivilegeCount = 1;
    newPrivileges.Privileges[0].Luid = luid;
    newPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // AdjustTokenPrivileges() - 해당 프로세스 접근 권한 얻는 함수
    return AdjustTokenPrivileges(
        token,                     // TokenHandle
        FALSE,                     // DisableAllPrivileges
        &newPrivileges,            // NewPrivileges
        sizeof(newPrivileges),     // BufferLength
        nullptr,                   // PreviousState (OPTIONAL)
        nullptr                    // ReturnLength (OPTIONAL)
    );
}

// 해당 프로세스의 pid를 사용하여 프로세스에 접근하는 함수
bool process_open(uint32_t pid, HANDLE& handle)
{
    handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
        PROCESS_VM_READ | PROCESS_VM_WRITE |
        PROCESS_CREATE_THREAD, FALSE, pid);

    return handle != nullptr;
}

bool inject(HANDLE process, const char* dll)
{
    auto thread = HANDLE{ nullptr };
    auto exit_code = 0;
    auto dllpath = VirtualAllocEx(process, nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!dllpath)
        goto fail;

    auto success = WriteProcessMemory(process, dllpath, dll, (strlen(dll) + 1) * sizeof(wchar_t), nullptr);

    if (!success)
        goto fail;

    // CreateRemoteThread() - Thread를 생성하는 함수인데, 이 함수를 호출하는 프로세스가 아닌 다른 프로세스에 Thread를 생성하는 기능을 가지고 있음
    // DLL을 읽어서 실행가능한 영역에 적재하는 함수인 LoadLibrary() 함수를 사용해 Inject한다.
    thread = CreateRemoteThread(process, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryA, dllpath, 0, nullptr);

    if (!thread) {
        goto fail;
    }

    // WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) - Thread가 특정 Signal이 발생하면 작업을 수행하고, 대기 상태로 돌아가는 걸 반복하는 함수
    // 참고로, Thread가 종료하면 WAIT_OBJECT_0을 리턴함. 고로 아래 if문은 Thread가 종료되면 GetExitCodeThread() 함수를 사용하여 상태(종료 여부)를 반환한다.
    if (WaitForSingleObject(thread, 4000) == WAIT_OBJECT_0) {
        exit_code = 0;
        GetExitCodeThread(thread, (DWORD*)&exit_code);
    }

fail:
    if (thread)
        CloseHandle(thread);
    if (dllpath)
        VirtualFreeEx(process, dllpath, 0, MEM_RELEASE);

    return exit_code != 0;
}

int main()
{

    tool* tools = new tool;
    // 관리자 권한으로 실행하는지 확인
    /*if (!IsUserAnAdmin())
    {
        std::cout << "[ERROR] Launch as admin." << '\n';
        std::cout << "Press any key to exit..." << '\n';
        std::cin.get();
        return EXIT_FAILURE;
    }*/

    HWND console = GetConsoleWindow();
    tools->setup_console("SSIBAL LOADER", console);


    constexpr auto TARGET_FILE = "SSIBAL.dll";
    constexpr auto TARGET_PROCESS = "csgo.exe";

    char    fullpath[MAX_PATH] = { 0 };
    auto    proc_id = 0u;
    auto    proc_handle = HANDLE{ nullptr };

    enable_debug_privilege(GetCurrentProcess());

    try {
        if (!file_exists(TARGET_FILE))
            throw std::runtime_error{ "DLL not found." };

        if (!process_exists(TARGET_PROCESS, proc_id))
            throw std::runtime_error{ "Process is not running." };

        if (!process_open(proc_id, proc_handle)) {
            throw std::runtime_error{ "Failed to open process." };
        }

        std::string user;
        std::string pass;
        std::string Code;

        std::cout << termcolor::blue << " [*] Code: " << termcolor::white;
        do
        {
            getline(std::cin, Code);
        } while (Code.empty());

        if (Code.size() != 11) return 0;
        if (Code != "testcode123") return 0;

        std::cout << termcolor::cyan << " [*] Username : " << termcolor::white;

        do
        {
            getline(std::cin, user);
        } while (user.empty());

        if (user.size() != 4) return 0;
        //if (user[0] != 'z' || user[1] != 'h' || user[2] != 'i' || user[3] != 't' || user[4] != 'k' || user[5] != 'u' || user[6] != 'r') return 0;
        if (user != "test") return 0;
        tools->HideText();
        std::cout << termcolor::cyan << " [*] Password : " << termcolor::white;

        do
        {
            getline(std::cin, pass);
        } while (pass.empty());

        std::cout << "\n";
		if (pass.size() != 4) return 0;
        if (pass[0] != 'f' || pass[1] != 'o' || pass[2] != 'w' || pass[3] != 'w') return 0;


        std::cout << termcolor::yellow << "[+] Please wait a moment...\n";
        std::cout << termcolor::yellow << "[+] Loading...\n" << termcolor::white;
        Sleep(2000);

        // _wfullpath() - 상대 경로를 절대 경로 이름으로 확장하는 함수
        _fullpath(fullpath, TARGET_FILE, MAX_PATH);

        if (!inject(proc_handle, fullpath)) {
            throw std::runtime_error{ "Failed to inject DLL." };
        }

    }
    catch (const std::exception& ex) {
        std::cout << "[ERROR] " << ex.what() << '\n';
        std::cout << "Press any key to exit..." << '\n';
        std::cin.get();
        return EXIT_FAILURE;
    }
    delete tools;
    return EXIT_SUCCESS;
}

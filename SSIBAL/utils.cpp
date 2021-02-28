#include "utils.h"
#pragma warning( disable : 4996 )

std::ofstream Console::internal::logger;

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

void Console::Init(const char* ConTitle, bool logToFile, bool detailed)
{
    internal::shouldLog = logToFile;
    internal::detailedLog = detailed;

    if (GetConsoleWindow() == NULL)
    {
        if (!AllocConsole())
        {
            MessageBox(NULL, ((std::string)ConTitle + " failed to Allocate Console").c_str(), NULL, MB_ICONEXCLAMATION);
            return;
        }
        if (freopen("CONOUT$", "w", stdout) == NULL) {
            MessageBox(NULL, ((std::string)ConTitle + " failed to load: stdout").c_str(), NULL, MB_ICONEXCLAMATION);
            return;
        }
        /*if (freopen("CONOUT$", "w", stderr)) {
            MessageBox(NULL, ((std::string)ConTitle + " failed to load: stderr").c_str(), NULL, MB_ICONEXCLAMATION);
            return;
        }*/
       /* if (freopen("CONIN$", "r", stdin)) {
            MessageBox(NULL, ((std::string)ConTitle + " failed to load: stdin").c_str(), NULL, MB_ICONEXCLAMATION);
            return;
        }*/
    }
    SetConsoleTitle(ConTitle);
    if (internal::shouldLog) {
        bool exists = true;
        std::ifstream file(__DATE__);
        int idx = 1;
        do
        {
            std::ifstream file((std::string)__DATE__ + " " + std::to_string(idx) + ".txt");
            if (!file.good())
                exists = false;
            else
                idx++;
        } while (exists);

        internal::logger.open(((std::string)__DATE__ + " " + std::to_string(idx) + ".txt").c_str());
        internal::logger << __TIMESTAMP__ << std::endl;
    }
}

void Console::Detach()
{
    if (internal::logger.is_open())
    {
        internal::logger.close();
    }
    FreeConsole();
}

void Console::Setsize()
{
    int x = 585, y = 400;

    HWND hWnd = GetConsoleWindow();
    MoveWindow(hWnd, 660, 340, x, y, TRUE);
    SetWindowPos(hWnd, HWND_TOP, 0, 0, 10, 10, SWP_NOSIZE);

    LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
    lStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);

    SetWindowLong(hWnd, GWL_STYLE, lStyle);

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    //CONSOLE_FONT_INFOEX cfi;

    //cfi.cbSize = sizeof(cfi);
    //cfi.nFont = 0;
    //cfi.dwFontSize.X = 0;                   // Width of each character in the font
    //cfi.dwFontSize.Y = 22;                  // Height
    //cfi.FontFamily = FF_DONTCARE;
    //cfi.FontWeight = FW_NORMAL;
    //wcscpy_s(cfi.FaceName, L"Tahoma"); // Choose your font
    //SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(console, &csbi);
    COORD scrollbar = {
        csbi.srWindow.Right - csbi.srWindow.Left + 1,
        csbi.srWindow.Bottom - csbi.srWindow.Top + 1
    };

    if (console == 0)
        throw 0;
    else
        SetConsoleScreenBufferSize(console, scrollbar);

    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, (255 * 96) / 100, LWA_ALPHA);
}

bool Console::MessageStatus(const char* msg, ConStatus state)
{
    ConColor clr = White;
    switch (state) {
    case Ok:
        clr = DarkGray;
        break;
    case Good:
        clr = LightGreen;
        break;
    case Warning:
        clr = Yellow;
        break;
    case Error:
        clr = Red;
        break;
    default:
        clr = White;
        break;
    }
    if (internal::detailedLog) {
        std::time_t now = std::time(0);
        std::tm* now_tm = std::gmtime(&now);
        char buf[42];
        std::strftime(buf, 42, "%Y-%m-%d %H:%M:%S ", now_tm);
        return Message(buf, "[", clr, internal::ConStatusArray[state], White, "] ", msg);
    }
    return Message("[", clr, internal::ConStatusArray[state], White, "] ", msg);
}

namespace Utils {

    void InitConsole()
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);   // 11 - 0x000B 하늘색
        Utils::ConsolePrint("Thank you for purchasing SSIBAL\n");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);   // 14 - 0x000E 노란색
        Utils::ConsolePrint("Initializing...\n\n");
    }

    void InitFinishConsole()
    {
        Utils::ConsolePrint("Finished. \n");
        Utils::ConsolePrint("Built on: %s %s\n\n", __DATE__, __TIME__);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);   // 15 - 0x000F 흰색

        /*Sleep(4000);
        system("cls");*/
    }

    void set_random_title()
    {
        srand((unsigned int)time(NULL));
        const char* maps = "QAZXSWEDCVFRTGBNHYUJMKIOLPqwertyuiopasdfghjklzxcvbnm123654789";
        char title[100]{ "title " };
        for (int i = 6; i < 30; i++) title[i] = maps[rand() % (strlen(maps) - 1)];
        system(title);
    }

    void set_font()
    {
        // 폰트 색상을 차례대로 변화시켜주는 함수, 하지만 loop문안에서 돌려야 정상적으로 작동함
        static int c = 2;
        c++;
        if (c == 15) c = 3;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
    }

    /*int get_fps()
    {
        using namespace std::chrono;
        static int count = 0;
        static auto last = high_resolution_clock::now();
        auto now = high_resolution_clock::now();
        static int fps = 0;

        count++;

        if (duration_cast<milliseconds>(now - last).count() > 1000)
        {
            fps = count;
            count = 0;
            last = now;
        }

        return fps;
    }*/

    void AttachConsole()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out = GetStdHandle(STD_OUTPUT_HANDLE);
        _err = GetStdHandle(STD_ERROR_HANDLE);
        _in = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);

        // 콘솔 색상 변경 (다른 색을 원한다면 BLUE, GREEN, RED를 원하는 색으로 섞어주면 됨)
        //SetConsoleTextAttribute(_out, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }

    void DetachConsole()
    {
        if (_out && _err && _in) {
            FreeConsole();

            if (_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if (_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if (_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

    char ConsoleReadKey()
    {
        if (!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }

    bool ConsolePrint(const char* fmt, ...)
    {
        if (!_out)
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }
};
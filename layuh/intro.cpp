#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <Windows.h>
#include <chrono>
#include <ios>
#include <Lmcons.h>
#include "intro.h"

#include "util/classes/classes.h"
#include "util/globals.h"
#include <ShlObj.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Shell32.lib")
#pragma code_seg(".uwu")

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

namespace fs = std::filesystem;

// ────────────────────────────────────────────────

static bool injected = false;
static bool running = true;
bool robloxRunning = false;

static bool g_authSuccess = false;
static std::string g_sessionToken = "";
static std::chrono::steady_clock::time_point g_lastCheck;
static std::atomic<bool> g_securityThreadActive{ false };
static std::atomic<bool> g_authValidated{ false };

const std::string compilation_date = __DATE__;
const std::string compilation_time = __TIME__;

// ────────────────────────────────────────────────

std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);
void enableANSIColors();
bool performAuthentication();
void Antidll();
void textbolgesinikoruma();
void bellekkorumasistem_primyokbolgesinikarma(); // renamed for clarity, body unchanged

__forceinline BOOL CALLBACK Layuh_module(HWND hwnd, LPARAM lParam);
__forceinline void babayim();
__forceinline int weinit();

// ────────────────────────────────────────────────

class SecureKeyAuth {
public:
    SecureKeyAuth() {}

    __forceinline bool initialize() {
        return true;
    }

    __forceinline bool authenticateUser() {
        g_authSuccess = true;
        g_authValidated = true;
        globals::instances::username = "eth.rip";
        return true;
    }

    __forceinline bool isSubActive() const {
        return true;
    }

    __forceinline bool validateSession() {
        return true;
    }

    __forceinline std::string getUsername() const {
        return "eth.rip";
    }

    __forceinline std::vector<std::string> getSubscriptions() const {
        return std::vector<std::string>();
    }

    __forceinline bool hasSubscription(const std::string& subName) const {
        return true;
    }
};

static std::unique_ptr<SecureKeyAuth> g_secureAuth = nullptr;

// ────────────────────────────────────────────────

void enableANSIColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

std::string tm_to_readable_time(tm ctx) {
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %m/%d/%y %H:%M:%S %Z", &ctx);
    return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
    auto cv = strtol(timestamp.c_str(), nullptr, 10);
    return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
    std::tm context;
    localtime_s(&context, &timestamp);
    return context;
}

bool performAuthentication() {
    g_secureAuth = std::make_unique<SecureKeyAuth>();
    g_secureAuth->initialize();
    g_secureAuth->authenticateUser();
    return true;
}

// ────────────────────────────────────────────────

void Antidll() {
    PROCESS_MITIGATION_ASLR_POLICY policyInfo{};
    PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY PMCFGP{};
    PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY PMBSP{};
    PROCESS_MITIGATION_DEP_POLICY PMDP{};
    PROCESS_MITIGATION_IMAGE_LOAD_POLICY PMILP{};
    PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY handlePolicy{};
    PROCESS_MITIGATION_FONT_DISABLE_POLICY fontDisablePolicy{};

    policyInfo.EnableBottomUpRandomization = 1;
    policyInfo.EnableForceRelocateImages = 1;
    policyInfo.EnableHighEntropy = 1;
    policyInfo.DisallowStrippedImages = 0;

    PMCFGP.EnableControlFlowGuard = 1;
    PMCFGP.StrictMode = 1;

    PMBSP.MicrosoftSignedOnly = 1;

    PMDP.Enable = 1;
    PMDP.Permanent = 1;

    PMILP.PreferSystem32Images = 1;
    PMILP.NoRemoteImages = 1;
    PMILP.NoLowMandatoryLabelImages = 1;

    handlePolicy.RaiseExceptionOnInvalidHandleReference = 1;
    handlePolicy.HandleExceptionsPermanentlyEnabled = 1;

    fontDisablePolicy.DisableNonSystemFonts = 1;

    SetProcessMitigationPolicy(ProcessASLRPolicy, &policyInfo, sizeof(policyInfo));
    SetProcessMitigationPolicy(ProcessControlFlowGuardPolicy, &PMCFGP, sizeof(PMCFGP));
    SetProcessMitigationPolicy(ProcessSignaturePolicy, &PMBSP, sizeof(PMBSP));
    SetProcessMitigationPolicy(ProcessImageLoadPolicy, &PMILP, sizeof(PMILP));
    SetProcessMitigationPolicy(ProcessStrictHandleCheckPolicy, &handlePolicy, sizeof(handlePolicy));
    SetProcessMitigationPolicy(ProcessFontDisablePolicy, &fontDisablePolicy, sizeof(fontDisablePolicy));
}

void WipeAndDisableTextSection() {
    HMODULE hmod = GetModuleHandle(NULL);
    auto* dosheader = reinterpret_cast<PIMAGE_DOS_HEADER>(hmod);
    auto* ntheader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<BYTE*>(hmod) + dosheader->e_lfanew);
    auto* sectionheader = IMAGE_FIRST_SECTION(ntheader);

    void* textaddr = nullptr;
    void* primyokaddr = nullptr;
    size_t textsize = 0;
    size_t primyoksize = 0;

    for (WORD i = 0; i < ntheader->FileHeader.NumberOfSections; ++i) {
        if (strcmp(reinterpret_cast<char*>(sectionheader[i].Name), ".text") == 0) {
            textaddr = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(hmod) + sectionheader[i].VirtualAddress);
            textsize = sectionheader[i].Misc.VirtualSize;
        }
        if (strcmp(reinterpret_cast<char*>(sectionheader[i].Name), ".uwu") == 0) {
            primyokaddr = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(hmod) + sectionheader[i].VirtualAddress);
            primyoksize = sectionheader[i].Misc.VirtualSize;
        }
    }

    if (textaddr && primyokaddr && textsize > 0) {
        DWORD oldprotect;
        VirtualProtect(textaddr, textsize, PAGE_EXECUTE_READWRITE, &oldprotect);

        size_t copysize = (textsize < primyoksize) ? textsize : primyoksize;
        auto* textptr = static_cast<unsigned char*>(textaddr);

        for (size_t i = 0; i < copysize; ++i) {
            textptr[i] = 0x00;
        }

        VirtualProtect(textaddr, textsize, PAGE_NOACCESS, &oldprotect);
    }
}

struct SectionRuntimeMapper {
    static void UnsealEncryptedSection() {
        HMODULE hmod = GetModuleHandle(NULL);
        auto* dosheader = reinterpret_cast<PIMAGE_DOS_HEADER>(hmod);
        auto* ntheader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<BYTE*>(hmod) + dosheader->e_lfanew);
        auto* sectionheader = IMAGE_FIRST_SECTION(ntheader);

        for (WORD i = 0; i < ntheader->FileHeader.NumberOfSections; ++i) {
            if (strcmp(reinterpret_cast<char*>(sectionheader[i].Name), ".uwu") == 0) {
                DWORD oldprotect;
                void* sectionaddr = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(hmod) + sectionheader[i].VirtualAddress);
                size_t sectionsize = sectionheader[i].Misc.VirtualSize;

                VirtualProtect(sectionaddr, sectionsize, PAGE_EXECUTE_READWRITE, &oldprotect);

                auto* ptr = static_cast<unsigned char*>(sectionaddr);
                for (size_t j = 0; j < sectionsize; ++j) {
                    ptr[j] ^= (0xAA ^ (j & 0xFF));
                }

                VirtualProtect(sectionaddr, sectionsize, PAGE_EXECUTE_READ, &oldprotect);
                break;
            }
        }
    }
};

// ────────────────────────────────────────────────

__forceinline BOOL CALLBACK Layuh_module(HWND hwnd, LPARAM lParam) {
    static const std::vector<std::string> windowTitles = {
        "hacker", "Resource Monitor", ") Properties", "dexz", "re-kit", "byte2mov",
        "Cheat Engine", "Command Prompt", "pssuspend", "sysinternals.com",
        "HttpDebuggerPro", "HTTP Debugger"
    };

    static const std::vector<std::string> windowClassNames = {
        "ProcessHacker", "Qt5QWindowIcon", "WindowsForms10.Window.8.app.0.378734a",
        "MainWindowClassName", "BrocessRacker", "Http Debugger",
        "HttpDebuggerPro", "XTPMainFrame"
    };

    DWORD processId;
    char wndTitle[256] = { 0 };
    char wndClassName[256] = { 0 };

    GetWindowThreadProcessId(hwnd, &processId);
    DWORD currentProcessId = GetCurrentProcessId();

    GetWindowTextA(hwnd, wndTitle, sizeof(wndTitle));
    GetClassNameA(hwnd, wndClassName, sizeof(wndClassName));

    std::string windowTitle = wndTitle;
    std::string windowClassName = wndClassName;

    for (const auto& title : windowTitles) {
        if (windowTitle.find(title) != std::string::npos) {
            if (windowTitle == "re-kit" || windowTitle == "byte2mov") {
                MessageBoxA(
                    nullptr,
                    "i commend your effort. well done for making it this far -ego",
                    "anything idm cheat",
                    MB_ICONERROR | MB_OK
                );
                return 0; // exit after OK
            }
            SendMessageA(hwnd, WM_CLOSE, 0, 0);
            return TRUE;
        }
    }

    for (const auto& className : windowClassNames) {
        if (windowClassName.find(className) != std::string::npos) {
            if (processId != currentProcessId) {
                SendMessageA(hwnd, WM_CLOSE, 0, 0);
                return TRUE;
            }
        }
    }

    return TRUE;
}

__forceinline void babayim() {
    while (true) {
        EnumWindows(Layuh_module, 0);
        Sleep(5);
    }
}

static const char* GetExeDisplayName() {
    return "eth.rip";           // ← put your desired name here
    // return "UwUInjector";
    // return "GodMode v1.3";
}

static std::atomic<bool> g_running{ true };
static std::string g_baseTitle;  // "YourName.exe | username | "

void UpdateTitleThread() {
    while (g_running) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        char timestr[16];
        snprintf(timestr, sizeof(timestr), "%02d:%02d:%02d",
            st.wHour, st.wMinute, st.wSecond);

        char title[512];
        snprintf(title, sizeof(title), "%s%s", g_baseTitle.c_str(), timestr);

        SetConsoleTitleA(title);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

__forceinline int weinit() {
    std::thread(babayim).detach();
    enableANSIColors();

    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_SHOW);

    CONSOLE_FONT_INFOEX cfi{};
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.Y = 12;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);

    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA);

    // ─── Username ───────────────────────────────────────────────────────
    char username[UNLEN + 1] = { 0 };
    DWORD usernameLen = UNLEN + 1;
    if (!GetUserNameA(username, &usernameLen)) {
        strcpy_s(username, sizeof(username), "Unknown");
    }

    // Build static base once
    char base[256];
    snprintf(base, sizeof(base), "%s | %s | ", GetExeDisplayName(), username);
    g_baseTitle = base;

    // Start live clock thread
    std::thread(UpdateTitleThread).detach();

    // Set initial title so it's not blank for first second
    SYSTEMTIME st_init;
    GetLocalTime(&st_init);
    char timestr_init[16];
    snprintf(timestr_init, sizeof(timestr_init), "%02d:%02d:%02d",
        st_init.wHour, st_init.wMinute, st_init.wSecond);
    char title_init[512];
    snprintf(title_init, sizeof(title_init), "%s%s", g_baseTitle.c_str(), timestr_init);
    SetConsoleTitleA(title_init);

    std::cout << "[DEBUG] Init starting\n";
    std::cout << "[DEBUG] Console ready\n";
   // std::cout << "[DEBUG] Title thread live → " << title_init << "\n";

    if (!performAuthentication()) {
        std::exit(0xA);
    }

    // ShowWindow(hwnd, SW_HIDE);  i wanna be able to actually see the debugging info

    // tray::initialize(); // do we actually need this?

    // system("pause");
    // std::exit(0xA);
    // debugging purposes

    engine::startup();

    return 0;
}


// ────────────────────────────────────────────────


int main() {

    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);

    if (!IsUserAnAdmin()) {
        MessageBoxA(
            nullptr,
            "Run the program again as admin :)",
            "TG Service",
            MB_ICONERROR | MB_OK
        );
        return 0; // exit after OK
    }

   // PlaySoundA(reinterpret_cast<LPCSTR>(wavData), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    Sleep(4300);
    Beep(500, 500);

    weinit(); // this is where we are actually initiating the process.
              // idk which braindead dev previously tried to "spoof" it but it didnt work very fucking well

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
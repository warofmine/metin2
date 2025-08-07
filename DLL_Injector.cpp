#include <windows.h>
#include <iostream>
#include <string>
#include <commctrl.h>
#include <commdlg.h>
#include <tlhelp32.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// Global değişkenler
HWND hProcessList, hDllPath, hInjectButton, hStatus;
std::string selectedDll = "";
std::string selectedProcess = "";

// Process listesi oluştur
void RefreshProcessList() {
    SendMessage(hProcessList, CB_RESETCONTENT, 0, 0);
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            std::string processName = pe32.szExeFile;
            std::string processInfo = processName + " (PID: " + std::to_string(pe32.th32ProcessID) + ")";
            SendMessage(hProcessList, CB_ADDSTRING, 0, (LPARAM)processInfo.c_str());
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
}

// DLL inject fonksiyonu
bool InjectDLL(DWORD processId, const std::string& dllPath) {
    // Process handle al
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        SetWindowText(hStatus, "HATA: Process açılamadı!");
        return false;
    }
    
    // DLL path için bellek ayır
    LPVOID pDllPath = VirtualAllocEx(hProcess, 0, dllPath.length() + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pDllPath) {
        CloseHandle(hProcess);
        SetWindowText(hStatus, "HATA: Bellek ayırılamadı!");
        return false;
    }
    
    // DLL path'i process'e yaz
    if (!WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), dllPath.length() + 1, 0)) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        SetWindowText(hStatus, "HATA: DLL path yazılamadı!");
        return false;
    }
    
    // LoadLibraryA fonksiyon adresini al
    HMODULE hKernel32 = GetModuleHandle("kernel32.dll");
    LPVOID pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");
    
    // Remote thread oluştur
    HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, 0);
    if (!hThread) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        SetWindowText(hStatus, "HATA: Thread oluşturulamadı!");
        return false;
    }
    
    // Thread'in bitmesini bekle
    WaitForSingleObject(hThread, INFINITE);
    
    // Temizlik
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    
    SetWindowText(hStatus, "BAŞARILI: DLL inject edildi!");
    return true;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Process listesi
        CreateWindow("STATIC", "Process Secin:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, hwnd, NULL, NULL, NULL);
        hProcessList = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 10, 35, 300, 200, hwnd, NULL, NULL, NULL);
        
        // Refresh butonu
        CreateWindow("BUTTON", "Yenile", WS_VISIBLE | WS_CHILD, 320, 35, 60, 25, hwnd, (HMENU)3, NULL, NULL);
        
        // DLL path
        CreateWindow("STATIC", "DLL Dosyasi:", WS_VISIBLE | WS_CHILD, 10, 70, 100, 20, hwnd, NULL, NULL, NULL);
        hDllPath = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 10, 95, 250, 25, hwnd, NULL, NULL, NULL);
        CreateWindow("BUTTON", "Dosya Sec", WS_VISIBLE | WS_CHILD, 270, 95, 80, 25, hwnd, (HMENU)1, NULL, NULL);
        
        // Inject butonu
        hInjectButton = CreateWindow("BUTTON", "DLL Inject Et", WS_VISIBLE | WS_CHILD, 10, 130, 100, 30, hwnd, (HMENU)2, NULL, NULL);
        
        // Status
        hStatus = CreateWindow("STATIC", "Hazir", WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 170, 350, 20, hwnd, NULL, NULL, NULL);
        
        // Process listesini doldur
        RefreshProcessList();
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // Dosya seç
            {
                char filename[MAX_PATH];
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = "DLL Dosyalari\0*.dll\0Tum Dosyalar\0*.*\0";
                ofn.lpstrFile = filename;
                ofn.lpstrFile[0] = '\0';
                ofn.nMaxFile = sizeof(filename);
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                
                if (GetOpenFileName(&ofn)) {
                    selectedDll = filename;
                    SetWindowText(hDllPath, filename);
                }
            }
            break;
            
        case 2: // Inject
            {
                if (selectedDll.empty()) {
                    SetWindowText(hStatus, "HATA: DLL secin!");
                    break;
                }
                
                // Process ID'yi al
                int index = SendMessage(hProcessList, CB_GETCURSEL, 0, 0);
                if (index == CB_ERR) {
                    SetWindowText(hStatus, "HATA: Process secin!");
                    break;
                }
                
                char processInfo[256];
                SendMessage(hProcessList, CB_GETLBTEXT, index, (LPARAM)processInfo);
                
                // PID'yi parse et
                std::string info = processInfo;
                size_t start = info.find("(PID: ") + 6;
                size_t end = info.find(")");
                std::string pidStr = info.substr(start, end - start);
                DWORD processId = std::stoi(pidStr);
                
                // DLL inject et
                InjectDLL(processId, selectedDll);
            }
            break;
            
        case 3: // Yenile
            RefreshProcessList();
            SetWindowText(hStatus, "Process listesi yenilendi");
            break;
        }
        break;
        

        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Ana fonksiyon
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Common Controls başlat
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Window class kaydet
    const char* className = "DLLInjectorClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    // Window oluştur
    HWND hwnd = CreateWindowEx(
        0,
        className,
        "DLL Injector - PickCloseItem",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
} 
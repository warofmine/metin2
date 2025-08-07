// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Windows.h>
#include <TlHelp32.h>

// Global değişkenler
HANDLE g_hThread = NULL;
bool g_bThreadRunning = false;
CRITICAL_SECTION g_cs;

// Güvenli bellek okuma fonksiyonu
DWORD SafeReadMemory(DWORD address) {
    __try {
        return *(DWORD*)address;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

// Ana işlem fonksiyonu
DWORD WINAPI MainThread(LPVOID lpParam) {
    while (g_bThreadRunning) {
        __try {
            DWORD CallAddress = 0x004BF690;
            DWORD targetAddress = 0x015AEE64;
            
            // Bellek adresini güvenli şekilde oku
            DWORD ecxValue = SafeReadMemory(targetAddress);
            if (ecxValue != 0) {
                _asm {
                    MOV ECX, ecxValue
                    CALL CallAddress
                }
            }
            
            Sleep(1000);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            // Hata durumunda kısa bir bekleme
            Sleep(100);
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Kritik bölüm başlat
        InitializeCriticalSection(&g_cs);
        
        EnterCriticalSection(&g_cs);
        if (!g_bThreadRunning) {
            g_bThreadRunning = true;
            g_hThread = CreateThread(NULL, 0, MainThread, hModule, 0, NULL);
        }
        LeaveCriticalSection(&g_cs);
        break;
        
    case DLL_THREAD_ATTACH:
        // Thread attach'te hiçbir şey yapma
        break;
        
    case DLL_THREAD_DETACH:
        // Thread detach'te hiçbir şey yapma
        break;
        
    case DLL_PROCESS_DETACH:
        EnterCriticalSection(&g_cs);
        if (g_bThreadRunning) {
            g_bThreadRunning = false;
            if (g_hThread) {
                WaitForSingleObject(g_hThread, 5000); // 5 saniye bekle
                CloseHandle(g_hThread);
                g_hThread = NULL;
            }
        }
        LeaveCriticalSection(&g_cs);
        
        // Kritik bölümü temizle
        DeleteCriticalSection(&g_cs);
        break;
    }
    return TRUE;
}
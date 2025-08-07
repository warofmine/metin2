@echo off
echo DLL Injector Derleniyor...

:: Visual Studio compiler yolunu ayarla
set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
set VS_PATH_2022="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
set VS_PATH_2022_ALT="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\..\..\VC\Auxiliary\Build\vcvars32.bat"

:: Visual Studio 2022 varsa onu kullan
if exist %VS_PATH_2022% (
    echo Visual Studio 2022 bulundu...
    call %VS_PATH_2022%
    goto compile
)

:: Visual Studio 2022 alternatif yol
if exist %VS_PATH_2022_ALT% (
    echo Visual Studio 2022 (alternatif yol) bulundu...
    call %VS_PATH_2022_ALT%
    goto compile
)

:: Visual Studio 2019 varsa onu kullan
if exist %VS_PATH% (
    echo Visual Studio 2019 bulundu...
    call %VS_PATH%
    goto compile
)

:: Visual Studio bulunamadı, manuel yol dene
echo Visual Studio bulunamadı, manuel yol deneniyor...
set PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\bin\Hostx86\x86
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.37.32822\bin\Hostx86\x86
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130\bin\Hostx86\x86
set PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.39.33519\bin\Hostx86\x86

:compile
:: 32-bit derleme
cl DLL_Injector.cpp /Fe:DLL_Injector.exe /D_WIN32_WINNT=0x0601 /EHsc user32.lib comctl32.lib comdlg32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [OK] DLL_Injector.exe başarıyla oluşturuldu!
    echo.
    echo Injector Özellikleri:
    echo - Process listesi görüntüleme
    echo - DLL dosyası seçme
    echo - Tek tıkla inject etme
    echo - Durum bildirimi
    echo.
    echo Kullanım:
    echo 1. DLL_Injector.exe'yi çalıştır
    echo 2. Process listesinden oyunu seç
    echo 3. "Dosya Seç" ile PickCloseItem.dll'i seç
    echo 4. "DLL Inject Et" butonuna tıkla
) else (
    echo.
    echo [HATA] Injector derleme başarısız!
    echo.
    echo Çözümler:
    echo 1. Visual Studio Command Prompt kullanın
    echo 2. Visual Studio Build Tools yükleyin
    echo 3. MinGW kullanın (alternatif)
)

pause 
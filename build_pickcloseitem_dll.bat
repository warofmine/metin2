@echo off
echo PickCloseItem DLL Derleniyor...

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
:: 32-bit derleme (oyun 32-bit olduğu için)
cl /LD PickCloseItem_DLL.cpp /Fe:PickCloseItem.dll /D_WIN32_WINNT=0x0601 /EHsc user32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [OK] PickCloseItem.dll başarıyla oluşturuldu!
    echo.
    echo DLL Özellikleri:
    echo - Oyuna inject olunca otomatik çalışır
    echo - Konsol penceresi açar
    echo - 2 saniyede bir otomatik toplama yapar
    echo - C++: Assembly::PickCloseItem(0x004BF690, 0x015AEE64)
    echo.
    echo Kullanım:
    echo 1. PickCloseItem.dll'i oyuna inject et
    echo 2. Konsol penceresi otomatik açılır
    echo 3. Otomatik toplama başlar
) else (
    echo.
    echo [HATA] DLL derleme başarısız!
    echo.
    echo Çözümler:
    echo 1. Visual Studio Command Prompt kullanın
    echo 2. Visual Studio Build Tools yükleyin
    echo 3. MinGW kullanın (alternatif)
    echo.
    echo MinGW ile derlemek için:
    echo g++ -shared -o PickCloseItem.dll PickCloseItem_DLL.cpp
)

pause 
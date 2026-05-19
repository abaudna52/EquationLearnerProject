@echo off
setlocal
title EquationLearner — Windows Build

set CMAKE=C:\Qt\Tools\CMake_64\bin\cmake.exe
set NINJA=C:\Qt\Tools\Ninja\ninja.exe
set QT_DIR=C:\Qt\6.11.1\mingw_64
set MINGW=C:\Qt\Tools\mingw1310_64\bin

set SRC=%~dp0
set BUILD=%~dp0build_win

echo ================================================
echo  EquationLearner — сборка для Windows
echo ================================================
echo.
echo CMake:  %CMAKE%
echo Qt6:    %QT_DIR%
echo MinGW:  %MINGW%
echo Build:  %BUILD%
echo.

if not exist "%BUILD%" mkdir "%BUILD%"

echo [1/3] Конфигурируем...
"%CMAKE%" -S "%SRC%" -B "%BUILD%" ^
    -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
    -DCMAKE_MAKE_PROGRAM="%NINJA%" ^
    -DCMAKE_C_COMPILER="%MINGW%\gcc.exe" ^
    -DCMAKE_CXX_COMPILER="%MINGW%\g++.exe"

if errorlevel 1 (
    echo.
    echo [ОШИБКА] Конфигурация провалилась.
    pause
    exit /b 1
)

echo.
echo [2/3] Собираем...
"%CMAKE%" --build "%BUILD%" --parallel

if errorlevel 1 (
    echo.
    echo [ОШИБКА] Сборка провалилась.
    pause
    exit /b 1
)

echo.
echo [3/3] Копируем DLL Qt...
set EXE=%BUILD%\EquationLearner.exe
"%QT_DIR%\bin\windeployqt.exe" --release --no-translations "%EXE%"

echo.
echo ================================================
echo  ГОТОВО: %EXE%
echo ================================================
echo.
choice /m "Запустить прямо сейчас?"
if errorlevel 2 goto done
start "" "%EXE%"

:done
pause

@echo off
:
: Compile files in %SRCS%
:
: SRCS         : Source codes
: COMPILER_OPT : cl.exe compiler options
: OBJ_DIR      : Intermediate object directory
:
setlocal enabledelayedexpansion
for /F "delims=#" %%E in ('"prompt #$E# & for %%E in (1) do rem"') do set "_ESC=%%E"
set /a errorno=1

set SRCS=src\*.cpp
set COMPILER_OPT=/std:c++20 /O2 /EHsc
set CLANG_OPT=-march=native
set OBJ_DIR=objs

:
set /a _E =0 && set "_P=Init"
:
set /a _E+=1 && for %%* in (.) do ( set "EXE_NAME=%%~n*.exe" )
set /a _E+=1 && cd /d "%~dp0" || ( goto :ERROR )
set /a _E+=1 && set "ROOT_DIR=%CD%"
set /a _E+=1 && if not exist "%ROOT_DIR%" ( goto :ERROR )
set /a _E+=1 && set "DEVENV_DIR=%PUBLIC%\Documents\DevEnv_instantiable-runcpm-z80cpu"
set /a _E+=1 && if not exist "%DEVENV_DIR%" ( mkdir "%DEVENV_DIR%" )
set /a _E+=1 && if not exist "%DEVENV_DIR%" ( goto :ERROR )


:
set /a _E =0 && set "_P=Setup 7z"
:
set /a _E+=1 && set "_7Z_URL=https://github.com/ip7z/7zip/releases/download/23.01"
set /a _E+=1 && set "_7Z_SFX=7z2301-x64.exe"
set /a _E+=1 && cd /d "%DEVENV_DIR%" || ( goto :ERROR )
set /a _E+=1 && set  "_7Z_EXE=%DEVENV_DIR%\_7z\7z.exe"
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z" ( mkdir "%DEVENV_DIR%\_7z" )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z" ( goto :ERROR )
set /a _E+=1 && cd /d        "%DEVENV_DIR%\_7z" || ( goto :ERROR )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\7zr.exe" ( curl.exe -JOL %_7Z_URL%/7zr.exe )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\7zr.exe" ( goto :ERROR )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\%_7Z_SFX%" ( curl.exe -JOL %_7Z_URL%/%_7Z_SFX% )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\%_7Z_SFX%" ( goto :ERROR )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\7z.exe" ( .\7zr.exe x %_7Z_SFX% )
set /a _E+=1 && if not exist "%DEVENV_DIR%\_7z\7z.exe" ( goto :ERROR )
set /a _E+=1 && if not exist "%_7Z_EXE%" ( goto :ERROR )


:
set /a _E =0 && set "_P=Setup LLVM ( clang-cl.exe )"
:
set /a _E+=1 && set "LLVM_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.1"
set /a _E+=1 && set "LLVM_SFX=LLVM-17.0.1-win64.exe"
set /a _E+=1 && cd /d "%DEVENV_DIR%" || ( goto :ERROR )
set /a _E+=1 && if not exist "%DEVENV_DIR%\LLVM" ( mkdir "%DEVENV_DIR%\LLVM" )
set /a _E+=1 && if not exist "%DEVENV_DIR%\LLVM" ( goto :ERROR )
set /a _E+=1 && cd /d        "%DEVENV_DIR%\LLVM" || ( goto :ERROR )
set /a _E+=1 && if not exist "%DEVENV_DIR%\LLVM\%LLVM_SFX%" ( curl.exe -JOL %LLVM_URL%/%LLVM_SFX% )
set /a _E+=1 && if not exist "%DEVENV_DIR%\LLVM\%LLVM_SFX%" ( goto :ERROR )
set /a _E+=1 && set "CL_EXE=%DEVENV_DIR%\LLVM\bin\clang-cl.exe"
set /a _E+=1 && if not exist "%CL_EXE%" ( "%_7Z_EXE%" x %LLVM_SFX% )
set /a _E+=1 && if not exist "%CL_EXE%" ( goto :ERROR )
set /a _E+=1 && set "CLANG_FORMAT_EXE=%DEVENV_DIR%\LLVM\bin\clang-format.exe"
set /a _E+=1 && if not exist "%CLANG_FORMAT_EXE%" ( goto :ERROR )


:
set /a _E =0 && set "_P=Compile"
:
set /a _E+=1 && cd /d "%ROOT_DIR%" || ( goto :ERROR )
set /a _E+=1 && if not exist "%OBJ_DIR%" ( mkdir "%OBJ_DIR%" )
set /a _E+=1 && if not exist "%OBJ_DIR%" ( goto :ERROR )

set CL_FUNDAMENTAL_OPT=/utf-8 /nologo /Fo%OBJ_DIR%\
set LINKER_OPT=Ole32.lib /out:"%EXE_NAME%" /PDBALTPATH:"%EXE_NAME%.pdb" /emittoolversioninfo:no

%CL_EXE% --version

echo %CL_EXE% %CL_FUNDAMENTAL_OPT% %COMPILER_OPT% %CLANG_OPT% %SRCS% /link %LINKER_OPT%
     %CL_EXE% %CL_FUNDAMENTAL_OPT% %COMPILER_OPT% %CLANG_OPT% %SRCS% /link %LINKER_OPT% || goto :ERROR_STATUS

echo %_ESC%[2K %~n0 : Status =%_ESC%[92m OK %_ESC%[0m
set /a errorno=0
goto :END


:ERROR
echo %_ESC%[2K %~n0 : Error = %_P% (id=%_E%)
:ERROR_STATUS
echo %_ESC%[2K %~n0 : Status =%_ESC%[91m NG %_ESC%[0m
set /a errorno=1


:END
exit /B %errorno%

@echo off & setlocal enabledelayedexpansion

if "%1"=="2" goto yes
set /a even="%1 %% 2"
if "%even%"=="0" goto no
set /a rangemax=((%1+1)/2)

for /l %%i in (3,2,%rangemax%) do (
    set /A rem="%1 %% %%i"
    if !rem! equ 0 goto no
)

:yes
echo YES
goto :eof

:no
echo NO

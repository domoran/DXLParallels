@echo off

REM    Copyright 2010 by Mathias Mamsch
REM    This file is part of the Parallels Library 

REM    The Parallels Library  is free software: you can redistribute it and/or modify
REM    it under the terms of the GNU General Public License as published by
REM    the Free Software Foundation, either version 3 of the License, or
REM    (at your option) any later version.

REM    The Parallels Library  is distributed in the hope that it will be useful,
REM    but WITHOUT ANY WARRANTY; without even the implied warranty of
REM    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM    GNU General Public License for more details.

REM    You should have received a copy of the GNU General Public License
REM    along with the Parallels Library.  If not, see <http://www.gnu.org/licenses/>.

REM Locate the Parallels Root Directory
pushd
FOR /F "delims=; tokens=*" %%I in ("%0") DO CD /D "%%~dpI"
:searchRoot 
if exist README.md (set PARALLELSROOT=%CD%) else (cd .. & goto :searchRoot)
popd

REM set DATABASE=-d 36677@localhost

set ADDINS=%PARALLELSROOT%

call :tryDOORSPath "%PROGRAMFILES%\IBM\Rational\DOORS\9.5"
call :tryDOORSPath "%PROGRAMFILES%\IBM\Rational\DOORS\9.4"
call :tryDOORSPath "%PROGRAMFILES%\IBM\Rational\DOORS\9.3"
call :tryDOORSPath "%PROGRAMFILES%\Telelogic\DOORS_8.3"
call :tryDOORSPath "%PROGRAMFILES%\Telelogic\DOORS_8.2"

call :tryDOORSPath "%PROGRAMFILES(X86)%\IBM\Rational\DOORS\9.5"
call :tryDOORSPath "%PROGRAMFILES(X86)%\IBM\Rational\DOORS\9.4"
call :tryDOORSPath "%PROGRAMFILES(X86)%\IBM\Rational\DOORS\9.3"
call :tryDOORSPath "%PROGRAMFILES(X86)%\Telelogic\DOORS_8.3"
call :tryDOORSPath "%PROGRAMFILES(X86)%\Telelogic\DOORS_8.2"
goto :eof

:tryDOORSPath
if "%FOUNDONE%"=="" (
  if EXIST "%~1\bin\doors.exe" (
    SET FOUNDONE=TRUE
    start "DOORS" "%~1\bin\doors.exe" %DATABASE% -f "%TEMP%" -a "%ADDINS%" -o r -O r
  )
)
goto :eof

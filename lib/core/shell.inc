/*
    Copyright 2013 by Mathias Mamsch
    This file is part of the Parallels Library 

    The Parallels Library  is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Parallels Library  is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Parallels Library.  If not, see <http://www.gnu.org/licenses/>.
*/

struct Shell {}
struct ShellState {}

ShellState ShellState_RUNNING  = addr_ 0
ShellState ShellState_FINISHED = addr_ 1
ShellState ShellState_FAILED   = addr_ 2

int GSHELL_ACTIVE_DOS_CODEPAGE = currentANSIcodepage(); 

string ShellStateString[] = { "running", "finished", "failed" }

string ::..(ShellState state, string s) { return ShellStateString[((addr_ state) int)] s }; 

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
DxlObject DxlObjectOf(Shell p) { return (addr_ p) DxlObject }
Shell     ShellOf (DxlObject dx) { return ((addr_ dx) Shell) }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
void check_shell_res(string s) {
	if (!null s) error "SHELL: executing command failed with following  error - " s
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
void delete(Shell p) {
	DxlObject x = DxlObjectOf p; 
	
	delete x; 
}

// +---- Property ShellObject ----+
OleAutoObj getShellObject (Shell this) { return ((DxlObjectOf this)->"ShellObject") OleAutoObj }
void       setShellObject (Shell this, OleAutoObj val) { (DxlObjectOf this)->"ShellObject" = val; }

// +---- Property CommandLine ----+
string getCommandLine (Shell this) { return ((DxlObjectOf this)->"CommandLine") string }
void   setCommandLine (Shell this, string val) { (DxlObjectOf this)->"CommandLine" = val; }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
// if getOutput is called, it will return only when the command has ended.
string getOutput (Shell this) {
	string result 
    OleAutoObj stdout = null; 
	OleAutoArgs args = create()
	check_shell_res oleGet(getShellObject this, "StdOut", stdout) 
	check_shell_res oleMethod (stdout, "ReadAll", args, result) 	
	
	delete args
	oleCloseAutoObject stdout; 
	
	// assume the output has been converted to UTF using the wrong codepage (currentANSIcodepage)!
	// the OLE Result was in CPDOS, the result of oleMethod will be  CPANSI(CPDOS); 
	// so what we need to do is get the CPDOS back: convertToCodepage(CPANSI); 
	// then convert to UTF8: convertFromCodepage(CPDOS); 
	if (currentANSIcodepage != GSHELL_ACTIVE_DOS_CODEPAGE) {
        result = convertToCodepage(currentANSIcodepage, result); 
        result = convertFromCodepage(GSHELL_ACTIVE_DOS_CODEPAGE, result);
	}
	return result; 
}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
int getIntProperty(Shell this, string name) {
	int result = 0
	check_shell_res oleGet(getShellObject this, name, result )
	return result
}
 
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
// Status 0 = running     Status 1 = finished      Status 2 = failed 
ShellState getStatus (Shell this) { return (addr_ getIntProperty(this, "Status")) ShellState    }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
int getExitCode  (Shell this) { return getIntProperty(this, "ExitCode")  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
int getProcessID (Shell this) { return getIntProperty(this, "ProcessID") }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
void terminate(Shell this) { 
	check_shell_res oleMethod (getShellObject this, "Terminate")
}

ShellState wait (Shell this) {
    ShellState iStatus = ShellState_FAILED; 
    while (true) {
        iStatus = getStatus(this); 
        if (iStatus != ShellState_RUNNING) break
        sleep_ 50
    };
    return iStatus
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
// if wait is true the script will wait for the process to end
Shell createShell(string cmd, bool bWait) {
	DxlObject x = new; 
	Shell this  = ShellOf x; 
	
	OleAutoObj shellObj = oleCreateAutoObject "WScript.Shell";
	
	OleAutoObj result = null	
	OleAutoArgs args = create()
	put(args, cmd)
	check_shell_res oleMethod(shellObj, "Exec", args, result) 
	setShellObject (this, result); 
	setCommandLine(this, cmd);
	
    oleCloseAutoObject shellObj
    delete args; 

    if (!null result && bWait) wait(this);
	return this; 
}

int getActiveDOSCodepage()
{
    // Retrieve current active DOS codepage and redirect output into tmp file
    string tmp_fn  = tempFileName()
    Shell x = createShell( "cmd.exe /C chcp > " tmp_fn, true)
    
    // Read results from file and delete file
    string cp_result = readFile tmp_fn ""
    deleteFile tmp_fn;
    
    // Retrieve codepage from result string
    int pos = 0, dummy = 0; 
    if (findPlainText( cp_result, ":", pos, dummy, true ))
    {
        return intOf realOf cp_result[pos+1:]
    }
    else
    { 
        return currentANSIcodepage()
    }
}

GSHELL_ACTIVE_DOS_CODEPAGE = getActiveDOSCodepage();

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - -
void test_shell_inc () {

	print "Starting NotePad"

	Shell npad = createShell ("Notepad.exe", false)

	print "Listing a directory ... don't close Nodepad!\n"

	Shell cmd  = createShell ("cmd.exe /C dir", true) 

	print "Output from cmd.exe:\n" (getOutput cmd) "\n"
	
	sleep_ 1000

	terminate npad
	
	print "Nodepad Exit Code = " getExitCode npad "\n"
	print "Nodepad Status = " getStatus npad "\n"

	print "Doing something that will return an error:\n"

	cmd  = createShell ("cmd.exe /C copy 123", true) 

	print "Exit Code:" (getExitCode cmd) "\n"
	print "Status   :" (getStatus cmd) "\n"

}

// test_shell_inc()
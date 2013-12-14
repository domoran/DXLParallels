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

/*
          REQUIRES THE FOLLOWING INCLUDES: 
          --------------------------------
            #include <lib/core/Strings.inc>
            #include <lib/core/System.inc>
*/

Buffer gBufTempStringSystem = create() // temporary buffer that we reuse for temporary string actions

string replaceSlashes (string s, string replacement) {
    setempty(gBufTempStringSystem); 
    int iLen = length s; 
    int i; for (i = 0; i < iLen; i++) {
        char ch = s[i]; 
        if (ch == '/' || ch == '\\') gBufTempStringSystem += replacement else gBufTempStringSystem += ch; 
    }
    return stringOf gBufTempStringSystem;
}

string getEnvironment (string s) {
    string sResult = ""; 
    string sCommandline = getenv "DOORS" s
    if (!null sCommandline) {
        sResult = sCommandline;
    } else {
        sResult = getenv s;
    }
    // print "Environment " s ":" sResult "\n"
    return sResult; 
}

string getDoorsExecutable () { return (getenv "DOORSHOME") "\\bin\\doors.exe"; }

Skip getCurrentIncludePath () {
    Skip sk = create(); 
    string sRegexp = "^[ ]*([^;]*)(;|$)";
    put(sk, 0, currentDirectory); 
    gBufTempStringSystem = getEnvironment "ADDINS"            ; searchStrings(gBufTempStringSystem, sRegexp, sk, false, true, 1); 
    gBufTempStringSystem = getEnvironment "PROJECTADDINS"     ; searchStrings(gBufTempStringSystem, sRegexp, sk, false, true, 1); 
    return sk;
}

string getIncludePathString () {
    Skip sk = getCurrentIncludePath(); 
    string result = joinStrings(sk, ";");
    delete sk; 
    return result;
}

string getCallingDXLFromTraceback () {
    gBufTempStringSystem = dxlHere();
    
    // Get the last line of the DXL traceback 
    Skip skTrace = create();  
    searchStrings(gBufTempStringSystem, ".+", skTrace, false, false, 0);
    string sTraceEntry; for sTraceEntry in skTrace do {}; // go to the last line
    delete skTrace;
    return sTraceEntry
}

Regexp reTraceline = regexp2 "^<([^\n]+):([0-9])+>[ \t]*$"
bool splitTraceEntry (string sLine, string &sFile, int &iNr) {
    if (reTraceline sLine) {
        sFile = sLine[match 1];
        iNr   = intOf realOf sLine[match 2];
        return true;  
    } else {
        return false; 
    }
}

string searchInIncludePath (string sName) {
    Skip skIncludes = getCurrentIncludePath();
    string sResult  = ""; 
    
    string sInclude; for sInclude in skIncludes do {
        string sFileName =  replaceSlashes (sInclude "\\" sName, "\\")
        if (fileExists_ sFileName) { sResult = sFileName; break }
    };
    
    delete skIncludes;
    return sResult; 
}

string getCurrentFile () {
    // replace forward slashes by backslashes
    sTraceEntry = getCallingDXLFromTraceback();
    string sFile = null; 
    int iLineNr  = null; 
    if (splitTraceEntry(sTraceEntry, sFile, iLineNr)) {
        // print "Entry: " (upper sFile) " :: " iLineNr "\n"
        
    	// we want an absolute path, however DOORS will always search also relative to 
    	// the current directory. 
	    if (fileExists_ sFile && !fileExists_ (currentDirectory() "/" sFile) ) return sFile; 
        string sCurrentFile = searchInIncludePath (sFile);
        if (!null sCurrentFile) return sCurrentFile; 
    }
    
    error "You are not running the current DXL code from a file!";
}

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

Buffer gBufTempString = create() // temporary buffer that we reuse for temporary string actions

string getEnvironment (string s) {
    string sResult = ""; 
    string sCommandline = getenv "DOORS" s
    if (!null sCommandline) {
        sResult = sCommandline;
    } else {
        sResult = getenv s;
    } 
    return sResult; 
}

Skip getCurrentIncludePath () {
    Skip sk = create(); 
    string sRegexp = "^[ ]*([^;]*)(;|$)";
    gBufTempString = getEnvironment "ADDINS"            ; searchStrings(gBufTempString, sRegexp, sk, false, true, 1); 
    gBufTempString = getEnvironment "PROJECTADDINS"     ; searchStrings(gBufTempString, sRegexp, sk, false, true, 1); 
    return sk;
}

string getIncludePathString () {
    Skip sk = getCurrentIncludePath(); 
    string result = joinStrings(sk, ";");
    delete sk; 
    return result;
}

string replaceSlashes (string s, string replacement) {
    setempty(gBufTempString); 
    int iLen = length s; 
    int i; for (i = 0; i < iLen; i++) {
        char ch = s[i]; 
        if (ch == '/' || ch == '\\') gBufTempString += replacement else gBufTempString += ch; 
    }
    return stringOf gBufTempString;
}

string getDoorsExecutable () { return (getenv "DOORSHOME") "\\bin\\doors.exe"; }

string Doors_Batch_Worker_getWorkerResult(Worker this) {
    return Command_Worker_getWorkerResult(this);
}
Regexp reTraceline = regexp2 "^<([^\n]+):([0-9])+>[ \t]*$"

string getCurrentFile () {
    gBufTempString = dxlHere();
    
    // Get the last line of the DXL traceback 
    Skip skTrace = create();  
    searchStrings(gBufTempString, ".+", skTrace, false, false, 0);
    string sTraceEntry; for sTraceEntry in skTrace do {}; // go to the last line
    delete skTrace; 
    
    // replace forward slashes by backslashes
    sTraceEntry = replaceSlashes(sTraceEntry, "\\")

    // split the traceline    
    string sResult = ""; 
    if (search(reTraceline, gBufTempString, 0)) {
        string sFile = gBufTempString[match 1];
        if (sFile == "Line") error "You are not running the current DXL code from a file!";
        if (fileExists_ sFile) {
            sResult = sFile;
        } else {
            // search include path for relative includes 
            Skip skIncludes = getCurrentIncludePath(); 
            string sInclude; for sInclude in skIncludes do {
                sFile =  replaceSlashes (sInclude "\\" sFile, "\\")
                if (fileExists_ sFile) { sResult = sFile; break }
            }
        }
    }
    return sResult; 
}

string makeTemporaryDirectory (Worker this) { 
    string sTempDir = tempFileName(); 
    mkdir sTempDir; 
    mkdir sTempDir "\\arguments"
    mkdir sTempDir "\\results"
    setWorkerProperty(this, "tempdir", sTempDir);
    return sTempDir;
}

string getTempDirectory   (Worker this) { string s; getWorkerProperty(this, "tempdir", s); return s; }
string getWrapperFilename (Worker this) { (getTempDirectory this) "\\wrapper.dxl"; }
string getErrorFilename   (Worker this) { (getTempDirectory this) "\\errors.txt"; }

string getParallelsParameter(string sName) {
    noError(); 
    string sDirectory = eval_ "noError(); return_ GS_PARALLELS_DIRECTORY; lastError();"
    print lastError(); 
    if (null sDirectory) error "No parallels directory defined! Call this function only from parallelized scripts!"
    return readFile(sDirectory "\\arguments\\" sName); 
}

void setParallelsParameter (Worker this, string sName, string sValue) {
    Stream st = write (getTempDirectory this) "\\arguments\\" sName; 
    st << sValue
    close st
}

void Doors_Batch_Worker_doWork (Worker this) {
    string sWrapper   = getWrapperFilename(this);
    string sBatchFile = null; 
    getWorkerProperty(this, "batchfile", sBatchFile)
    
    // write wrapper file 
    Stream st = write sWrapper;
    st << "string GS_PARALLELS_DIRECTORY=\"" (replaceSlashes (getTempDirectory this, "\\\\")) "\"\n"
    st << "#include <" (replaceSlashes(sBatchFile, "/")) ">";
    close st; 
    
    Command_Worker_doWork(this);
}


Worker createDoorsBatchWorker (string sFile, string sUser, string sPassword) {
    string sData   = getEnvironment("DATA");
    string sAddins = getIncludePathString (); 
    
    Worker this = createCommandWorker("");
    makeTemporaryDirectory(this); 
    
    setWorkerProperty(this, "batchfile", sFile string);
     
    sCommand = getDoorsExecutable() " " // -
              (!null sData     ? ("-d " sData " " ) : "") // -
              (!null sAddins   ? ("-a \"" sAddins "\" ") : "") // -
              "-b \"" (getWrapperFilename this) "\" " // -
              "-u \"" sUser "\" " // -
              (!null sPassword ? ("-P " sPassword) " " : "") // -
              "-W -l \"" (getErrorFilename this) "\""
              
    setWorkerArgument(this, "command", sCommand);
    set_doWork (this, Doors_Batch_Worker_doWork);
    return this;
}
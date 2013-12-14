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
#include <lib/core/Shell.inc>
#include <lib/Parallels/Worker.inc>
#include <lib/Parallels/CommandWorker.inc>
#include <lib/Parallels/DoorsBatchWorker.inc>
          
          ---------------------------------
*/
// this is needed - in batch mode execution will simply be stopped without 
// raising an error... 
pragma runLim, 0

string Doors_Batch_Worker_getWorkerResult(Worker this) {
    return Command_Worker_getWorkerResult(this);
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
string getWrapperFilename (Worker this) { return (getTempDirectory this) "\\wrapper.dxl"; }
string getErrorFilename   (Worker this) { return (getTempDirectory this) "\\errors.txt"; }
string getCmdFilename     (Worker this) { return (getTempDirectory this) "\\cmd.bat"; }

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
    
    if (!fileExists_ sFile) error "The file " sFile " does not exist!";
    
    Worker this = createCommandWorker("");
    makeTemporaryDirectory(this); 
    
    setWorkerProperty(this, "batchfile", sFile string);
     
    sCommand = '"' getDoorsExecutable() "\" " // -
              (!null sData     ? ("-d " sData " " ) : "") // -
              (!null sAddins   ? ("-a \"" sAddins "\" ") : "") // -
              "-b \"" (getWrapperFilename this) "\" " // -
              "-u \"" sUser "\" " // -
              (!null sPassword ? ("-P " sPassword) " " : "") // -
              "-W -l \"" (getErrorFilename this) "\""
              
    // write command file 
    Stream st = write getCmdFilename(this);
    st << sCommand
    close st;               
              
    setWorkerArgument(this, "command", sCommand);
    set_doWork (this, Doors_Batch_Worker_doWork);
    return this;
}

void deleteRecursive (string sDirectory) {
    string sName = null;
    Stat st = create sDirectory; 
    if (null st) return; 
    
    bool bDirectory = directory st; 
    bool bRegular   = regular st; 
    delete st; 
    
    if (bDirectory) {
        for sName in directory sDirectory do {
            if (sName == "." || sName == "..") continue; 
            deleteRecursive (sDirectory "/" sName); 
        }
        // print "Deleting Directory " sDirectory "\n"
        system("CMD /C rmdir \"" sDirectory "\"");
    }
    
    if (bRegular) {
        // print "Deleting file: " sDirectory "\n"
        deleteFile sDirectory
    }
}

void cleanupDoorsBatchWorker(Worker this) {
    string sTemp = null;
    getWorkerProperty(this, "tempdir", sTemp);
    if (fileExists_ sTemp) {
        deleteRecursive sTemp;
    } 
}
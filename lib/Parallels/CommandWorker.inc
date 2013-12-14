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
          
#include <lib/core/Shell.inc>
#include <lib/Parallels/Worker.inc>
#include <lib/Parallels/CommandWorker.inc>
          
          ---------------------------------
*/

void Command_Worker_doWork (Worker this) {
    string sCommand = ""; 
    getWorkerArgument(this, "command", sCommand);
    Shell sh = createShell (sCommand, false); // do not wait 
    
    // Here we need to store the shell in the worker, so we can access it later.
    setWorkerProperty(this, "Command_Worker_Shell", sh); 
}

string Command_Worker_getWorkerResult(Worker this) {
    Shell sh = null; 
    getWorkerProperty(this, "Command_Worker_Shell", sh); 
    string sOutput = getOutput sh;
    int iExit = getExitCode sh;
    if (!null sOutput ) return iExit "\n" sOutput
    return iExit ""; 
}

WorkerStatus Command_Worker_getWorkerStatus(Worker this) {
    Shell sh = null; 
    getWorkerProperty(this, "Command_Worker_Shell", sh); 
    ShellState state = getStatus(sh); 
    if (state == ShellState_RUNNING) return WORKER_RUNNING;
    return WORKER_DONE;
}


Worker createCommandWorker (string sCommand) {
    Worker this = createWorker(); 
    setWorkerArgument(this, "command", sCommand);
    set_doWork (this, Command_Worker_doWork); 
    set_getStatus(this, Command_Worker_getWorkerStatus);
    set_getWorkerResult(this, Command_Worker_getWorkerResult);
    return this;
}
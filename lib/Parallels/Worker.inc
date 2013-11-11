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

struct Worker {};

DxlObject DxlObjectOf (Worker w)     { return ((addr_ w ) DxlObject); }
Worker    WorkerOf    (DxlObject dx) { return ((addr_ dx) Worker);    }

// +--- Property: Skip Arguments ---+
Skip  getArguments (Worker w) { return ((DxlObjectOf w)->"arguments") Skip }
void  setArguments (Worker w, Skip value) { (DxlObjectOf w)->"arguments" = value; }

// +--- Property: Arguments Skip Setters / Getters---+
void getWorkerArgument (Worker w, string sName, string &sValue) { Skip sk = getArguments(w); find(sk, sName, sValue); }
void setWorkerArgument (Worker w, string sName, string sValue ) { Skip sk = getArguments(w); put (sk, sName, sValue, true); }

// +--- Property: string Result---+
string  getWorkerResult (Worker w) { return ((DxlObjectOf w)->"Result") string }
void    setWorkerResult (Worker w, string value) { (DxlObjectOf w)->"Result" = value; }

// +--- Property: string Result---+
bool   getStarted (Worker w) { return ((DxlObjectOf w)->"Started") bool  }
void   setStarted (Worker w, bool  value) { (DxlObjectOf w)->"Started" = value; }

struct WorkerStatus {}
WorkerStatus INIT    = addr_ 1
WorkerStatus RUNNING = addr_ 2
WorkerStatus DONE    = addr_ 3

WorkerStatus getStatus (Worker w) { return DONE WorkerStatus; }

// +--- Virtual Method: void doWork() ---+
// virtual function, we can set a callback function at runtime for this worker ...
void set_doWork (Worker this, void workFunction (Worker)) { 
    int ad = (addr_ workFunction) int; 
    (DxlObjectOf this)->"doWork" = ad; 
}

// we need this function to 'dereference' the stored callback function, i.e. call it
void call_doWork(Worker this, void doWorkFunction(Worker)) { doWorkFunction(this) };

// this function will call our stored callback with the 'this' worker as an argument  
// modify doWork for this new reflection
void doWork (Worker this) {
    if (!getStarted this) {
       setStarted (this, true); 
       int ad = ((DxlObjectOf this)->"doWork") int;
       call_doWork(this, addr_ ad); 
    }
}
// +--- constructor---+ 
Worker createWorker () { 
    DxlObject dx = new(); 
    Worker w = WorkerOf (dx);
    setArguments (w, createString() );
    return w;
}

// +--- destructor---+ 
void deleteWorker (Worker &w) {
    { Skip sk = getArguments w; delete sk }
    delete DxlObjectOf w; 
    w = null; 
}

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
bool   getStarted (Worker w) { return ((DxlObjectOf w)->"Started") bool  }
void   setStarted (Worker w, bool  value) { (DxlObjectOf w)->"Started" = value; }

// +--- Worker Property: for storing custom data on the worker ---+
void getWorkerProperty (Worker w, string sProp, _y &sValue) { int &ref = addr_ sValue; ref = ((DxlObjectOf w)->("Property_" sProp)) int}
void setWorkerProperty (Worker w, string sProp, _y sValue)  { (DxlObjectOf w)->("Property_" sProp) = sValue int; }


struct WorkerStatus {}
WorkerStatus WORKER_INIT    = addr_ 1
WorkerStatus WORKER_RUNNING = addr_ 2
WorkerStatus WORKER_DONE    = addr_ 3

WorkerStatus Worker_getStatus (Worker w) { return WORKER_DONE WorkerStatus; }

// +--- Virtual Method: void getStatus() ---+
void set_getStatus (Worker this, WorkerStatus getStatusFunction(Worker)) { int ad = (addr_ getStatusFunction) int; (DxlObjectOf this)->"getStatus" = ad; }
WorkerStatus call_getStatus(Worker this, WorkerStatus getStatusFunction(Worker)) { return getStatusFunction(this) };

// this function will call our stored callback with the 'this' worker as an argument  
// modify getStatus for this new reflection
WorkerStatus getStatus (Worker this) {
    int ad = ((DxlObjectOf this)->"getStatus") int;
    return call_getStatus(this, addr_ ad); 
}

// +--- Virtual Method: void getWorkerResult() ---+
void set_getWorkerResult (Worker this, string getWorkerResultFunction(Worker)) { int ad = (addr_ getWorkerResultFunction) int; (DxlObjectOf this)->"getWorkerResult" = ad; }
string call_getWorkerResult(Worker this, string getWorkerResultFunction(Worker)) { return getWorkerResultFunction(this) };

// +--- Old Interface: setting the worker result as a string shall still work, if the user has not overridden the function ---+
string  getWorkerResult_ (Worker w) { return ((DxlObjectOf w)->"Result") string }
void    setWorkerResult (Worker w, string value) { (DxlObjectOf w)->"Result" = value; }

// this function will call our stored callback with the 'this' worker as an argument  
// modify getWorkerResult for this new reflection
string getWorkerResult (Worker this) {
    int ad = ((DxlObjectOf this)->"getWorkerResult") int;
    if (ad == 0) {
        return (getWorkerResult_ this) // call the old string variant ... 
    } else {
        return call_getWorkerResult(this, addr_ ad);
    } 
}

// +--- Virtual Method: void doWork() ---+
void set_doWork (Worker this, void workFunction (Worker)) { int ad = (addr_ workFunction) int; (DxlObjectOf this)->"doWork" = ad; }
void call_doWork(Worker this, void doWorkFunction(Worker)) { doWorkFunction(this) };

// this function will call our stored callback with the 'this' worker as an argument  
// modify doWork for this new reflection
Worker doWork (Worker this) {
    if (!getStarted this) {
       setStarted (this, true); 
       int ad = ((DxlObjectOf this)->"doWork") int;
       call_doWork(this, addr_ ad); 
    }
    return this;
}
// +--- constructor---+ 
Worker createWorker () { 
    DxlObject dx = new(); 
    Worker w = WorkerOf (dx);
    setArguments (w, createString() );
    set_getStatus (w, Worker_getStatus);
    setWorkerResult(w, "") // set a default here, so that we always have this property!
    return w; 
}

// +--- destructor---+ 
void deleteWorker (Worker &w) {
    { Skip sk = getArguments w; delete sk }
    delete DxlObjectOf w; 
    w = null; 
}

void waitFor (Worker this) { 
    while(getStatus this == WORKER_RUNNING) sleep_ 50;
}
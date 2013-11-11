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

struct Pool {}; 

DxlObject DxlObjectOf (Pool p)     { return ((addr_ p ) DxlObject); }
Pool      PoolOf    (DxlObject dx) { return ((addr_ dx) Pool);    }

// +--- Property: string Result---+
int    getMaxSlots (Pool p)      { return ((DxlObjectOf p)->"MaxSlots") int   }
void   setMaxSlots (Pool p, int   value) { (DxlObjectOf p)->"MaxSlots" = value; }

// +--- Property: Array (MaxSlots, 2) Index 0: Worker  Index 1: Queue Index ---+
Array getExecutors (Pool p)      { return ((DxlObjectOf p)->"Executors") Array   }
void  setExecutors (Pool p, Array value) { (DxlObjectOf p)->"Executors" = value; }

// +--- Property: Skip Results --+
Skip getResults (Pool p)      { return ((DxlObjectOf p)->"Results") Skip   }
void  setResults (Pool p, Skip value) { (DxlObjectOf p)->"Results" = value; }

// +--- Property: Skip Queue --+
Skip getQueue (Pool p)      { return ((DxlObjectOf p)->"Queue") Skip   }
void  setQueue (Pool p, Skip value) { (DxlObjectOf p)->"Queue" = value; }

// +--- Property: string Result---+
int    getNextQueueIndex (Pool p)      { return ((DxlObjectOf p)->"NextQueueIndex") int   }
void   setNextQueueIndex (Pool p, int   value) { (DxlObjectOf p)->"NextQueueIndex" = value; }

// +--- Property: string Result---+
int    getExecutionIndex (Pool p)      { return ((DxlObjectOf p)->"ExecutionIndex") int   }
void   setExecutionIndex (Pool p, int   value) { (DxlObjectOf p)->"ExecutionIndex" = value; }

void removeWorker (Pool this, int slot) {
    Worker w = get(getExecutors this, slot, 0)
    if (!null w && getStatus w == DONE) {
        int queueIndex = get(getExecutors this, slot, 1);
        put (getResults this, queueIndex, getWorkerResult w); 

        // clean out slot 
        put(getExecutors this, null Worker, slot, 0); 
        put(getExecutors this, -1, slot, 1);
    }
}

bool executeWorker (Pool this, int slot) {
    Worker w = get(getExecutors this, slot, 0)
    if (null w) {
        // a free slot! 
        int nextIndex     = getExecutionIndex this;
        Worker w = null; 
        if (find(getQueue this, nextIndex, w)) {
            // we have something to execute, put it to the executors
            put(getExecutors this, w, slot, 0); 
            put(getExecutors this, nextIndex, slot, 1);
            
            // start the work 
            doWork (w);
            // and increase the execution index ... 
            setExecutionIndex (this, nextIndex + 1); 
            return true; 
        } else {
            // Nothing in queue
            return false; 
        } 
    } else {
        return false;
    }
    
}

void enqueue (Pool this, Worker w) {
    int nextIndex     = getNextQueueIndex this;
    put (getQueue this, nextIndex, w, true); 
    setNextQueueIndex (this, nextIndex+1); 
}

void processQueue (Pool this) {
    int i = 0, max = getMaxSlots this; 
    for (i = 0; i < max; i++) removeWorker (this, i); 
    for (i = 0; i < max; i++) executeWorker(this, i); 
} 

Pool createPool (int max) {
    DxlObject dx = new(); Pool this = PoolOf dx;
    setMaxSlots  (this, max); 
    setExecutors (this, create (max, 2)) 
    setResults   (this, create() ); 
    setQueue     (this, create() );
    
    setNextQueueIndex(this, 0); 
    setExecutionIndex(this, 0);
    return this; 
}

string getQueueResult (Pool this, int index) {
    string sResult = null; 
    find (getResults this, index, sResult); 
    return sResult; 
}

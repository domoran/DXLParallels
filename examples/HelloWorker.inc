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

void Hello_Worker_doWork (Worker this) {
    string sName = ""; 
    getWorkerArgument(this, "name", sName);
    string sResult = "Hello, " sName "!"; 
    setWorkerResult (this, sResult); 
}

Worker createHelloWorker (string sName) {
    Worker w = createWorker(); 
    setWorkerArgument(w, "name", sName);
    set_doWork (w, Hello_Worker_doWork); 
    return w
}

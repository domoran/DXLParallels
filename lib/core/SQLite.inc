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

struct SQLite {}; 

string gsSqlLiteExecutable = searchInIncludePath("examples/SqlExport/sqlite3.exe");
if (null gsSqlLiteExecutable) error "No SQL Lite found. Make sure it is on your include path!";  

DxlObject DxlObjectOf (SQLite this)     { return ((addr_ this ) DxlObject); }
SQLite      SQLiteOf    (DxlObject dx) { return ((addr_ dx) SQLite);    }

// +--- property: string DBFile ---+
string    getDBFile (SQLite this)      { return ((DxlObjectOf this)->"DBFile") string   }
void   setDBFile (SQLite this, string   value) { (DxlObjectOf this)->"DBFile" = value; }

// +--- property: string CurrentFileName ---+
string    getCurrentFileName (SQLite this)      { return ((DxlObjectOf this)->"CurrentFileName") string   }
void   setCurrentFileName (SQLite this, string   value) { (DxlObjectOf this)->"CurrentFileName" = value; }

// +--- property: string OutputFileName ---+
string    getOutputFileName (SQLite this)      { return ((DxlObjectOf this)->"OutputFileName") string   }
void   setOutputFileName (SQLite this, string   value) { (DxlObjectOf this)->"OutputFileName" = value; }

// +--- property: StreamEx CurrentFile ---+
StreamEx    getCurrentFile (SQLite this)      { return ((DxlObjectOf this)->"CurrentFile") StreamEx }
void   setCurrentFile (SQLite this, StreamEx   value) { (DxlObjectOf this)->"CurrentFile" = value; }

StreamEx createTempSQLFile (SQLite this) {
    string sqlFileName = tempFileName() ".sql" 
    StreamEx stEx = createStreamEx(write(sqlFileName, CP_UTF8));
    setCurrentFileName (this, sqlFileName); 
    setCurrentFile(this, stEx); 
    return stEx;  
}

SQLite createSQLite (string sDBFile, bool bOverwrite) {
    if (bOverwrite && fileExists_ sDBFile) deleteFile sDBFile; 
    DxlObject dx = new(); 
    SQLite this = addr_ dx; 
    setDBFile(this, sDBFile);

    StreamEx stEx = createTempSQLFile(this); // will set currentFile ...    
    string sOutput = tempFileName() ".txt";
    setOutputFileName(this, sOutput);
    serializeString(stEx, ".output stdout\n");
    
    return this; 
}

SQLite createSQLite (string sDBFile) { return createSQLite(sDBFile, false); }

string getCommandForExecution_ (SQLite this) {
    if (!null getCurrentFile(this)) { 
        closeStreamEx (getCurrentFile this);
        setCurrentFile(this, null StreamEx);
    } 
    
    return "cmd /C " '"' gsSqlLiteExecutable '"' " " (getDBFile this) " <" (getCurrentFileName this) " >" (getOutputFileName this);
}

void cleanup(SQLite this) {
    if (!null getOutputFileName(this)) deleteFile getOutputFileName(this);
    if (!null getCurrentFile(this)) deleteFile getCurrentFileName(this);
}

void deleteSQLite (SQLite &this) {
    cleanup(this); 
    DxlObject dx = DxlObjectOf(this); delete dx;  
} 

Buffer getResult (SQLite this) {
    Buffer buf = create(10000); 
    readFileWithoutLeak(getOutputFileName(this), buf, CP_UTF8);
    return buf;
}

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
string    getSQLCommandsFileName (SQLite this)      { return ((DxlObjectOf this)->"SQLCommandsFileName") string   }
void   setSQLCommandsFileName (SQLite this, string   value) { (DxlObjectOf this)->"SQLCommandsFileName" = value; }

// +--- property: StreamEx CurrentFile ---+
StreamEx    getSQLCommandsFile (SQLite this)      { return ((DxlObjectOf this)->"SQLCommandsFile") StreamEx }
void   setSQLCommandsFile (SQLite this, StreamEx   value) { (DxlObjectOf this)->"SQLCommandsFile" = value; }

// +--- property: string OutputFileName ---+
string    getOutputFileName (SQLite this)      { return ((DxlObjectOf this)->"OutputFileName") string   }
void   setOutputFileName (SQLite this, string   value) { (DxlObjectOf this)->"OutputFileName" = value; }

StreamEx createNewSQLCommandsFile (SQLite this) {
    string sqlFileName = tempFileName() ".sql" 
    StreamEx stEx = createStreamEx(write(sqlFileName, CP_UTF8));
    setSQLCommandsFileName (this, sqlFileName); 
    setSQLCommandsFile(this, stEx);
    
    string sOutput = tempFileName() ".txt";
    setOutputFileName(this, sOutput);
    stEx += ".output stdout\npragma journal_mode = off;\npragma synchronous=off;\n";
    return stEx;  
}

SQLite createSQLite (string sDBFile, bool bOverwrite) {
    if (bOverwrite && fileExists_ sDBFile) deleteFile sDBFile; 
    DxlObject dx = new(); 
    SQLite this = addr_ dx; 
    setDBFile(this, sDBFile);
    
    createNewSQLCommandsFile(this); // will set the command SQL File ...
        
    return this; 
}

SQLite createSQLite (string sDBFile) { return createSQLite(sDBFile, false); }

string getCommandForExecution_ (SQLite this) {
    if (!null getSQLCommandsFile(this)) { 
        closeStreamEx (getSQLCommandsFile this);
        setSQLCommandsFile(this, null StreamEx);
    } 
    
    return "cmd /C " '"' gsSqlLiteExecutable '"' " " (getDBFile this) " <" (getSQLCommandsFileName this) " >" (getOutputFileName this);
}

void cleanup(SQLite this) {
    if (!null getOutputFileName(this)) deleteFile getOutputFileName(this);
    if (!null getSQLCommandsFile(this)) deleteFile getSQLCommandsFileName(this);
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

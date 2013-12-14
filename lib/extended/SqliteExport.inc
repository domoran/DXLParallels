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
#include <lib/core/StreamEx.inc>
#include <lib/core/System.inc>
#include <lib/core/SQLite.inc>
#include <lib/core/binary.inc>
#include <lib/extended/SqliteExport.inc>
          
          ---------------------------------
*/

Buffer SqliteExportTempBuffer    = create(50000); 

void setupDoorsDatabase (SQLite db, bool allowItemStructureQuery) {
    StreamEx sqlFile = getSQLCommandsFile(db);

    sqlFile += "
               create table item_structure (item_id integer, parent integer, depth integer);
               create table items  (item_id integer PRIMARY KEY, parent integer, type TEXT, name TEXT);
               
               create table object_structure (module integer, absNo integer, parent_object integer, depth integer);
               create table objects (module integer, absNo integer, parent_object integer, deleted integer);
                
               create table attribute_values (module integer, absNo integer, attribute_name TEXT, richvalue TEXT, plainvalue TEXT);
               ";
               
    if (allowItemStructureQuery) {
        sqlFile += "
                    create trigger item_tree after insert on items BEGIN
                      insert into item_structure values (new.item_id, new.parent, 1); 
                      insert into item_structure select new.item_id, c.parent, c.depth + 1 from item_structure c where c.item_id = new.parent;
                    END;
        "; 
        
        sqlFile += "
                   create trigger objects_tree after insert on objects BEGIN
                     insert into object_structure values (new.module, new.absNo, new.parent_object, 1); 
                     insert into object_structure select new.module, new.absNo, c.parent_object, c.depth + 1 from object_structure c where c.module = new.module and c.absNo = new.parent_object;
                   END;
                   
                   create index objects_index on objects(module,absNo);
                   create index structure_parent_index on object_structure(module,absNo);
                   create index attribute_value_index on attribute_values(module,absNo);
        ";
    } 
}

Buffer SqliteExportBufRich  = create();  
Buffer SqliteExportBufPlain = create();  
Regexp reEscapeQuote = regexp2 "'";

void exportAttributeValues (StreamEx strmOut, Module m, Skip skAttributes) {
    AttrDef ad

    Skip skAttrs = skAttributes; 
    if (null skAttributes) {
        skAttrs = create();
        int count = 0;  
        for ad in m do {
            if (!ad.object) continue; 
            put (skAttrs, count++, ad); 
        }
    }
    
    string sModuleID = (hex2Int uniqueID m) ",";
    string sName     = null;

    Object o; for o in entire m do {
        int iNr = o."Absolute Number";
        string sInsertStatement = "\ninsert into attribute_values values (" sModuleID iNr ", '" 
        
        for ad in skAttrs do {
            string sName = ad.name;
            SqliteExportBufRich  = richTextWithOle o.sName
            SqliteExportBufPlain = o.sName
            if (!hasSpecificValue(o, ad)) continue; 
            
            if (length SqliteExportBufPlain == length SqliteExportBufRich && SqliteExportBufRich == SqliteExportBufPlain) SqliteExportBufPlain = ""; 
            
            SqliteExportTempBuffer += sInsertStatement
            SqliteExportTempBuffer += sName
            SqliteExportTempBuffer += "','"
            combineEscaped(SqliteExportTempBuffer, SqliteExportBufRich, reEscapeQuote, "'"); 
            SqliteExportTempBuffer += "','" 
            combineEscaped(SqliteExportTempBuffer, SqliteExportBufPlain, reEscapeQuote, "'"); 
            SqliteExportTempBuffer += "');"
        }

        if (length SqliteExportTempBuffer > 1000000) serializeAndEmptyBuffer(strmOut, SqliteExportTempBuffer);
    }
    
    SqliteExportTempBuffer += "\n";
    serializeAndEmptyBuffer(strmOut, SqliteExportTempBuffer);
}

void exportModule (StreamEx strmOut, Module m) {
    string sID = (hex2Int uniqueID m) ",";
    
    Object o; for o in entire m do {
        Object oParent = parent o
        SqliteExportTempBuffer += "insert into objects values("
        SqliteExportTempBuffer += sID;
        SqliteExportTempBuffer += o."Absolute Number" ", " 
        SqliteExportTempBuffer += ((null oParent) ? "0," : (oParent."Absolute Number" ","));
        SqliteExportTempBuffer += (isDeleted o ? "1" : "0") ");\n";
    }
    
    exportAttributeValues (strmOut, m, null); 
}
 

void exportFolders (StreamEx strmOut, Folder f, bool bRecurse, bool filterCallback(Item)) {
    string sID = uniqueID f
    string folderID = (hex2Int sID) ", '";
    Item I; for I in f do {
        if (filterCallback I) {
            sID = uniqueID I
            
            SqliteExportTempBuffer += "insert into items values (";
            SqliteExportTempBuffer += (hex2Int sID) ","; 
            SqliteExportTempBuffer += folderID
            SqliteExportTempBuffer += (type I) "','" (name I) "');\n";
    
            if ((type I == "Folder" || type I == "Project") && bRecurse) exportFolders(strmOut, folder fullName I, true, filterCallback); 
        }
    }
    
    serializeAndEmptyBuffer(strmOut, SqliteExportTempBuffer); 
}

void exportModule (StreamEx strmOut, Item I) {
    if (type I == "Formal") {
        noError(); Module m = read(fullName I, false, true); print lastError();
        if (!null m) {
            exportModule(strmOut, m)
            close (m); 
        }
    }
}

void exportModules (StreamEx strmOut, Folder f, bool bRecurse, bool filterCallback(Item)) {
    Item I; for I in f do {
        if ((type I == "Folder" || type I == "Project") && bRecurse) { 
            exportModules(strmOut, folder fullName I, true, filterCallback);
        } 
        
        if (filterCallback I) {
            exportModule (strmOut, I); 
        }
    }
    
    serializeAndEmptyBuffer(strmOut, SqliteExportTempBuffer); 
}


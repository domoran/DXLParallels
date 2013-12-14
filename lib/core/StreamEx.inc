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

+++++ Warning - Performance Considerations - +++++

StreamEx allows to write library functions that can output to a buffer, a file, a config file or a network socket. For this
virtual functions are used, to redirect the calls to the correct function. These virtual function call add an overhead to the 
call. These result in penalties when called often and with only a few bytes of data. The following table shows the penalties: 

Base (1EFC) =   1,24E-06            

HD Speed                                       Penalty               Penalty
   MB/s    Bytes/1EFC        Bytes/5EFC      for 30 Bytes         for 300 Bytes
   1           1,24            6,18             20,6%                 2,1%
   3           3,71           18,53             61,8%                 6,2%
   5           6,18           30,88            102,9%                10,3%
   10         12,35           61,75            205,8%                20,6%
   50         61,75          308,75           1029,2%               102,9%
   100       123,50          617,50           2058,3%               205,8%

If the harddisc speed is very high, then the time to transfer 30Bytes is very small and the overhead adds a large penalty to
the transfer time. If the hardisc speed is low, the penalty will not be so large. Therefore it is advisable to use a buffer 
to buffer the transfers: 

Buffer buf = create(); 
StreamEx strm = ...

while (mainLoop) {
    buf += data 
    if (length buf > 300) { appendBuffer(strm, buf); setempty buf; }
}   

*/ 


// Provide functions for reading and writing files without leaks 

// *************************** Each time a file is opened for read or write a Stream is leaked ***********************
// *************************** Therefore we use eval_ to write and read files ****************************************

Buffer *gBufWriteTextContentBuffer
Buffer gBufTextFilenameBuffer = create()
int gIntTextfileCodepage = CP_UTF8
bool gBoolAppendTextFile = false

string gStrWriteBufferCode = "
    // we use that double pointer to not have to copy the buffer contents
    Buffer **bufTextContentBufferDblPtr  = addr_ " ((addr_ (&gBufWriteTextContentBuffer)) int) "
    Buffer bufTextFilenameBuffer = (addr_ " ((addr_ gBufTextFilenameBuffer) int) ") Buffer
    int &iCodepage = addr_ " ((addr_ (&gIntTextfileCodepage)) int) "
    bool &bAppendRef  = addr_ " ((addr_ (&gBoolAppendTextFile)) int) "
    bool bAppend = bAppendRef
    
    Buffer &bufText = **bufTextContentBufferDblPtr
    string strErr = null

    // Create file and write contents; and catch errors if any
    Stream os = null
    
    noError
    if (bAppend == false) {
        os = write( bufTextFilenameBuffer \"\", iCodepage )
    } else { 
        os = append ( bufTextFilenameBuffer \"\", iCodepage )
    }
    
    if (!null (strErr = lastError())) { return_ strErr; halt }
    
    noError()
    
    // Write needed contents to the batch file stream:
    os << bufText
    
    // Save stream contents to file and close it
    close( os )
    
    return_ lastError()
"

string writeOrAppendFileWithoutLeak (string file_Name, Buffer fileContent, int codepage, bool bAppend) {
    gBufTextFilenameBuffer = file_Name
    gBufWriteTextContentBuffer  = &fileContent
    gIntTextfileCodepage = codepage
    gBoolAppendTextFile = bAppend
    
    noError()    
    string msgEval = eval_ gStrWriteBufferCode
    string strErr = lastError()
    if (msgEval != "") return msgEval 
    return strErr 
}


string gStrFileReadCode = "
    // we use that double pointer to not have to copy the buffer contents
    Buffer **bufTextContentBufferDblPtr  = addr_ " ((addr_ (&gBufWriteTextContentBuffer)) int) "
    Buffer bufTextFilenameBuffer = (addr_ " ((addr_ gBufTextFilenameBuffer) int) ") Buffer
    int &iCodepage = addr_ " ((addr_ (&gIntTextfileCodepage)) int) "
    Buffer &bufText = **bufTextContentBufferDblPtr
    
    Stream is = read (bufTextFilenameBuffer \"\", iCodepage)
    
    Buffer bufInput = create(1000000)
    // Iterate over all content of the file
    while (true)
    {
        is >> bufInput
        
        // Process data if some could be read of stream is reached
        if (length bufInput > 0) {         
            combine(bufText, bufInput, 0)
        }
        
        // Prepare next iteration
        setempty( bufInput )
        
        if (end of is) break
    }

    delete( bufInput )
"

string readFileWithoutLeak (string file_Name, Buffer fileContent, int codepage) {
    gBufTextFilenameBuffer = file_Name
    gBufWriteTextContentBuffer  = &fileContent
    gIntTextfileCodepage = codepage
    
    noError()    
    string msgEval = eval_ gStrFileReadCode
    string strErr = lastError()
    if (msgEval != "") return msgEval 
    return strErr 
}

string readTextFile (string sFileName) {
    Buffer buf = create(); 
    string sErr = readFileWithoutLeak(sFileName, buf, CP_UTF8)
    if (!null sErr) error "Could not read file: " sFileName ": " sErr
    string sResult = stringOf buf
    delete buf
    return sResult
}

void writeFile(string sFilename, string sValue) {
    Buffer buf = create(); 
    buf = sValue; 
    writeOrAppendFileWithoutLeak (sFilename, buf, CP_UTF8, false);
    delete buf
}

void writeFile(string sFilename, Buffer buf) {
    writeOrAppendFileWithoutLeak (sFilename, buf, CP_UTF8, false);
}

void appendFile(string sFilename, string sValue) {
    Buffer buf = create(); 
    buf = sValue; 
    writeOrAppendFileWithoutLeak (sFilename, buf, CP_UTF8, true);
    delete buf
}

void appendFile(string sFilename, Buffer buf) {
    writeOrAppendFileWithoutLeak (sFilename, buf, CP_UTF8, true);
}

struct StreamEx {};
struct StreamType {}

StreamType Buffer_Stream = addr_ 1;
StreamType File_Stream   = addr_ 2;
StreamType IPC_Stream    = addr_ 3;

// atomatic generated interface for class StreamEx
Array garStreamExMemory = create(50, 4)
int   giStreamExHandleCount = 0
Skip skFreeStreamExNodes = create()
int giFreeStreamExNodes = 0

struct StreamEx {}

// constructor & destructor for List
StreamEx createStreamEx_ () {
    StreamEx x = null
     
    if (giFreeStreamExNodes > 0) {
        if (!find(skFreeStreamExNodes, giFreeStreamExNodes--, x)) error "StreamEx memory error!"
    } else {
        x = (addr_ (++giStreamExHandleCount) ) StreamEx;
    }   

    return x
}

// +---- Property StreamObj (Slot 1) ----+
void getStream (StreamEx this, _y &ref) { int &iref = addr_ ref; iref = (get(garStreamExMemory, ((addr_ this) int)     , 1)) int; }
void setStream (StreamEx this, _y val)  { put(garStreamExMemory, val, ((addr_ this) int), 1); }

// +---- Property StreamTypeObj  (Slot 2) ----+
StreamType getStreamType (StreamEx this)     { return (get(garStreamExMemory, ((addr_ this) int)     , 2)) StreamType; }
void   setStreamType (StreamEx this, StreamType val) { put(garStreamExMemory, val, ((addr_ this) int), 2); }


// +--- Virtual Method  (Slot 3) : void serializeBuffer() ---+
void set_serializeBuffer (StreamEx this, void serializeStringFunction (StreamEx, Buffer)) { int ad = (addr_ serializeStringFunction) int; put(garStreamExMemory, ad, ((addr_ this) int), 3); }
void call_serializeBuffer(StreamEx this, Buffer buf, void serializeBufferFunction (StreamEx, Buffer)) { serializeBufferFunction(this, buf) };

void serializeBuffer (StreamEx this, Buffer buf) {
    int ad = (get(garStreamExMemory, ((addr_ this) int), 3)) int;
    if (ad == 0) error "No serializeBuffer function was defined for this serializer!";
    call_serializeBuffer(this, buf, addr_ ad); 
}

void serializeAndEmptyBuffer (StreamEx this, Buffer buf) {
    int ad = (get(garStreamExMemory, ((addr_ this) int), 3)) int;
    if (ad == 0) error "No serializeBuffer function was defined for this serializer!";
    call_serializeBuffer(this, buf, addr_ ad);
    setempty buf;   
}

// +--- Virtual Method  (Slot 4) : void serializeString() ---+
void set_serializeString (StreamEx this, void serializeStringFunction (StreamEx, string)) { int ad = (addr_ serializeStringFunction) int; put(garStreamExMemory, ad, ((addr_ this) int), 4); }
void call_serializeString(StreamEx this, string str, void serializeStringFunction (StreamEx, string)) { serializeStringFunction(this, str) };

void serializeString (StreamEx this, string buf) {
    int ad = (get(garStreamExMemory, ((addr_ this) int), 4)) int;
    if (ad == 0) error "No serializeString function was defined for this serializer!";
    call_serializeString(this, buf, addr_ ad); 
}

void StreamStream_appendString (StreamEx this, string val) {
    Stream st = null; 
    getStream(this, st); 
    st << val
}

void StreamStream_appendBuffer (StreamEx this, Buffer val) {
    Stream st = null; 
    getStream(this, st); 
    st << val
}

void BufferStream_appendString (StreamEx this, string val) {
    Buffer st = null; 
    getStream(this, st); 
    st += val; 
}

void BufferStream_appendBuffer (StreamEx this, Buffer val) {
    Buffer st = null; 
    getStream(this, st); 
    combine(st, val, 0); 
}


void closeStreamEx(StreamEx this) {
    StreamType sType = getStreamType(this);
    if (sType == File_Stream) { Stream st = null; getStream(this, st); if (!null st) close(st); } 
}

StreamEx createStreamEx (Stream strm) {
    StreamEx this = createStreamEx_();    
    setStream(this, strm);
    setStreamType(this, File_Stream);
    set_serializeString(this, StreamStream_appendString); 
    set_serializeBuffer(this, StreamStream_appendBuffer); 
    return this; 
}

StreamEx createStreamEx (Buffer strm) {
    StreamEx this = createStreamEx_();    
    setStream(this, strm);
    setStreamType(this, Buffer_Stream)
    set_serializeString(this, BufferStream_appendString); 
    set_serializeBuffer(this, BufferStream_appendBuffer); 
    return this; 
}

// Syntax sugar 
void ::+= (StreamEx this, string val) {
    serializeString(this, val);
}

void ::+= (StreamEx this, Buffer buf) {
    serializeBuffer(this, buf);
}

void ::<- (StreamEx this, Buffer buf) {
    serializeBuffer(this, buf);
    setempty(buf);  
}

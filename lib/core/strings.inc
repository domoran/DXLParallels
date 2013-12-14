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
bool valueExists (Skip sk, string sValue) { string s; for s in sk do { if (s == sValue) return true; }; return false } 

void searchStrings(Buffer bufSource, string sPattern,  Skip skResult, bool keepEmpty, bool noDuplicates, int iGroup) {
    if (null sPattern) error "null pattern was passed to searchStrings!"
    // print "Searching for " sPattern " in " bufSource "\n";
    int iPos = 0, iPosOld = -1, iCount = 0; 
    Regexp rePattern = regexp2 sPattern;
    string sMatch = null; for sMatch in skResult do iCount++; // count items in Skip ... 
    
    int iLen = length bufSource;
    while (search(rePattern, bufSource, iPos) && iPos < iLen) {
        int iEndMatch = end 0, iStart = start iGroup, iEnd = end iGroup; 
        if (iEndMatch < 0) error "Null match error in searchStrings at iPos = " iPos " iStart " iStart " iEnd: " iEnd " End Match: " iEndMatch "\n";
        if (iEnd >= 0) sMatch = bufSource[iPos+iStart:iPos+iEnd]; else sMatch = ""; 
        if ( (sMatch != ""  || keepEmpty                     ) && 
             (!noDuplicates || !valueExists(skResult, sMatch)) 
           ) {
                put(skResult, iCount++, sMatch);
                // print "Putting with key " (iCount - 1) ":" sMatch "\n";
        }
        iPos += iEndMatch + 1;
    }
    
    delete rePattern;
    return skResult; 
}

void combineEscaped (Buffer bufTarget, Buffer bufSource, Regexp reEscape, string sEscapeSequence) {
    int iPos = 0; 
    int iLen = length bufSource;
    while (search(reEscape, bufSource, iPos) && iPos < iLen) {
        int iStartMatch = start 0; 
        int iEndMatch   = end 0; 
        if (iEndMatch < 0) error "Null match error in searchStrings at iPos = " iPos " iStartMatch " iStartMatch " End Match: " iEndMatch "\n";
        
        // we found an escape sequence, so first add the string before the match
        if (iStartMatch > 0) combine(bufTarget, bufSource, iPos, iPos + iStartMatch - 1);
        
        // add the escaped sequence
        bufTarget += sEscapeSequence; 
        // print "Adding: " bufSource " IPos = " iPos " Start = " iStartMatch " End = " iEndMatch "\n"; 
        combine(bufTarget, bufSource, iPos + iStartMatch, iPos + iEndMatch); 
        iPos += iEndMatch + 1;
    }
    if (iPos < iLen) combine(bufTarget, bufSource, iPos, iLen-1);
}  

string joinStrings(Skip sk, string sSep) {
    Buffer buf = create();
    int count = 0;  
    string sVal; for sVal in sk do {
        if (count++ > 0) buf += sSep; 
        buf += sVal;
    }
    string sResult = stringOf buf; 
    delete buf; 
    return sResult; 
}
    


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
        }
        iPos += iEndMatch + 1;
    }
    
    delete rePattern;
    return skResult; 
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
    


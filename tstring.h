/*GPL*START*
 * tstring - NULL byte tolerant sophisticated string class
 * 
 * Copyright (C) 1998 by Johannes Overmann <overmann@iname.com>
 * Copyright (C) 2008 by Tong Sun <suntong001@users.sourceforge.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *GPL*END*/  

#ifndef _ngw_tstring_h_
#define _ngw_tstring_h_

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "ttypes.h"
#include "tminmax.h"
#include "trelops.h"
#include "terror.h"
#include "tbaseexception.h"



template<class T> class TArray;

/**@name null tolerant string class */
/*@{*/
/// null tolerant string class
class TString {
 public:
   // flags for operator(int,int)
   enum {START=-1002, END=-1001};      
   // flags for scanToken()
   enum {ALPHA=1, NUM=2, DIGIT=2, LOWER=4, UPPER=8, PRINT=16, XDIGIT=32, 
      SPACE=64, ALNUM=1|2, PUNCT=128, CNTRL=256, GRAPH=1024,
      ALL=2048, NONE=0};
   /// case flags for modify case
   enum {NOT=0, CAPITALIZE=-1};
 private:
   // internal string representation
   class Rep {
    public:
      int len; // length without term 0 byte
      int mem; // allocated mem without term 0 byte
      int ref; // reference count (>=1)
      bool vulnerable; // true == always grab by clone, never by reference
      //                  (the string has become vulnerable to the outside)
      // char data[mem+1] string bytes follow (+1 for term 0 byte)
      
      // return pointer to string data
      char *data() {return (char *)(this + 1);} // 'this + 1' means 'the byte following this object'
      // character access
      char& operator[] (int i) {return data()[i];}
      // reference
      Rep* grab() {if(vulnerable) return clone(); ++ref; return this;}
      // dereference
      void release() {if(--ref == 0) delete this;}
      // copy this representation
      Rep *clone(int minmem = 0) {
	 Rep *p = create( /* Max */ minmem>len ? minmem:len);   
	 p->len = len;
	 memcpy(p->data(), data(), len+1);
	 return p;}
      // terminate string with 0 byte
      void terminate() {*(data()+len) = 0;} // set term 0 byte
      
      // static methods
      // operator new for this class
      static void * operator new (size_t size, int tmem) {
	 return ::operator new (size + tmem + 1);}
      
      // create a new representation
      static Rep *create(int tmem) {
	 int m = 32;
	 while((m-1-int(sizeof(Rep))) < tmem) m<<=1;
	 Rep *p = new (m-1-sizeof(Rep)) Rep;
	 p->mem = m-1-sizeof(Rep); p->ref = 1; p->vulnerable = false;
	 return p;}
            
      // return pointer to the null string representation
      static Rep * nulRep() {if(nul == 0) createNulRep(); return nul;}

      // return pointer to the zero string representation (string conatining a literal 0: "0" (and not "\0"))
      static Rep * zeroRep() {if(zero == 0) createZeroRep(); return zero;}
	 
      // create null string representation
      static void createNulRep();
      
      // create zero string representation
      static void createZeroRep();

    private:
      // static null string ("") representation
      static Rep* nul;
      static int nul_mem[10];
      // static zero string ("0") representation
      static Rep* zero;
      static int zero_mem[10];
      
      // forbid copy/construction
      //Rep();
      //Rep(const Rep&);
      Rep& operator = (const Rep&);
   };
   
 public:
   /**@name constructor & destructor */
   /*@{*/
   /// default construction
   TString(): rep(Rep::nulRep()->grab()) {}
   /// copy construction
   TString(const TString& a):rep(a.rep->grab()) {}
   /// init from cstring
   TString(const char *s):rep(0) {
      if(s){
	 int len = strlen(s);
	 rep = Rep::create(len);
	 rep->len = len;
	 strcpy(rep->data(), s);
      } else rep = Rep::nulRep()->grab();
   }
   /// extract bytearray s of length len 
   TString(const char *s, int len):rep(0) {
      if(s && len){
	 rep = Rep::create(len);
	 rep->len = len;
	 if(len) memcpy(rep->data(), s, len);      
	 rep->terminate();
      } else rep = Rep::nulRep()->grab();
   }
   /// create string of chars c with length n
   explicit TString(char c, int n):rep(0) {
      if(n>0) {
	 rep = Rep::create(n);
	 rep->len = n;
	 if(n) memset(rep->data(), c, n);      
	 rep->terminate();      
      } else rep = Rep::nulRep()->grab();
   }
   /// char to string conversion
   TString(char c):rep(0) {rep = Rep::create(1); rep->len = 1; (*rep)[0] = c; rep->terminate();}
   /// int to string conversion
   TString(int i):rep((i==0)?(Rep::zeroRep()->grab()):(Rep::nulRep()->grab())) {if(i) sprintf("%d", i);}
   /// int to string conversion with format
   TString(int i, const char *format):rep(Rep::nulRep()->grab()) {sprintf(format, i);}
   /// double to string conversion
   TString(double d, const char *format = "%g"):rep(Rep::nulRep()->grab()) {sprintf(format, d);}
   /// destructor
   ~TString() {rep->release();}
   /*@}*/
      
   
   /**@name main interface */
   /*@{*/
   /// return length in bytes
   int len() const {return rep->len;}
   /// clear string
   void empty() {replaceRep(Rep::nulRep()->grab());}
   /// implicit conversion to c-style string
   operator const char * () const {return rep->data();}
   /// explicit conversion, needed for {\tt printf}
   const char * operator * () const {return rep->data();}
   /// direct data access: {\bf dangerous! do not use!}
   char * data() {invulnerableDetach(); return rep->data();}
   /// true if string not empty, else false
   operator bool() const {return rep->len > 0;}
   /// append string
   TString& operator += (const TString& a) {if(a) {append(a.rep->data(), a.rep->len);} return *this;}
   /// append cstring
   TString& operator += (const char *a) {if(a) append(a, strlen(a)); return *this;}
   /// append byte array a of length len
   TString& append(const char *a, int alen) {
      if(a) {
	 detachResize(rep->len + alen);
	 memcpy(rep->data() + rep->len, a, alen);
	 rep->len += alen;
	 rep->terminate();
      }
      return *this;
   }
   /// assign string a to this
   TString& operator = (const TString& a) 
   {if(&a != this) {rep->release(); rep = a.rep->grab();} return *this;}
   /// direct character access: const/readonly
   char operator [] (int i) const /* throw(IndexOutOfRange) */ {
      if((unsigned int)i <= (unsigned int)rep->len) return (*rep)[i];
      throw IndexOutOfRange(i, rep->len);
   }
   /// direct character access: read/write
   char& operator [] (int i) /* throw(IndexOutOfRange) */ {
      if((unsigned int)i < (unsigned int)rep->len) {detach(); return (*rep)[i];}
      throw IndexOutOfRange(i, rep->len);
   } 
   /// substring extraction (len=end-start)
   TString operator() (int start, int end) const /* throw(InvalidRange) */ {
      if(start==START) start = 0;
      if(end  ==END  ) end = rep->len;
      if(((unsigned int)start > (unsigned int)rep->len)||
	 ((unsigned int)end   > (unsigned int)rep->len)||(end < start)) {
	 throw InvalidRange(start, end, rep->len);
      }
      return TString(rep->data()+start, end-start); 
   }
   /// ASCII to number conversion
   bool toLong(long& long_out, int base = 0) const;
   bool toInt(int& int_out, int base = 0) const;
   bool toDouble(double& double_out) const;
   bool toBool(bool& bool_out) const;
   /*@}*/
   
      
   /**@name scanning */
   /*@{*/
   /// return a scanned token with scanner
   TString scanToken(int& scanner, int flags, 
		  const char *allow=0, const char *forbid=0, 
		  bool allow_quoted=false) const;
   /// scan a token or quoted string to out with scanner
   TString scanString(int& scanner, int flags, 
		  const char *allow=0, const char *forbid=0) const {
		     return scanToken(scanner, flags, allow, forbid, true);}
   /// scan a token up to char upto
   TString scanUpTo(int& scanner, char upto) const {
      int start(scanner);
      while((uint(scanner)<uint(rep->len))&&((*rep)[scanner]!=upto)) ++scanner;
      return operator()(start, scanner);}
   /// scan a token to out up to chars upto
   TString scanUpTo(int& scanner, const char *upto) const {
      int start(scanner);
      while((uint(scanner)<uint(rep->len))&&(strchr(upto, (*rep)[scanner])==0))
	++scanner;
      return operator()(start, scanner);}
   /// return the rest of the scanned string
   TString scanRest(int& scanner) const {if(uint(scanner)<uint(rep->len)) {
      int start(scanner);scanner=rep->len;return operator()(start, scanner);
   } return TString();}   
   /// skip spaces
   void skipSpace(int& scanner) const 
   {while((uint(scanner)<uint(rep->len))&&isspace((*rep)[scanner]))++scanner;}
   /// perhaps skip one char c
   void perhapsSkipOneChar(int& scanner, char c) const 
   {if((uint(scanner)<uint(rep->len))&&((*rep)[scanner]==c)) ++scanner;}
   /// return true if the end of string (eos) is reached
   bool scanEOS(int scanner) const 
   {if(scanner >= rep->len) return true; else return false;}
   
   
   /// return the last character in the string or 0 if empty
   char lastChar() const {return rep->len?(*rep)[rep->len-1]:0;}
   /// return the first character in the string or 0 if empty
   char firstChar() const {return (*rep)[0];}
   /// return true if entire string consists of whitespace
   bool consistsOfSpace() const;
   /// return true if string has prefix 
   bool hasPrefix(const TString& prefix) const;
   /// return true if string has suffix 
   bool hasSuffix(const TString& suffix) const;
   /// return index of first occurence of char c or -1 if not found
   int firstOccurence(char c) const;
   /// remove whitespace at beginning and end 
   void cropSpace();
   /// remove whitespace at end
   void cropSpaceEnd();
   /// collapse whitespace 
   void collapseSpace();
   /// replace char from with char to
   void translateChar(char from, char to);
   /// expand unprintable chars to C-style backslash sequences
   void expandUnprintable(void);
   /// backslashify backslash and quotes 
   void backslashify(void);
   /// compile C-style backslash sequences back to unprintable chars
   void compileCString(void);   
   /**
    remove quotes
    @param allow_bslash true == backslashing allowed to protect quotes
    @param crop_space   true == remove leading/trailing spaces not protected by quotes
    */
   void unquote(bool allow_bslash = true, bool crop_space = true);
   /// return and remove the first words that fit into a string of length max
   TString getFitWords(int max); // throw(InvalidWidth);
   /// remove the first words that fit into a string of length max and return in block format
   TString getFitWordsBlock(int max); // throw(InvalidWidth);
   /// truncate to maximal length max
   void truncate(int max);
   /// remove html tags (level == number of open brakets before call, init:0)
   void removeHTMLTags(int& level);
   /*@}*/
      
   /**@name search/replace */
   /*@{*/
   /// replace substring search with replace, return number of replacements (not regexp, use TRegEx to match regular expressions)
   int searchReplace(const TString& search, const TString& replace,
		     bool ignore_case=false, bool whole_words=false, 
		     bool preserve_case=false, int progress=0,
		     const TString& pre_padstring=TString(), 
		     const TString& post_padstring=TString(), TArray<int> *match_pos=0);
   /// return number of occurences of pat (not regexp)
   int search(const TString& pat, 
	      bool ignore_case=false, bool whole_words=false,
	      int progress=0, TArray<int> *match_pos=0) const; // throw(StringIsEmpty);
   /*@}*/
      
   /**@name file I/O */
   /*@{*/
   /// read line from file like fgets, no line length limit
   bool readLine(FILE *file);
   /// write string to file, return number of bytes written
   int write(FILE *file) const;
   /// read len bytes from file to string, return bytes read
   int read(FILE *file, int len); // throw(InvalidWidth);
   /// read whole file into one string, return 0 on success -x on error
   int readFile(const char *filename);
   /*@}*/
   
   /**@name filename manipulation */
   /*@{*/
   /// remove leading path from filename
   void extractFilename();   
   /// remove part after last slash
   void extractPath();   
   /// add a slash at the end if it is missing
   void addDirSlash();
   /// remove last char if last char is a slash
   void removeDirSlash();      
   /// extract part after the last period (empty string if no extension, leading period is ignored)
   void extractFilenameExtension();
   /// make paths comparable (kill multislash, dots and resolve '..')
   void normalizePath();
   /// check for absolute path
   bool isAbsolutePath() const {if((*rep)[0]=='/') return true; return false;}
   /// get truncated filename (for printing puroses)
   TString shortFilename(int maxchar) const;
   /*@}*/
   
   /**@name misc */
   /*@{*/
   /// get percentage of nonprintable and nonspace chars (0.0 .. 100.0)
   double binaryPercentage() const;
   /// check for 0 in string (then its not a real cstring anymore)
   bool containsNulChar() const {rep->terminate(); 
      if(int(strlen(rep->data())) != rep->len) return true; else return false;}
   /// get a pointer to the at most max last chars (useful for printf)
   const char *pSuf(int max) const {return rep->data()+((max>=rep->len)?0:(rep->len-max));}
   /// sprintf into this string
   void sprintf(const char *format, ...) {
      va_list ap;
      int ret;
      va_start(ap, format);
#ifdef __STRICT_ANSI__
      // this is the unsecure and dirty but ansi compatible version
      // heuristic size, very large, may be still too small
      detachResize((1024 + 16*strlen(format)) * 2);      
      ret = vsprintf(rep->data(), format, ap); // not secure! may write out of bounds!
      if(ret >= rep->mem/2)
	 throw StringTooLong();
#else
      // this is the clean version (never overflows)
      int size = 32/4;
      do { 
	 size *= 4; // fast increase, printf may be slow
	 detachResize(size);
	 ret = vsnprintf(rep->data(), size, format, ap); 
      } while((ret == -1)||(ret>=size));
#endif
      va_end(ap);
      rep->len = ret;
   }
   /*@}*/
   
   /**@name case */
   /*@{*/
   /// convert to lower case
   void lower();
   /// convert to upper case
   void upper();
   /// convert to lower case, first char upper case
   void capitalize();
   /// check for lower case, empty string returns false      
   bool isLower() const;
   /// check for upper case, empty string returns false      
   bool isUpper() const;
   /// check for capitalized case, empty string returns false      
   bool isCapitalized() const;
   /*@}*/
      
 public:
   /**@name detach methods */
   /*@{*/
   /// detach from string pool, you should never need to call this
   void detach() {if(rep->ref > 1) {rep->release(); rep = rep->clone();}}
   // no, there is *not* a dangling pointer here (ref > 1)
   /** detach from string pool and make sure at least minsize bytes of mem are available
    (use this before the dirty version sprintf to make it clean)
    (use this before the clean version sprintf to make it fast)
    */
   void detachResize(int minsize) {
      if((rep->ref==1) && (minsize <= rep->mem)) return;
      Rep *p = rep->clone(minsize);
      rep->release(); 
      rep = p;
   }
   /// detach from string pool and declare that string might be externally modified (the string has become vulnerable)
   void invulnerableDetach() {detach(); rep->vulnerable = true;}
   /*@}*/
   
   /**@name exceptions */
   /*@{*/
   class StringTooLong: public TException {}; // for unsecure version of sprintf
   class StringIsEmpty: public TException {}; // for search (pattern)
   class InvalidWidth: public TException {
    public:
      InvalidWidth(int width): width(width) {}
      int width;
   };
   class InvalidRange: public TException {
    public:
      InvalidRange(int lower, int upper, int length): lower(lower), upper(upper), length(length) {}
      int lower, upper, length;
   };
   class IndexOutOfRange: public TIndexOutOfRangeException {
    public:
      IndexOutOfRange(int index, int length): TIndexOutOfRangeException(0, index, length-1) {}
   };
   /*@}*/

 private:
   // hidden string representation
   Rep *rep;
   
   // private methods
   void replaceRep(Rep *p) {rep->release(); rep = p;}

 public:
   // compare helpers
   static int _string_cmp(const TString& s1, const TString& s2) {
      int r = memcmp(s1.rep->data(),s2.rep->data(),tMin(s1.rep->len,s2.rep->len));
      if(r) return r;
      if(s1.rep->len > s2.rep->len) return +1;
      if(s1.rep->len < s2.rep->len) return -1;
      return 0;
   }
   static bool _string_equ(const TString& s1, const TString& s2) {
      if(s1.rep->len != s2.rep->len) return false;
      return memcmp(s1.rep->data(), s2.rep->data(), s1.rep->len)==0;
   }
};




/**@name concat operators */
/*@{*/
///
inline TString operator + (const TString& s1, const TString& s2) {
   TString r(s1); r += s2; return r; }
///
inline TString operator + (const char *s1, const TString& s2) {
   TString r(s1); r += s2; return r; }
///
inline TString operator + (const TString& s1, const char *s2) {
   TString r(s1); r += s2; return r; }
///
inline TString operator + (char s1, const TString& s2) {
   TString r(s1); r += s2; return r; }
///
inline TString operator + (const TString& s1, char s2) {
   TString r(s1); r += TString(s2); return r; }
/*@}*/



/**@name compare operators */
/*@{*/
///
inline bool operator == (const TString& s1, const TString& s2) {return TString::_string_equ(s1, s2);}
///
inline bool operator == (const TString& s1, const char   *s2) {return (strcmp(s1, s2)==0);}
///
inline bool operator == (const char   *s1, const TString& s2) {return (strcmp(s1, s2)==0);}
///
inline bool operator != (const TString& s1, const char   *s2) {return (strcmp(s1, s2)!=0);}
///
inline bool operator != (const char   *s1, const TString& s2) {return (strcmp(s1, s2)!=0);}
///
inline bool operator <  (const TString& s1, const TString& s2) {return (TString::_string_cmp(s1, s2) < 0);}
///
inline bool operator <  (const TString& s1, const char   *s2) {return (strcmp(s1, s2) < 0);}
///
inline bool operator <  (const char   *s1, const TString& s2) {return (strcmp(s1, s2) < 0);}
///
inline bool operator >  (const TString& s1, const char   *s2) {return (strcmp(s1, s2) > 0);}
///
inline bool operator >  (const char   *s1, const TString& s2) {return (strcmp(s1, s2) > 0);}
/*@}*/


/**@name misc friends and nonmembers */
/*@{*/
/// split string into pieces by characters in c-str separator
TArray<TString> split(const TString& s, const char *separator,
		     bool allow_quoting=false,
		     bool crop_space=false);

/// join, reverse the effect of split
TString join(const TArray<TString>& a, const TString& separator);

/// try to preserve case from 'from' to 'to' and return altered 'to' with case from 'from'
TString preserveCase(const TString& from, const TString& to);

/// modify case 
inline TString modifyCase(const TString& s, int _case) {
   TString r(s);
   switch(_case) {
    case TString::UPPER:      r.upper(); break;
    case TString::LOWER:      r.lower(); break;
    case TString::CAPITALIZE: r.capitalize(); break;
    default: break;      
   }
   return r;
}


/// load text file to array of strings
TArray<TString> loadTextFile(const char *fname);
/// load text file to array of strings
TArray<TString> loadTextFile(FILE *file);


/*@}*/
/*@}*/


#endif

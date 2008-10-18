/*GPL*START*
 * 
 * Copyright (C) 1998 by Johannes Overmann <overmann@iname.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * *GPL*END*/  

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "tstring.h"
#include "tarray.h"
#include "texception.h"




// todo: 
// make Split,Unquote,ReadLine,extractFilename,extractPath 0 byte safe


// 1997:
// 01:45 11 Jun split(): backslash behavior fixed (601 lines)
// 23:50 11 Jun strings may contain 0 bytes
// 12:00 19 Jun some filename extracting added
// 17:00 19 Jun more sophisticated search: ignore_case and whole_words
// 02:00 08 Jul substring extraction via operator() (start,end)
// 02:00 31 Jul new ContainsNulChar, new ReadFile, fixed \ \\ in ExpUnPrint
// 12:00 08 Aug new Upper Lower Capitalize
// 23:30 19 Aug improved collapseSpace()
// 00:00 27 Aug cropSpace() bug fixed (1 byte out of bound zero write)
// 20:00 30 Aug now cons accept 0 pointer as empty string
// 21:00 30 Aug addDirSlash() added (809 lines)
// 13:00 02 Sep isLower ... added, preserve_case for SearchReplace added (867)
// 23:45 16 Dec normalizePath() added
// 15:00 24 Dec started conversion to Rep reference model
// 18:00 27 Dec finished. debugging starts ... :)

// 1998:
// 00:30 09 Jan scanTools started (cc=817) (h=462)
// 00:05 12 Jan compare operators fixed (0 byte ...)
// 19:00 09 Oct zeroRep and fast string(int i) for i=0 
// 14:30 10 Oct xc16emu emuwid.s problem solved: memset()
// 14:36 10 Oct string(0) 80 times faster than string(1)! (zero_rep)
// 01:53 17 Oct createNulRep and createZeroRep non inline

// 1999:
// 14:55 31 Jan +=string speedup for empty string (cc=919, h=532)
// 15:08 31 Jan searchReplace: pre/post_padstring added
// 00:36 03 Feb getFitWordsBlock added (954)
// 23:02 04 Feb search/searchReplace match_pos added (954)
// 23:49 15 Feb class string renamed to class TString, tappframe simplified (1003)
// 00:46 16 Feb toLong/toDouble/toInt/toBool added (from old str2value.cc) (1016)
// 23:51 03 Mar cropSpaceEnd added, getFitWords space semantics change
// 23:46 13 Apr trelops.h replaces != and > operator (1034)
// 00:31 16 Apr started: replace fatalErrors by exceptions
// 23:48 20 Aug remove html tags added
// 22:17 09 Dec added operator != and > because trelops will not instantiate them for two different types



// global static null and zero rep members
TString::Rep* TString::Rep::nul = 0;
int TString::Rep::nul_mem[10];
TString::Rep* TString::Rep::zero = 0;
int TString::Rep::zero_mem[10];


// non inline Rep implementations

// create null string representation
void TString::Rep::createNulRep() {
   nul = (Rep *)nul_mem;
   memset(nul, 0, sizeof(int)*10); // assembler error without this (unnecessary) line, ... strange 
   nul->len = 0;
   nul->mem = 1;
   nul->ref = 1; // never modify/delete static object
   nul->vulnerable = false;
   nul->terminate();
}

// create zero string representation
void TString::Rep::createZeroRep() {
   zero = (Rep *)zero_mem;
   zero->len = 1;
   zero->mem = 1;
   zero->ref = 1; // never modify/delete static object
   zero->vulnerable = false;
   (*zero)[0] = '0';
   zero->terminate();
}
      
      
// non inline string implelentation

// returns true on success! returns value in bool_out!
bool TString::toBool(bool& bool_out) const {
   char buf[7];
   int i;
   for(i=0; i<6; i++) {
      buf[i] = tolower((*rep)[i]);
      if((buf[i]==0) || isspace(buf[i])) break; 
   }
   buf[i]=0;
   switch(i) {
    case 1:
      if((buf[0]=='1')||(buf[0]=='t')) bool_out = true;  return true;
      if((buf[0]=='0')||(buf[0]=='f')) bool_out = false; return true;
      break;
    case 2:
      if(strcmp(buf,"on")==0)          bool_out = true;  return true;
      if(strcmp(buf,"no")==0)          bool_out = false; return true;
      break;
    case 3:
      if(strcmp(buf,"yes")==0)         bool_out = true;  return true;
      if(strcmp(buf,"off")==0)         bool_out = false; return true;
      break;
    case 4:
      if(strcmp(buf,"true")==0)        bool_out = true;  return true;
      break;
    case 5:
      if(strcmp(buf,"false")==0)       bool_out = false; return true;
      break;
   }   
   return false;
}


// returns true on success
bool TString::toLong(long& long_out, int base) const {
   char *p;
   long r = strtol(rep->data(), &p, base);
   if(p == rep->data()) return false;
   if(*p) if(!isspace(*p)) return false;
   long_out = r;
   return true;
}


// returns true on success
bool TString::toInt(int& int_out, int base) const {
   char *p;
   int r = strtol(rep->data(), &p, base);
   if(p == rep->data()) return false;
   if(*p) if(!isspace(*p)) return false;
   int_out = r;
   return true;
}


// returns true on success
bool TString::toDouble(double& double_out) const {
   char *p;
   double r = strtod(rep->data(), &p);
   if(p == rep->data()) return false;
   if(*p) if(!isspace(*p)) return false;
   double_out = r;
   return true;
}


TString TString::shortFilename(int maxchar) const {
   if(rep->len <= maxchar) return *this;
   if(maxchar < 3) return "";
   return "..." + operator()(rep->len-maxchar+3, END);
}


TString TString::scanToken(int& scanner, int flags, 
		       const char *allow, const char *forbid,
		       bool allow_quoted) const 
{
   if(allow_quoted && (uint(scanner)<uint(rep->len))) {
      char q = (*rep)[scanner];
      if((q=='\'')||(q=='\"')) {
	 int st(++scanner);
	 while((uint(scanner)<uint(rep->len)) && ((*rep)[scanner]!=q)) 
	   ++scanner;
	 TString out = operator()(st, scanner);	 
	 if(uint(scanner)<uint(rep->len)) ++scanner;
	 return out;
      }
   }
   int start(scanner);
   for(; (uint(scanner)<uint(rep->len)); ++scanner) {
      char c = (*rep)[scanner];
      if(forbid && strchr(forbid, c)) break; 
      if((flags&ALL                )) continue;
      if(allow  && strchr(allow , c)) continue; 
      if((flags&ALPHA) && isalpha(c)) continue;
      if((flags&DIGIT) && isdigit(c)) continue;
      if((flags&LOWER) && islower(c)) continue;
      if((flags&UPPER) && isupper(c)) continue;
      if((flags&PRINT) && isprint(c)) continue;
      if((flags&GRAPH) && isgraph(c)) continue;
      if((flags&CNTRL) && iscntrl(c)) continue;
      if((flags&SPACE) && isspace(c)) continue;
      if((flags&XDIGIT)&&isxdigit(c)) continue;
      if((flags&PUNCT) && ispunct(c)) continue;
   }
   return operator()(start, scanner);
}


void TString::normalizePath() {
   TArray<TString> a = split(*this, "/", false, false);
   int i;

   // delete nul dirs (/./ and //)
   for(i=0; i < a.num(); ++i) {
      if((a[i].rep->len==0) || (a[i]==".")) {
	 a.slowRemove(i--);
      }
   }
   
   // check for absolute
   if((*rep)[0]=='/') empty();
   else operator=(".");

   // delete '..'
   for(i=0; i < a.num(); ++i) {
      if((a[i]=="..") && (i>=1) && (a[i-1]!="..")) {
	 a.slowRemove(--i);
	 a.slowRemove(i--);
      } 
   }
      
   // assemble string
   if(a.num()>0 || rep->len==0)
     operator += ("/" + join(a, "/"));
}


bool TString::isLower() const {
   if(rep->len==0) return false;
   for(int i=0; i<rep->len; i++) 
     if(isalpha((*rep)[i])) 
       if(isupper((*rep)[i])) 
	 return false;
   return true;
}


bool TString::isUpper() const {
   if(rep->len==0) return false;
   for(int i=0; i<rep->len; i++) 
     if(isalpha((*rep)[i])) 
       if(islower((*rep)[i])) 
	 return false;
   return true;
}


bool TString::isCapitalized() const {
   if(rep->len==0) return false;
   if(isalpha((*rep)[0])) if(islower((*rep)[0])) return false;
   for(int i=1; i<rep->len; i++) 
     if(isalpha((*rep)[i])) 
       if(isupper((*rep)[i])) 
	 return false;
   return true;   
}


void TString::lower() {
   detach();
   for(int i=0; i<rep->len; i++) (*rep)[i] = tolower((*rep)[i]);
}


void TString::upper() {
   detach();
   for(int i=0; i<rep->len; i++) (*rep)[i] = toupper((*rep)[i]);
}


void TString::capitalize() {
   lower();
   if(rep->len) (*rep)[0] = toupper((*rep)[0]);
}


static const char *bytesearch(const char *mem, int mlen, 
			      const char *pat, int plen,
			      bool ignore_case, bool whole_words) {
   int i,j;   
   for(i=0; i <= mlen-plen; i++) {
      if(ignore_case) {
	 for(j=0; j<plen; j++) 
	   if(tolower(mem[i+j]) != tolower(pat[j])) break;
      } else {
	 for(j=0; j<plen; j++) 
	   if(mem[i+j] != pat[j]) break;
      }
      if(j==plen) { // found
	 if(!whole_words) return mem + i;
	 else {
	    bool left_ok = true;
	    bool right_ok = true;
	    if(i > 0) if(isalnum(mem[i-1]) || (mem[i-1]=='_')) 
	      left_ok = false;
	    if(i < mlen-plen) if(isalnum(mem[i+plen]) || (mem[i+plen]=='_')) 
	      right_ok = false;
	    if(left_ok && right_ok) return mem + i;
	 }
      }
   }
   return 0; // not found
}


void TString::extractFilename() {
   char *p = strrchr(rep->data(), '/');
   if(p) operator=(p+1);
}


void TString::extractPath() {
   char *p = strrchr(rep->data(), '/');
   if(p) truncate((p - rep->data()) + 1);
   else empty();
}


void TString::removeDirSlash() {
   while((lastChar()=='/') && (rep->len > 1)) truncate(rep->len-1);
   
}


void TString::addDirSlash() {
   if(lastChar()!='/') operator += ("/");
}


void TString::extractFilenameExtension() {
   extractFilename();  // get file name
   char *p = strrchr(rep->data(), '.');
   if(p) {  // contains period
      if(p > rep->data()) { // last period not first char
	 operator=(p+1);    // get extension
	 return;
      }
   }
   empty(); // no extension
}


double TString::binaryPercentage() const {
   double bin=0;
   
   for(int i=0; i<rep->len; i++) 
     if((!isprint((*rep)[i])) && (!isspace((*rep)[i]))) bin+=1.0;
   return (bin*100.0)/double(rep->len);
}


int TString::searchReplace(const TString& tsearch, const TString& replace, 
			  bool ignore_case, bool whole_words,
			  bool preserve_case, int progress,
			  const TString& pre_padstring, const TString& post_padstring, TArray<int> *match_pos) {
   // get new length and positions
   if(progress) {putc('S', stderr);fflush(stderr);}
   int num = search(tsearch, ignore_case, whole_words, progress);
   if(progress) {putc('R', stderr);fflush(stderr);}   
   if(num==0) {
      return 0;
   }
   int newlen = rep->len + num*(replace.rep->len-tsearch.rep->len + 
				pre_padstring.len()+post_padstring.len());

   // create new string 
   Rep *newrep = Rep::create(newlen);   
   const char *p = rep->data();  // read
   char *q =    newrep->data();  // write
   const char *r;                // found substring
   int mlen = rep->len;          // rest of read mem
   for(int i=0; i < num; i++) {
      if(progress>0) if((i%progress)==0) {putc('.', stderr);fflush(stderr);}
      r = bytesearch(p, mlen, tsearch, tsearch.rep->len, ignore_case, whole_words);
      memcpy(q, p, r-p); // add skipped part
      q += r-p;      
      if(match_pos) (*match_pos) += int(q-newrep->data()); // enter start
      memcpy(q, pre_padstring.rep->data(), pre_padstring.rep->len); // add pre pad
      q += pre_padstring.len();
      if(!preserve_case) { // add replaced part
	 memcpy(q, replace.rep->data(), replace.rep->len);
      } else {
	 TString rep(preserveCase(TString(r, tsearch.rep->len), replace.rep->data()));
	 memcpy(q, rep.rep->data(), rep.rep->len);
      }
      q += replace.rep->len;      
      memcpy(q, post_padstring.rep->data(), post_padstring.rep->len); // add post pad
      q += post_padstring.len();
      if(match_pos) (*match_pos) += int(q-newrep->data()); // enter end
      mlen -= r-p;
      mlen -= tsearch.rep->len;
      p = r + tsearch.rep->len;
   }
   memcpy(q, p, mlen); // add rest
   replaceRep(newrep);
   rep->len = newlen;
   rep->terminate();
   return num;
}


int TString::search(const TString& pat, bool ignore_case, bool whole_words, int progress, TArray<int> *match_pos) const {
   if(!pat) throw StringIsEmpty();
   int num=0;
   int mlen=rep->len;
   const char *q;		      		      
   for(const char *p=rep->data(); (q=bytesearch(p, mlen, pat, pat.rep->len, 
					ignore_case, whole_words)); num++) {
      if(match_pos) (*match_pos) += int(q-rep->data());
      mlen -= q-p;
      mlen -= pat.rep->len;
      p = q + pat.rep->len;
      if(match_pos) (*match_pos) += int(p-rep->data());
      if(progress>0) if((num%progress)==0) {putc('.', stderr);fflush(stderr);}
   }
   return num;
}


bool TString::hasPrefix(const TString& pref) const {
   if(pref.rep->len > rep->len) return false;
   return memcmp(rep->data(), pref.rep->data(), pref.rep->len)==0;
}


bool TString::hasSuffix(const TString& suf) const {
   if(suf.rep->len > rep->len) return false;
   return memcmp(rep->data() + (rep->len - suf.rep->len), 
		 suf.rep->data(), suf.rep->len)==0;
}


bool TString::consistsOfSpace() const {
   for(int i=0; i<rep->len; i++) {
      if(!isspace((*rep)[i])) return false;
   }
   return true;
}


void TString::truncate(int max) {
   if((unsigned int)max < (unsigned int)rep->len) {
      detach();
      rep->len = max;
      rep->terminate();
   }
}


TString TString::getFitWordsBlock(int max) {
   TString r = getFitWords(max);
   int spaces;
   int fill = max - r.len();
   if(fill > 8) return r;
   int i,j;
      
   for(i=0; i < r.len(); i++)
     if(r[i] != ' ') break;
   for(spaces=0; i < r.len(); i++)
     if(r[i] == ' ') spaces++;
   if(fill > spaces) return r;
   TString t;
   t.detachResize(max);
   for(i=0, j=0; i < r.len(); i++) {
      if(r[i] != ' ') break;
      (*(t.rep))[j++] = r[i];
   }
   for(; i < r.len(); i++) {
      if((fill > 0)&&(r[i] == ' ')) {
	 (*(t.rep))[j++] = ' ';
	 (*(t.rep))[j++] = ' ';
	 fill--;
      } else (*(t.rep))[j++] = r[i];
   }
   t.rep->len = j;
   t.rep->terminate();
   return t;
}


void TString::cropSpaceEnd() {
   int e = rep->len;
   
   if(e == 0) return;
   else e--;
   while((e >= 0) && isspace((*rep)[e])) e--;
   truncate(e+1);               
}


TString TString::getFitWords(int max) {
   if(max < 1) 
     throw InvalidWidth(max);

   TString r(*this); // return value
   
   // check for lf
   int lf = firstOccurence('\n');
   if((lf!=-1) && (lf<=max)) {
      operator=(operator()(lf+1, END));
      r.truncate(lf);
      r.cropSpaceEnd();
      return r;
   }
   
   // string fits
   if(rep->len <= max) {
      empty();
      r.cropSpaceEnd();
      return r;
   }
   
   // find space
   int last_space = -1;
   int i;
   for(i=0; i <= max; i++) {
      if((*rep)[i] == ' ') last_space = i;
   }
   if(last_space==-1) last_space = max;
   
   // return 
   r.truncate(last_space);
   while(isspace((*rep)[last_space])) last_space++;
   operator=(operator()(last_space, END));
   r.cropSpaceEnd();
   return r;
}


void TString::unquote(bool allow_bslash, bool crop_space) {
   detach();
   
   char *p=rep->data();
   char *q=rep->data();
   char quote=0;
   char *nonspace=rep->data();
   
   if(crop_space) while(isspace(*p)) p++;
   for(; *p; p++) {
      if(allow_bslash && *p=='\\') {
	 if(p[1] == quote) {
	    p++;
	    if(*p == 0) break;
	 }
      } else {
	 if(quote) {
	    if(*p == quote) {
	       quote = 0;
	       continue;
	    }
	 } else {
	    if((*p == '\'') || (*p == '\"')) {
	       quote = *p;
	       continue;
	    }
	 }	 
      }
      if(quote || (!isspace(*p))) nonspace = q;
      *(q++) = *p;
   }   
   *q = 0;
   if(crop_space) if(*nonspace) nonspace[1] = 0;
   rep->len = strlen(rep->data());   
}


bool TString::readLine(FILE *file) {
   char buf[1024];
   
   empty();
   while(1) {	 
      buf[sizeof(buf)-2] = '\n';
      if(!fgets(buf, sizeof(buf), file)) break;
      operator+=(buf);
      if(buf[sizeof(buf)-2] == '\n') break;
   }
   if(rep->len) return true;
   else    return false;
}


int TString::write(FILE *file) const {
   return fwrite(rep->data(), 1, rep->len, file);   
}


int TString::read(FILE *file, int l) {
   if(l<0) throw InvalidWidth(l);
   rep->release();
   rep = Rep::create(l);
   int r = fread(rep->data(), 1, l, file);
   rep->len = r;
   rep->terminate();
   return r;
}


int TString::readFile(const char *filename) {
   struct stat buf;

   if(stat(filename, &buf)) return -1; // does not exist
   FILE *f=fopen(filename, "rb");
   if(f==0) return -2;                 // no permission?
   int r = read(f, buf.st_size);
   fclose(f);
   if(r != buf.st_size) return -3;     // read error
   return 0;
}

   
void TString::expandUnprintable() {
   Rep *newrep = Rep::create(rep->len*4);
   char *q = newrep->data(); // write
   char *p = rep->data();    // read
   int l=0;
   
   // expand each char
   for(int j=0; j < rep->len; ++j, ++p) {
      if(isprint(*p)) { // printable --> print
	 if(*p=='\\') { // backslashify backslash
	    *(q++) = '\\';	 
	    l++;	    
	 } 
	 *(q++) = *p;
	 l++;
      } else { // unprintable --> expand
	 *(q++) = '\\';	// leading backslash
	 l++;
	 switch(*p) {
#if 0
	  case '\a':
	    *(q++) = 'a';
	    l++;
	    break;
#endif
	  case '\b':
	    *(q++) = 'b';
	    l++;
	    break;
	  case '\f':
	    *(q++) = 'f';
	    l++;
	    break;
	  case '\n':
	    *(q++) = 'n';
	    l++;
	    break;
	  case '\r':
	    *(q++) = 'r';
	    l++;
	    break;
	  case '\t':
	    *(q++) = 't';
	    l++;
	    break;
	  case '\v':
	    *(q++) = 'v';
	    l++;
	    break;
	  default: // no single char control
	    uint i = (unsigned char)*p;
	    l+=3;
	    if(i<32) {  // print lower control octal
	       if(isdigit(p[1])) {
		  q += ::sprintf(q, "%03o", i);
	       } else {
		  q += ::sprintf(q, "%o", i);
		  if(i>=8) --l;
		  else l-=2;
	       }
	    } else {    // print octal or hex
	       if(isxdigit(p[1])) {
		  q += ::sprintf(q, "%03o", i);
	       } else {
		  q += ::sprintf(q, "x%02x", i);
	       }
	    }
	 }
      }
   }
   
   // end
   replaceRep(newrep);
   rep->len = l;
   rep->terminate();
}


void TString::backslashify() {
   Rep *newrep = Rep::create(rep->len*2);
   char *p = rep->data();
   char *q = newrep->data();
   int l=0;
   
   // backslashify each char
   for(int i=0; i<rep->len; i++, p++) {
      switch(*p) {
       case '\\':
	 *(q++) = '\\';
	 *(q++) = '\\';
	 l+=2;
	 break;
       case '\'':
	 *(q++) = '\\';
	 *(q++) = '\'';
	 l+=2;
	 break;
       case '\"':
	 *(q++) = '\\';
	 *(q++) = '\"';
	 l+=2;
	 break;
       default:
	 *(q++) = *p;
	 l++;
	 break;
      }
   }
   
   // end
   replaceRep(newrep);
   rep->len = l;
   rep->terminate();
}


void TString::compileCString() {
   detach();

   char *p = rep->data(); // read
   char *q = rep->data(); // write
   char c;                // tmp char
   int l=0;               // write
   int i=0;               // read
   
   while(i < rep->len) {
      c = *(p++); // read char
      i++;
      if(c == '\\') { // compile char
	 if(i>=rep->len) break;
	 c = *(p++);
	 i++;
	 switch(c) {
#if 0
	  case 'a':
	    c = '\a';
	    break;
#endif
	  case 'b':
	    c = '\b';
	    break;
	  case 'f':
	    c = '\f';
	    break;
	  case 'n':
	    c = '\n';
	    break;
	  case 'r':
	    c = '\r';
	    break;
	  case 't':
	    c = '\t';
	    break;
	  case 'v':
	    c = '\v';
	    break;
	  case 'x': // hex
	    char *q;
	    c = strtol(p, &q, 16);
	    i += q-p;
	    p = q;
	    break;	    
	  case '0': // octal
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	    char buf[4];
	    buf[0] = c;
	    buf[1] = *p;
	    buf[2] = (i < rep->len) ? p[1] : 0;
	    buf[3] = 0;
	    char *t;
	    c = strtol(buf, &t, 8);
	    i += (t-buf)-1;
	    p += (t-buf)-1;
	    break;	    
	 }	 
      } 
      *(q++) = c; // write char
      l++;
   }
   rep->len = l;
   rep->terminate();
}


void TString::removeHTMLTags(int& level) {
   detach();

   char *p = rep->data(); // read
   char *q = rep->data(); // write
   int l=0;               // write
   int i=0;               // read
   
   while(i < rep->len) {
      switch(*p) {
       case '<': 
	 level++;
	 break;

       case '>':
	 if(level > 0) level--;
	 break;
	 
       default:
	 if(level == 0) {
	    *(q++) = *p;
	    l++;
	 }
      }      
      p++;
      i++;
   }
   
   rep->len = l;
   rep->terminate();
}


void TString::cropSpace(void) {
   int first = rep->len;
   int last = 0;
   int i;
   
   // get first nonspace
   for(i=0; i < rep->len; ++i) 
     if(!isspace((*rep)[i])) {
	first = i;
	break;
     }
   
   // full of spaces   
   if(first == rep->len) {
      empty();
      return;
   }
   
   // get last nonspace
   for(i = rep->len - 1; i >= first; --i) 
     if(!isspace((*rep)[i])) {
	last = i;
	break;
     }
   ++last;
   
   // truncate
   if(first == 0) {
      truncate(last);
      return;
   }
     
   // extract substring
   operator=(operator()(first, last));   
}


void TString::collapseSpace(void) {
   detach();
   
   char *p = rep->data(); // read
   char *q = rep->data(); // write
   char last_char = ' ';
   int l=0;               // length
   char c;
   
   for(int i=0; i < rep->len; ++i, ++p) {
      if((!isspace(*p)) || (!isspace(last_char))) {
	 c = *p;
	 if(isspace(c)) c=' ';
	 *(q++) = c;
	 last_char = c;
	 l++;
      }
   }
   if(isspace(last_char)&&(l>0)) --l;
   rep->len = l;
   rep->terminate();
}


void TString::translateChar(char from, char to) {
   detach();   
   char *p = rep->data();   
   for(int i=0; i < rep->len; ++i, ++p)
     if(*p == from) *p = to;
}


int TString::firstOccurence(char c) const {
   int i;
   
   for(i=0; (i < rep->len) && ((*rep)[i] != c); ++i);
   if(i < rep->len) return i;
   else return -1;
}



// non member  implementation


TArray<TString> split(const TString &s, const char *sep, bool allow_quoting,
		     bool crop_space) {
   TArray<TString> r;
   int i=0;
   TArray<char> buf;
   const char *p = s;
   p--; // bias
   
   do {
      // next chunk
      p++;	  
      
      // collect chars to buf
      while(*p) {
	 if(strchr(sep, *p)) {
	    break;
	 } else	if(!allow_quoting) {
	    buf += *(p++);	    
	 } else if(*p=='\\') {
	    p++;
	    if(strchr(sep, *p)==0) buf += '\\';
	    if(*p) buf += *(p++);
	 } else if(*p=='\'') {
	    buf += '\'';
	    for(p++; *p && *p!='\''; p++) {
	       if(*p=='\\') {
		  p++;
		  buf += '\\';
		  if(*p) buf += *p;
	       } else 
		 buf += *p;
	    }
	    buf += '\'';
	    if(*p=='\'') p++;
	 } else if(*p=='\"') {
	    buf += '\"';
	    for(p++; *p && *p!='\"'; p++) {
	       if(*p=='\\') {
		  p++;
		  buf += '\\';
		  if(*p) buf += *p;
	       } else 
		 buf += *p;
	    }
	    buf += '\"';
	    if(*p=='\"') p++;
	 } else {
	    buf += *(p++);
	 }
      }
      
      // put buf to r
      buf+='\0';
      r[i] = buf.data();
      if(crop_space) r[i].cropSpace();
      i++;
      
      // cleanup
      buf.empty();
   } while(*p);
   
   r.fixedSize();
   return r;
}


TString join(const TArray<TString>& a, const TString& sep) {
   TString r;
   
   if(a.isEmpty()) return r;
   else r = a[0];   
   for(int i = 1; i < a.num(); i++) {
      r += sep;
      r += a[i]; 
   }
   return r;
}


TString preserveCase(const TString& from, const TString& to) {
   TString r(to);
   
   if(from.len() == to.len()) { 
      // same len
      for(int i=0; i < r.len(); i++) {
	 if(islower(from[i])) r[i] = tolower(r[i]);
	 else if(isupper(from[i])) r[i] = toupper(r[i]);
      }
   } else {   
      // some heuristics
      if(from.isLower()) r.lower();
      if(from.isUpper()) r.upper();
      if(from.isCapitalized()) r.capitalize();
   }
   
   return r;
}


TArray<TString> loadTextFile(const char *fname) {
   FILE *f = fopen(fname, "r");
   if(f==0) throw TFileOperationErrnoException(fname, "fopen(mode='r')", errno);
   TArray<TString> r;
   for(int i=0; r[i].readLine(f); i++);
   fclose(f);
   r.killLastElement();
   r.fixedSize();
   return r;
}


TArray<TString> loadTextFile(FILE *file) {
   TArray<TString> r;
   for(int i=0; r[i].readLine(file); i++);
   r.killLastElement();
   r.fixedSize();
   return r;
}






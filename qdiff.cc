/*GPL*START*
 * qdiff - binary diff and more
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

#include "tappconfig.h"
#include "trotfile.h"
#include "tdiffoutput.h"
#include "tminmax.h"
#include <stdio.h>

// *** qdiff ***

// Jun 05 1997: V0.7:
//   bugfix: missing final flush added
//   added: color support for IRIX
// Aug 02: fast and fixed eof handling
// Aug 30: large files optimization
// Aug 30: progress during match/sync
// 1998:
// Jan 31 : v0.8b alt-color added
// Oct 01 21:07: v0.8.1 prepare for sunsite

// 1999:
// Feb 16 20:03: v0.8.2 width defaults to terminal width feature added


const char *option_list[] ={
   "#usage='Usage: %n [OPTION]... FILE1 FILE2\n'",
   "#trailer='\n%n version %v\n *** (C) 1997-1999 by Johannes Overmann\n *** (C) 2008 by Tong Sun\ncomments, bugs and suggestions welcome: %e\n%gpl'",
   "#onlycl", // only command line options
   "name=byte-by-byte,      type=switch, char=b,                                     help=\"compare files byte by byte, like 'cmp'\", headline=diff options:",
   "name=no-heuristics,     type=switch, char=f,                                     help='do not use heuristics to speed up large differing blocks, note that the result is always correct but with this option you may find a smaller number of differing bytes'",
   "name=min-match,         type=int,    char=m, param=NUM,     default=20, lower=1, help='allow resynchronisation only after a minimum of NUM bytes match, this is an important parameter: lower values may result in a more detailed analysis or in useless results, higher values give a coarse analysis but resynchronisation is more robust'",
   "name=large-files,       type=switch, char=O,                                     help=optimize disk access for large files on the same disk (locks 16MB mem)",
   "name=formatted,         type=switch, char=a,                                     help='print formatted ascii text, line by line', headline='output modes:  (override automatic file type determination)'",
   "name=unformatted,       type=switch, char=u,                                     help='print unformatted ascii text, block by block'",
   "name=hex,               type=switch, char=x,                                     help='print hex dump, block by block'",
   "name=vertical,          type=switch, char=t,                                     help=print one byte per line (ignores width)",
   "name=no-color,          type=switch, char=c,                                     help=disable ansi coloring of output, headline=output options:",
   "name=alt-colors,        type=switch, char=C,                                     help='no bold ansi coloring (for SGI terminals and the like)'",
   "name=width,             type=int,    char=w, param=NUM,     default=0,           help=output maximal NUM chars (default is terminal width)",
   "name=bytes-per-line,    type=int,    char=B, param=NUM,                          help=print NUM bytes/chars per line",
   "name=tab-size,          type=int,    char=T, param=TABSIZE, default=8,  lower=1, help=TABSIZE in formatted mode",
   "name=line-numbers,      type=switch, char=l,                                     help=print line numbers in formatted mode",
   "name=no-line-break,     type=switch, char=n,                                     help=truncate (not break) lines in formatted mode",
   "name=show-lf-and-tab,   type=switch, char=L,                                     help=show newline/tab as <LF>/<HT> in formatted mode",
   "name=show-space,        type=switch, char=S,                                     help=show space as <SPC> in non hex modes",
   "name=unprintable,       type=string, char=U, param=CHAR,                         help=print CHAR for unprintable chars in non hex modes",
   "name=control-hex,       type=switch, char=H,                                     help=print control codes in hex (<x1B>\\, not <ESC>)",
   "name=alignment-marks,   type=switch, char=A,                                     help=print '-/+' before 32/64-bit words in hex mode",
   "name=stop-on-eof,       type=switch, char=e,                                     help=stop when end of a file is reached in vertical mode",
   "name=hide-match,        type=switch,                                             help=do not print matches",
   "name=hide-deletion,     type=switch,                                             help=do not print deletions",
   "name=hide-insertion,    type=switch,                                             help=do not print insertions",
   "name=hide-substitution, type=switch,                                             help=do not print substitutions",
   "name=range-match,       type=switch,                                             help=print match as byte range",
   "name=range-deletion,    type=switch,                                             help=print deletion as byte range",
   "name=range-insertion,   type=switch,                                             help=print insertion as byte range",
   "name=range-substitution,type=switch,                                             help=print substitution as two byte ranges",
   "name=range,             type=switch, char=R,                                     help=print everything as byte range",     
   "name=verbose,           type=switch, char=v,                                     help=verbose execution, headline='common options:'",
   "name=progress,          type=switch, char=P, help=show progress during work",
   "EOL"
};


bool prog = false;


int match(TROTFile& f1, int o1, TROTFile& f2, int o2) {
   int i;
   int s1 = f1.size();
   int s2 = f2.size();
   int i1 = o1;
   int i2 = o2;
   int print = 256*1024;
   int pri;

   if(!prog) print = -1;
   for(i=0, pri=print; (i1<s1) && (i2<s2) && (f1[i1] == f2[i2]); i++, i1++, i2++, pri--)
     if(pri == 0) {
	pri = print;
	fprintf(stderr, "mat(%5dK,%5dK)  \r", i1>>10, i2>>10);
	fflush(stderr);
     }
   return i;
}


int syncronizeOnlySubst(TROTFile& f1, int o1, TROTFile& f2, int o2, 
			int minmatch) {
   int s1 = f1.size();
   int s2 = f2.size();
   int i1 = o1;
   int i2 = o2;
   int mis = -1;
   int i;
   int print = 256*1024;
   int pri;

   if(!prog) print = -1;
   for(i=0, pri=print; (i1<s1) && (i2<s2) && ((i-mis) <= minmatch); i++, i1++, i2++, pri--) {
      if(f1[i1] != f2[i2]) mis = i;
      if(pri == 0) {
	 pri = print;
	 fprintf(stderr, "syn(%5dK,%5dK)  \r", i1>>10, i2>>10);
	 fflush(stderr);
      }
   }
   if((i - mis) > minmatch) return mis + 1; // match of len minmatch
   else return i; // eof
}


// return true if minmatch bytes match at o1/o2 in f1/f2
static inline bool compare(TROTFile& f1, int o1, TROTFile& f2, int o2, 
			   int minmatch) {
   int i1=o1;
   int i2=o2;
   
   if((f1.size()-i1) < minmatch) return false;
   if((f2.size()-i2) < minmatch) return false;
   for(int i=0; i<minmatch; i++, i1++, i2++)
     if(f1[i1] != f2[i2]) return false;
   return true;
}


void syncronize(TROTFile& f1, int o1, TROTFile& f2, int o2, int minmatch,
		bool heurist, int& out_sub, int& out_ins, int& out_del) {
   out_ins = 0;
   out_del = 0;
   out_sub = 0;
   
   // check for eof:
   if(o1==f1.size()) {
      out_ins = f2.size()-o2;
      return;
   }
   if(o2==f2.size()) {
      out_del = f1.size()-o1;
      return;
   }
   
   // simple diff engine: search for sync
   int max_i = tMax(f1.size()-o1, f2.size()-o2) - minmatch;
   int print = 20;
   if(!prog) print=-1;
   for(int i=0; i <= max_i; i++, print--) {
      for(int j=0; j <= i; j++) {
	 if(compare(f1, o1+i, f2, o2+j, minmatch)) {
	    if(heurist) {
	       while((i>0) && (j>0) && (f1[o1+i-1]==f2[o2+j-1])) {
		  i--;
		  j--;
	       }
	    }
	    out_sub = j;
	    out_del = i-j;
	    return;
	 }
	 if(compare(f1, o1+j, f2, o2+i, minmatch)) {
	    if(heurist) {
	       while((i>0) && (j>0) && (f1[o1+j-1]==f2[o2+i-1])) {
		  i--;
		  j--;
	       }
	    }
	    out_sub = j;
	    out_ins = i-j;
	    return;
	 }
      }
      if(heurist) i += i/10;
      if(print==0) {
	 print=heurist?10:100;
	 fprintf(stderr, "syncing byte range%8d (%s)\r", i, heurist?"heuristic":"exhaustive");
      }
   }
   
   // no sync found: 
   out_sub = tMin(f1.size()-o1, f2.size()-o2);
   out_del = f1.size() - o1 - out_sub;
   out_ins = f2.size() - o2 - out_sub;
}







// main
int main(int argc, char *argv[]) {   
   // init command line options
   TAppConfig ac(option_list, "option_list", argc, argv, 0, 0, VERSION);
   if(ac.numParam()!=2) {
      userError("need two files to compare, try '--help' for more information.\n");
   } 
   prog = ac("progress");
   
   // 1MB
   int numbuf = 16;
   int bufsize = 64*1024;
   
   if(ac("large-files")) {
      // 16MB
      numbuf = 4;
      bufsize = 4*1024*1024;
   }
   
   // init files
   TROTFile f1(ac.param(0), numbuf, bufsize);
   TROTFile f2(ac.param(1), numbuf, bufsize);
   int s1=f1.size();
   int s2=f2.size();
   
   // files empty?
   if((s1==0) && (s2==0)) {
      printf("both files are empty, nothing to compare\n");
      return 0;
   }
   if(s1==0) {
      printf("file '%s' is empty, nothing to compare\n", f1.name());
      return 0;
   }
   if(s2==0) {
      printf("file '%s' is empty, nothing to compare\n", f2.name());
      return 0;
   }
   
   // init output
   TDiffOutput out(f1, f2, ac);
   
   // additional config
   bool bytebybyte = ac("byte-by-byte");
   bool stoponeof = ac("stop-on-eof");
   bool heurist = !ac("no-heuristics");
   int minmatch = ac.getInt("min-match");
   
   // do diff
   int o1=0;
   int o2=0;
   int i;
   int ins, del, sub;
   while((s1!=o1)&&(s2!=o2)) {
      if(bytebybyte) {
	 i = syncronizeOnlySubst(f1, o1, f2, o2, minmatch);
	 if(i) out.sub(i);
	 o1 += i;
	 o2 += i;
      } else {
	 syncronize(f1, o1, f2, o2, minmatch, heurist, sub, ins, del);
	 if(sub) out.sub(sub, ins, del);
	 else {
	    if(del) out.del(del);
	    if(ins) out.ins(ins);
	 }
	 o1 += sub + del;
	 o2 += sub + ins;
      }
      i = match(f1, o1, f2, o2);
      if(i) out.mat(i);
      o1 += i;
      o2 += i;
   }
   if((o1 != s1) && (o2 != s2))
     fatalError("internal error: (o1!=s1) && (o2!=s2)\n");
   if(o1 != s1) {
      if(stoponeof) {
	 out.flush();
	 printf("eof in file '%s', %d uncompared bytes follow in file '%s'\n",
		f2.name(), s1-o1, f1.name());
      } else out.del(s1-o1);
   }
   if(o2 != s2) {
      if(stoponeof) {
	 out.flush();
	 printf("eof in file '%s', %d uncompared bytes follow in file '%s'\n",
		f1.name(), s2-o2, f2.name());
      } else out.ins(s2-o2);
   }
   out.flush();
   
   // end
   return 0;
}



/*GPL*START*
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

#ifndef _tdiffoutput_h_
#define _tdiffoutput_h_

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "terror.h"
#include "trotfile.h"
#include "tappconfig.h"

class TDiffOutput {
 public:
   // ctor & dtor
   TDiffOutput(TROTFile& f1, TROTFile& f2, const TAppConfig& ac);
   ~TDiffOutput();
   
   // interface
   void ins(int i); // insertion 
   void del(int i); // deletion
   void sub(int i, int ins=0, int del=0); // substitution
   void mat(int i); // match
   
   void flush();    // flush buffers: assume no more output   
   
 private:  // private data
   TROTFile& f1;    // file data
   TROTFile& f2;
   int o1;          // current offset in file
   int o2;
   const TAppConfig& ac;  // for command line options
   enum MODE_T {VERTICAL, F_ASCII, U_ASCII, HEX} mode;
   enum DIFF_T {NIL, MAT, SUB, DEL, INS};
   bool verbose;
      
   // output options:
   bool hide_mat;
   bool hide_ins;
   bool hide_del;
   bool hide_sub;
   bool range_mat;
   bool range_ins;
   bool range_del;
   bool range_sub;
   bool no_color;
   int width;
   int bytes_per_line;
   int max_bytes_per_line;
   int half_line_len;
   char *linebuf1;
   char *linebuf2;
   char *linebuf1p;
   char *linebuf2p;
   int bytesin1;
   int bytesin2;
   DIFF_T lastcolor1;
   DIFF_T lastcolor2;
   bool needadr1;
   bool needadr2;
   int line1;
   int line2;
   int tab_size;
   bool alignment_marks;
   bool show_lf_and_tab;
   bool show_space;
   bool line_numbers;
   bool no_line_break;
   char unprint;
   bool control_hex;
   
   // private methods
   MODE_T autoMode();
   void setStrLen(char *str, int len) const;
   void printSplitLine(char *abuf1, char *abuf2) const;
   void putHexElem(int o1, uchar b1, int o2, uchar b2, DIFF_T diff);
   void putAscElem(int o1, uchar b1, int o2, uchar b2, DIFF_T diff, bool formatted);
   const char *colorStr(DIFF_T diff) const;
   int charLen(uchar c, int pos) const;
   void putSpace(char **p, int num) const;
   void putChar(char **p, uchar c, int pos) const;
   const char *printChar(uchar c, char *buf) const;
   
   // forbid copy
   TDiffOutput(const TDiffOutput&);   
   const TDiffOutput& operator= (const TDiffOutput&);
};

#endif



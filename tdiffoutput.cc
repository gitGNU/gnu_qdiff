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

#include <sys/ioctl.h>
#include "tdiffoutput.h"
#include "ctype.h"


static const char *color_nor = "\033[m";
static const char *color_ins = "\033[00;32m";
static const char *color_del = "\033[01;31m";
static const char *color_sub = "\033[01;33m";
static const char *color_mat = "\033[01;37m";
static const char *color_sep = "\033[00;34m";


TDiffOutput::~TDiffOutput() {
   delete[] linebuf1;
   delete[] linebuf2;
}

TDiffOutput::TDiffOutput(TROTFile& file1, TROTFile& file2, 
			 const TAppConfig& appconf):
f1(file1), f2(file2), o1(0), o2(0), ac(appconf), mode(VERTICAL), 
verbose(false),
hide_mat(false),
hide_ins(false),
hide_del(false),
hide_sub(false),
range_mat(false),
range_ins(false),
range_del(false),
range_sub(false),
no_color(false),
width(0),
bytes_per_line(0),
max_bytes_per_line(0),
half_line_len(0),
linebuf1(0),
linebuf2(0),
linebuf1p(0),
linebuf2p(0),
bytesin1(0),
bytesin2(0),
lastcolor1(NIL),
lastcolor2(NIL),
needadr1(false),
needadr2(false),
line1(0),
line2(0),
tab_size(0),
alignment_marks(false),
show_lf_and_tab(false),
show_space(false),
line_numbers(false),
no_line_break(false),
unprint(0),
control_hex(false)
{
   // adjust colors
   if(ac("alt-colors")) {
      color_nor = "\033[00m";
      color_ins = "\033[35m";
      color_del = "\033[31m";
      color_sub = "\033[33m";
      color_mat = "\033[37m";
      color_sep = "\033[34m";
   }
   
   // verbose?
   verbose = ac("verbose");

   // some format options
   alignment_marks = ac("alignment-marks");
   line_numbers = ac("line-numbers");
   no_line_break = ac("no-line-break");
   control_hex = ac("control-hex");
   TString up = ac.getString("unprintable");
   if(up.len()==0) unprint = 0;
   else if(up.len()==1) unprint = up[0];
   else 
     userError("parameter for --unprintable-char should be a single char! (was '%s')\n", *up);
   
   // format?
   width = ac.getInt("width");
   if(width == 0) {
      struct winsize win;
      if(ioctl(1, TIOCGWINSZ, &win) == 0) 
	width = win.ws_col;
      else 
	width = 80;
   }
   if(width < 42) width = 42;
   half_line_len = (width-1)/2;
   tab_size = ac.getInt("tab-size");
   show_lf_and_tab = ac("show-lf-and-tab");
   show_space = ac("show-space");
   
   // which mode?
   bool hex = ac("hex");
   bool f_ascii = ac("formatted");
   bool u_ascii = ac("unformatted");
   bool vertical = ac("vertical");
   int i = hex+f_ascii+u_ascii+vertical;
   switch(i) {
    case 0: // automatic format
      mode = autoMode();
      break;
    case 1:
      if(hex)     mode = HEX;
      if(f_ascii) mode = F_ASCII;
      if(u_ascii) mode = U_ASCII;
      break;
    default:
      userError("specify only one of {--hex, --formatted-ascii, --unformatted-ascii, --vertical}\n");
   }
   switch(mode) {
    case HEX:
      if(verbose) printf("printing hex dump, block by block (hex mode)\n");
      max_bytes_per_line = (((width-1)/2)-8)/3;   
      bytes_per_line = ac.getInt("bytes-per-line");
      if(bytes_per_line<=0) bytes_per_line = max_bytes_per_line;
      if(unprint) 
	userError("--unprintable-char not useful in hex mode!\n");
      if(show_space) 
	userError("--show-space not useful in hex mode!\n");
      if(control_hex) 
	userError("--control-hex not useful in hex mode!\n");
      break;
    case F_ASCII:
      if(line_numbers)
	max_bytes_per_line = half_line_len-9;
      else
	max_bytes_per_line = half_line_len;
      bytes_per_line = ac.getInt("bytes-per-line");
      if(bytes_per_line<=0) bytes_per_line = max_bytes_per_line;      
      if(verbose) printf("printing formatted ascii text, line by line (formatted-ascii mode)\n");
      break;
    case U_ASCII:
      max_bytes_per_line = half_line_len-9;   
      bytes_per_line = ac.getInt("bytes-per-line");
      if(bytes_per_line<=0) bytes_per_line = max_bytes_per_line;      
      if(verbose) printf("printing unformatted ascii text, block by block (unformatted-ascii mode)\n");
      break;
    case VERTICAL:
      if(verbose) printf("printing one byte per line (vertical mode)\n");
      break;
   }
   if(mode!=F_ASCII) {
      show_lf_and_tab = true;
      if(line_numbers) 
	userError("--line-numbers make only sense for formatted ascii mode!\n");
      if(no_line_break) 
	userError("--no-line-break makes only sense for formatted ascii mode!\n");
      line_numbers = true;
   }
   
   // disable color?
   if(ac("no-color")) { 
      color_sep=color_sub=color_mat=color_del=color_ins=color_nor="";
   }

   // hide:
   hide_mat = ac("hide-match");
   hide_ins = ac("hide-insertion");
   hide_del = ac("hide-deletion");
   hide_sub = ac("hide-substitution");
   if(hide_mat && hide_ins && hide_del && hide_sub) 
     userError("specify not all of {--hide-match, --hide-deletion, --hide-insertion, --hide-substitution}\n");
   
   // range:
   range_mat = ac("range-match");
   range_ins = ac("range-deletion");
   range_del = ac("range-insertion");
   range_sub = ac("range-substitution");
   if(ac("range")) {
      range_mat = range_sub = range_ins = range_del = true;
   }

   // alloc some mem:
   linebuf1 = new char[half_line_len*16];
   linebuf2 = new char[half_line_len*16];
   bytesin1 = bytesin2 = 0;
   lastcolor1 = lastcolor2 = NIL;
   linebuf1p = linebuf1;
   linebuf2p = linebuf2;
   needadr1 = needadr2 = true;
   line1 = line2 = 1;
}


const char *TDiffOutput::printChar(uchar c, char *buf) const {
   if((c==32)&& show_space) return "SPC";
   // printable codes
   if(isprint(c)) {
      buf[0] = '\'';
      buf[1] = c;
      buf[2] = '\'';
      buf[3] = 0;
      return buf;
   }      
   // control codes
   switch(c) {
    case  0: return "NUL";
    case  1: return "SOH";
    case  2: return "STX";
    case  3: return "ETX";
    case  4: return "EOT";
    case  5: return "ENQ";
    case  6: return "ACK";
    case  7: return "BEL";
    case  8: return "BS ";
    case  9: return "HT ";
    case 10: return "LF ";
    case 11: return "VT ";
    case 12: return "FF ";
    case 13: return "CR ";
    case 14: return "SO ";
    case 15: return "SI ";
    case 16: return "DLE";
    case 17: return "DC1";
    case 18: return "DC2";
    case 19: return "DC3";
    case 20: return "DC4";
    case 21: return "NAK";
    case 22: return "SYN";
    case 23: return "ETB";
    case 24: return "CAN";
    case 25: return "EM ";
    case 26: return "SUB";
    case 27: return "ESC";
    case 28: return "FS ";
    case 29: return "GS ";
    case 30: return "RS ";
    case 31: return "US ";
    case 127:return "DEL";
   }
   // all other codes
   return "   ";
}


void TDiffOutput::setStrLen(char *str, int len) const {
   char *p=str;
   int l=0;
   bool esc_seq = false;   
   while(*p) {
      if(l==len) break;
      if(*p==27) {
	 esc_seq = true;
	 while(*p!='m') p++;
	 p++;
      } else {
	 p++;
	 l++;
      }
   }  
   while(l<len) {
      *(p++)=' ';
      l++;
   }
   *p=0; // terminate string
   if(esc_seq) strcpy(p, color_nor); // switch to normal
}


void TDiffOutput::flush() {
   if(bytesin1 || bytesin2)
     printSplitLine(linebuf1, linebuf2);
   *linebuf1=0;
   *linebuf2=0;
   bytesin1 = bytesin2 = 0;
   lastcolor1 = lastcolor2 = NIL;
   linebuf1p = linebuf1;
   linebuf2p = linebuf2;
   needadr1 = needadr2 = true;
}


void TDiffOutput::putChar(char **p, uchar c, int pos) const {
   const char *hex="0123456789ABCDEF";
   if(c>=128) {           // 128-255
      if(unprint) {
	 *((*p)++)=unprint;
	 **p=0;
	 return;
      }
      *((*p)++)='<';
      *((*p)++)='x';
      *((*p)++)=hex[c>>4];
      *((*p)++)=hex[c&15];
      *((*p)++)='>';
      **p=0;
      return;
   }
   if((c>32)&&(c<=126)) { //  32-126
      *((*p)++)=c;
      **p=0;
      return;
   }
   if((c==' ')&&(!show_space)) {
      *((*p)++)=' ';
      **p=0;
      return;
   }
   if((c=='\n')&&(!show_lf_and_tab)) {
      **p=0;
      return;
   }
   if((c=='\t')&&(!show_lf_and_tab)) {
      do {
	 *((*p)++)=' '; pos++;
      } while(pos%tab_size);
      **p=0;      
      return;
   }

   if(unprint) {
      *((*p)++)=unprint;
      **p=0;
      return;
   }
      
   if(control_hex) {         
      *((*p)++)='<';
      *((*p)++)='x';
      *((*p)++)=hex[c>>4];
      *((*p)++)=hex[c&15];
      *((*p)++)='>';
      **p=0;
      return;
   }

   *((*p)++)='<';
   const char *q = printChar(c, NULL);
   *((*p)++)=q[0];
   *((*p)++)=q[1];
   if(charLen(c, 0)==5)   
     *((*p)++)=q[2];
   *((*p)++)='>';
   **p=0;
}


void TDiffOutput::putSpace(char **p, int num) const {
   while(num--) *((*p)++)=' ';
   **p=0;
}


int TDiffOutput::charLen(uchar c, int pos) const {
   int four=4;
   if(control_hex) four = 5;  //   :)
   if(unprint) {
      if(c==9) {
	 if(show_lf_and_tab) return 1;
	 else return tab_size-(pos%tab_size);
      }
      return 1;
   }
   switch(c) {       // 0-31
    case  0:
    case  1:
    case  2:
    case  3:
    case  4:
    case  5:
    case  6:
    case  7:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 26:
    case 27:
      return 5;
    case  8:
//    case  9:
//    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 25:
    case 28:
    case 29:
    case 30:
    case 31:
      return four;
    case 9:
      if(show_lf_and_tab) return four;
      else return tab_size-(pos%tab_size);
    case 10:
      if(show_lf_and_tab) return four;
      else return 1;
    case 32:
      if(show_space) return 5;
      else return 1;
    default:
      if(c>=127) return 5; // 127-255
      if(c>32) return 1;  //  33-126
   }
   fatalError("internal error: should never get here!\n");
}


void TDiffOutput::putAscElem(int off1, uchar b1, int off2, uchar b2, DIFF_T diff,
			     bool formatted) {
   if((bytesin1+charLen(off1>=0?b1:b2,off1>=0?bytesin1:bytesin2)>bytes_per_line)||
      (bytesin2+charLen(off2>=0?b2:b1,off2>=0?bytesin2:bytesin1)>bytes_per_line)) {
      if(!no_line_break) flush();
   }
   if((bytesin1 > max_bytes_per_line)||(bytesin2 > max_bytes_per_line)&&0) {
      bytesin1+=charLen(b1, bytesin1);
      bytesin2+=charLen(b2, bytesin2);
   } else {
      if(off1 >= 0) {
	 if(bytesin1==0 && line_numbers) {
	    if(!formatted) linebuf1p += sprintf(linebuf1, "%08X:", off1);
	    else           linebuf1p += sprintf(linebuf1, "% 8d:", line1);
	    needadr1 = false;
	 }
	 if(needadr1 && line_numbers) {
	    char t = linebuf1[9];
	    if(!formatted) sprintf(linebuf1, "%08X:", off1);
	    else           sprintf(linebuf1, "% 8d:", line1);
	    linebuf1[9] = t;
	    needadr1 = false;
	 }
	 if(lastcolor1 != diff) {
	    linebuf1p += sprintf(linebuf1p, colorStr(diff));
	    lastcolor1 = diff;
	 }
	 putChar(&linebuf1p, b1, bytesin1); 
	 bytesin1+=charLen(b1, bytesin1);
	 if((off2>=0)&&(charLen(b2, bytesin2)>charLen(b1, bytesin1))) {
	    putSpace(&linebuf1p, charLen(b2, bytesin2)-charLen(b1, bytesin1));
	    bytesin1+=charLen(b2, bytesin2)-charLen(b1, bytesin1);
	 }
      } else {
	 if(bytesin1==0  && line_numbers) linebuf1p += sprintf(linebuf1, "         ");
	 putSpace(&linebuf1p, charLen(b2, bytesin2));
	 bytesin1 += charLen(b2, bytesin2);
      }
      if(off2 >= 0) {
	 if(bytesin2==0 && line_numbers) {
	    if(!formatted) linebuf2p += sprintf(linebuf2, "%08X:", off2);
	    else           linebuf2p += sprintf(linebuf2, "% 8d:", line2);
	    needadr2 = false;
	 }
	 if(needadr2 && line_numbers) {
	    char t = linebuf2[9];
	    if(!formatted) sprintf(linebuf2, "%08X:", off2);
	    else           sprintf(linebuf2, "% 8d:", line2);
	    linebuf2[9] = t;
	    needadr2 = false;
	 }
	 if(lastcolor2 != diff) {
	    linebuf2p += sprintf(linebuf2p, colorStr(diff));	 
	    lastcolor2 = diff;
	 }
	 putChar(&linebuf2p, b2, bytesin2); 
	 bytesin2+=charLen(b2, bytesin2);
	 if((off1>=0)&&(charLen(b1, bytesin1)>charLen(b2, bytesin2))) { 
	    putSpace(&linebuf2p, charLen(b1, bytesin1)-charLen(b2, bytesin2)); 
	    bytesin2+=charLen(b1, bytesin1)-charLen(b2, bytesin2);	    
	 }
      } else {
	 if(bytesin2==0 && line_numbers) linebuf2p += sprintf(linebuf2, "         ");
	 putSpace(&linebuf2p, charLen(b1, bytesin1));
	 bytesin2+=charLen(b1, bytesin1);
      }
   }
   if(formatted) {
      if(((off1>=0)&&(b1=='\n'))||((off2>=0)&&(b2=='\n'))) {
	 if((off1>=0)&&(b1=='\n')) line1++;
	 if((off2>=0)&&(b2=='\n')) line2++;
	 flush(); 
      }
   }
}


void TDiffOutput::putHexElem(int off1, uchar b1, int off2, uchar b2, DIFF_T diff) {
   if((bytesin1 == bytes_per_line)||(bytesin2 == bytes_per_line)) flush();
   if((bytesin1 > max_bytes_per_line)||(bytesin2 > max_bytes_per_line)) {
      bytesin1++;
      bytesin2++;
   } else {
      if(off1 >= 0) {
	 if(bytesin1==0) {
	    linebuf1p += sprintf(linebuf1, "%08X:", off1);
	    needadr1 = false;
	 }
	 if(needadr1) {
	    char t = linebuf1[9];
	    sprintf(linebuf1, "%08X:", off1);
	    linebuf1[9] = t;
	    needadr1 = false;
	 }
	 if(lastcolor1 != diff) {
	    linebuf1p += sprintf(linebuf1p, colorStr(diff));
	    lastcolor1 = diff;
	 }
	 char t1 = ' ';
	 if(alignment_marks) {
	    if((off1&7)==0) t1 = '+';
	    else if((off1&3)==0) t1 = '-';
	 }
	 if(bytesin1) linebuf1p += sprintf(linebuf1p, "%c%02X", t1, b1);
	 else         linebuf1p += sprintf(linebuf1p,  "%02X", b1);
	 bytesin1++;
      } else {
	 if(bytesin1==0) linebuf1p += sprintf(linebuf1, "           ");
	 else linebuf1p += sprintf(linebuf1p, "   ");
	 bytesin1++;
      }
      if(off2 >= 0) {
	 if(bytesin2==0) {
	    linebuf2p += sprintf(linebuf2, "%08X:", off2);
	    needadr2 = false;
	 }
	 if(needadr2) {
	    char t = linebuf2[9];
	    sprintf(linebuf2, "%08X:", off2);
	    linebuf2[9] = t;
	    needadr2 = false;
	 }
	 if(lastcolor2 != diff) {
	    linebuf2p += sprintf(linebuf2p, colorStr(diff));	 
	    lastcolor2 = diff;
	 }
	 char t2 = ' ';
	 if(alignment_marks) {
	    if((off2&7)==0) t2 = '+';
	    else if((off2&3)==0) t2 = '-';
	 }
	 if(bytesin2) linebuf2p += sprintf(linebuf2p, "%c%02X", t2, b2);
	 else         linebuf2p += sprintf(linebuf2p,  "%02X", b2);
	 bytesin2++;
      } else {
	 if(bytesin2==0) linebuf2p += sprintf(linebuf2, "           ");
	 else linebuf2p += sprintf(linebuf2p, "   ");
	 bytesin2++;
      }
   }
}


const char *TDiffOutput::colorStr(DIFF_T diff) const {
   switch(diff) {
    case MAT: return color_mat;
    case SUB: return color_sub;
    case INS: return color_ins;
    case DEL: return color_del;
    default:
      fatalError("internal error: diff=%d\n", diff);
   }
}


void TDiffOutput::printSplitLine(char *buf1, char *buf2) const {
   setStrLen(buf1, half_line_len);
   setStrLen(buf2, half_line_len);
   printf("%s%s|%s%s\n", buf1, color_sep, color_nor, buf2);
}


void TDiffOutput::mat(int num) {
   int i;
   char buf1[10];
   char buf2[10];
   switch(mode) {
    case VERTICAL:
      if(hide_mat) {
	 o1 += num;
	 o2 += num;
	 return;
      }
      if(range_mat) {
	 printf("0x%08X (%10d): %s%10d bytes match     %s :(%10d) 0x%08X\n", 
		o1, o1, color_mat, num, color_nor, o2, o2);
	 o1 += num;
	 o2 += num;
	 return;
      }
      for(i=0; i<num; i++, o1++, o2++) {
	 printf("0x%08X (%10d): %s%s %3d 0x%02X   0x%02X %3d %s%s :(%10d) 0x%08X\n",
		o1, o1, color_mat, printChar(f1[o1], buf1), f1[o1], f1[o1], 
		f2[o2], f2[o2], printChar(f2[o2], buf2), color_nor, o2, o2);
      }
      break;

    case F_ASCII:
    case U_ASCII:
    case HEX:
      if(hide_mat) {
	 flush();
	 o1 += num;
	 o2 += num;
	 return;
      }
      if(range_mat) {
	 flush();
	 sprintf(linebuf1, "%08X: %s%10d bytes match%s", o1, color_mat, num, 
		 color_nor);
	 sprintf(linebuf2, "%08X: %s%10d bytes match%s", o2, color_mat, num, 
		 color_nor);
	 printSplitLine(linebuf1, linebuf2);
	 o1 += num;
	 o2 += num;
	 return;
      }
      for(i=0; i<num; i++, o1++, o2++) {
	 if(mode==HEX) putHexElem(o1, f1[o1], o2, f2[o2], MAT);
	 else          putAscElem(o1, f1[o1], o2, f2[o2], MAT, mode==F_ASCII);
      }
      break;
   }
}


void TDiffOutput::sub(int num, int ins, int del) {
   int i;
   char buf1[10];
   char buf2[10];
   switch(mode) {
    case VERTICAL:
      if(hide_sub) {
	 o1 += num + del;
	 o2 += num + ins;
	 return;
      }
      if(range_sub) {
	 printf("0x%08X (%10d): %s%10d subst %10d%s :(%10d) 0x%08X\n",
		o1, o1, color_sub, num + del, num + ins, color_nor, o2, o2);
	 o1 += num + del;
	 o2 += num + ins;
	 return;
      }
      for(i=0; i<num; i++, o1++, o2++) {
	 printf("0x%08X (%10d): %s%s %3d 0x%02X ! 0x%02X %3d %s%s :(%10d) 0x%08X\n",
		o1, o1, color_sub, printChar(f1[o1], buf1), f1[o1], f1[o1], 
		f2[o2], f2[o2], printChar(f2[o2], buf2), color_nor, o2, o2);
      }
      for(i=0; i<del; i++, o1++) {
	 printf("0x%08X (%10d): %s%s %3d 0x%02X !%s\n",
		o1, o1, color_sub, printChar(f1[o1], buf1), f1[o1], f1[o1], color_nor);
      }
      for(i=0; i<ins; i++, o2++) {
	 printf("                                      %s! 0x%02X %3d %s%s :(%10d) 0x%08X\n",
		color_sub, f2[o2], f2[o2], printChar(f2[o2], buf1), color_nor, o2, o2);
      }
      break;

    case F_ASCII:
    case U_ASCII:
    case HEX:
      if(hide_sub) {
	 flush();
	 o1 += num + del;
	 o2 += num + ins;
	 return;
      }
      if(range_sub) {
	 flush();
	 sprintf(linebuf1, "%08X: %s%10d bytes substituted%s", o1, color_sub, 
		 num + del, color_nor);
	 sprintf(linebuf2, "%08X: %s%10d bytes substituted%s", o2, color_sub, 
		 num + ins, color_nor);
	 printSplitLine(linebuf1, linebuf2);
	 o1 += num + del;
	 o2 += num + ins;
	 return;
      }
      for(i=0; i<num; i++, o1++, o2++) {
	 if(mode==HEX) putHexElem(o1, f1[o1], o2, f2[o2], SUB);
	 else          putAscElem(o1, f1[o1], o2, f2[o2], SUB, mode==F_ASCII);
      }
      for(i=0; i<del; i++, o1++) {
	 if(mode==HEX) putHexElem(o1, f1[o1], -1, 0, SUB);
	 else          putAscElem(o1, f1[o1], -1, 0, SUB, mode==F_ASCII);
      }
      for(i=0; i<ins; i++, o2++) {
	 if(mode==HEX) putHexElem(-1, 0, o2, f2[o2], SUB);
	 else          putAscElem(-1, 0, o2, f2[o2], SUB, mode==F_ASCII);
      }
      break;
   }
}


void TDiffOutput::del(int num) {
   int i;
   char buf[10];
   switch(mode) {
    case VERTICAL:
      if(hide_del) {
	 o1 += num;
	 return;
      }
      if(range_del) {
	 printf("0x%08X (%10d): %s%10d bytes deleted   %s\n",
		o1, o1, color_del, num, color_nor);
	 o1 += num;
	 return;
      }
      for(i=0; i<num; i++, o1++) {
	 printf("0x%08X (%10d): %s%s %3d 0x%02X <%s\n",
		o1, o1, color_del, printChar(f1[o1], buf), f1[o1], f1[o1], color_nor);
      }
      break;

    case F_ASCII:
    case U_ASCII:
    case HEX:
      if(hide_del) {
	 flush();
	 o1 += num;
	 return;
      }
      if(range_del) {
	 flush();
	 sprintf(linebuf1, "%08X: %s%10d bytes deleted%s", o1, color_del, 
		 num, color_nor);
	 *linebuf2=0;
	 printSplitLine(linebuf1, linebuf2);
	 o1 += num;
	 return;
      }
      for(i=0; i<num; i++, o1++) {
	 if(mode==HEX) putHexElem(o1, f1[o1], -1, 0, DEL);
	 else          putAscElem(o1, f1[o1], -1, 0, DEL, mode==F_ASCII);
      }
      break;
   }
}


void TDiffOutput::ins(int num) {
   int i;
   char buf[10];
   switch(mode) {
    case VERTICAL:
      if(hide_ins) {
	 o2 += num;
	 return;
      }
      if(range_ins) {
	 printf("                         %s%10d bytes inserted  %s :(%10d) 0x%08X\n",
		color_ins, num, color_nor, o2, o2);
	 o2 += num;
	 return;
      }
      for(i=0; i<num; i++, o2++) {
	 printf("                                      %s> 0x%02X %3d %s%s :(%10d) 0x%08X\n",
		color_ins, f2[o2], f2[o2], printChar(f2[o2], buf), color_nor, o2, o2);
      }
      break;

    case F_ASCII:
    case U_ASCII:
    case HEX:
      if(hide_ins) {
	 flush();
	 o2 += num;
	 return;
      }
      if(range_ins) {
	 flush();
	 sprintf(linebuf1, "%08X: %s%10d bytes inserted%s", o2, color_ins, 
		 num, color_nor);
	 *linebuf2=0;
	 printSplitLine(linebuf2, linebuf1);
	 o2 += num;
	 return;
      }
      for(i=0; i<num; i++, o2++) {
	 if(mode==HEX) putHexElem(-1, 0, o2, f2[o2], INS);
	 else          putAscElem(-1, 0, o2, f2[o2], INS, mode==F_ASCII);
      }
      break;
   }
}


TDiffOutput::MODE_T TDiffOutput::autoMode() {
   int i;
   double newline=0;
   double noascii=0; 
   double num=0;
   int s1=10000; // chars to read from each file
   int s2=10000;
   
   // adjust sizes
   if(s1 > f1.size()) s1 = f1.size();
   if(s2 > f2.size()) s2 = f2.size();
   
   // count chars
   for(i=0; i<s1; i++, num++) {
      if(f1[i]=='\n') newline++;
      if((f1[i]>126)||(f1[i]==0)) noascii++;
   }
   for(i=0; i<s1; i++, num++) {
      if(f1[i]=='\n') newline++;
      if((f1[i]>126)||(f1[i]==0)) noascii++;
   }
   
   // determine mode
   num /= 100.0; // convert to percent
   newline /= num;
   noascii /= num;
   if(noascii > 10.0) {
      if(verbose) printf("files contain %.1f%% > 10.0%% non ascii chars ==> hex mode\n", noascii);
      return HEX;
   }
   if(newline < 1.0) {
      if(verbose) printf("files contain %.1f%% < 1.0%% newline chars ==> unformatted-ascii mode\n", newline);
      return U_ASCII;
   }
   if(verbose) printf("files contain %.1f%% >= 1.0%% newline chars ==> formatted-ascii mode\n", newline);
   return F_ASCII;
}





















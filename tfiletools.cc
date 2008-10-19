/*GPL*START*
 * tapplication frame helper - some file utils
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

#include "tfiletools.h"
#include <sys/stat.h>
#include <unistd.h>

bool fisdir(const char *fname) {
   struct stat buf;   
   if(stat(fname, &buf)) return false;
   if(S_ISDIR(buf.st_mode)) return true;
   else return false;
}

bool fisregular(const char *fname) {
   struct stat buf;   
   if(stat(fname, &buf)) return false;
   if(S_ISREG(buf.st_mode)) return true;
   else return false;
}

bool fissymlink(const char *fname) {
#ifdef __STRICT_ANSI__
   fname = fname;
   return false;
#else
   struct stat buf;   
   if(lstat(fname, &buf)) return false;
   if(S_ISLNK(buf.st_mode)) return true;
   else return false;
#endif
}

bool fexists(const char *fname) {
   struct stat buf;   
   if(stat(fname, &buf)) return false;
   else return true;
}

int flen(const char *fname) {
   struct stat buf;   
   if(stat(fname, &buf)) return -1;
   return buf.st_size;
}

int flen(int fdes) {
   struct stat buf;   
   if(fstat(fdes, &buf)) return -1;
   return buf.st_size;
}

#ifndef __STRICT_ANSI__
int flen(FILE *file) {
   struct stat buf;   
   if(fstat(fileno(file), &buf)) return -1;
   return buf.st_size;
}
#endif













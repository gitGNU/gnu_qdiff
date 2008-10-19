/*GPL*START*
 * tapplication frame helper - some file utils
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *GPL*END*/  

#ifndef _ngw_file_tools_h_
#define _ngw_file_tools_h_
#include <stdio.h>

// history: (start unknown, ca. 1997, like tobject)
// 1998:
// 01 Oct 13:34 fisdir added
// 07 Dec 21:58 flen(FILE*), flen(int) added



// return true if file is a regular file
bool fisdir(const char *fname);

// return true if file is a regular file
bool fisregular(const char *fname);

// return true if file is a symbolic link
bool fissymlink(const char *fname);

// return false if nothing with this name exists
bool fexists(const char *fname);

// return length of file or -1 if file does not exist
int flen(const char *fname);
int flen(int fdes);
#ifndef __STRICT_ANSI__
int flen(FILE *file);
#endif

#endif













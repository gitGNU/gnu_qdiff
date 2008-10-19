/*GPL*START*
 * terror - error handling and reporting for a tapplication
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *GPL*END*/  



#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define _in_terror_cc_
#include "terror.h"

// history:
// 1999:
// 14:06 15 Feb terror.cc derived from tobject.cc
// 21:12 25 Aug noreturn hack for gcc2.95.1
// 2008-10-19 Fix missing #include for gcc 4.3


// global data:

// application name set by TAppconfig:
const char *terrorApplicationName = 0;


void userWarning(const char *message, ...) {
   va_list ap;
   
   va_start(ap, message);
   if(terrorApplicationName) fprintf(stderr, "\r%s warning: ", terrorApplicationName);
   else fprintf(stderr, "\rwarning: ");
   vfprintf(stderr, message, ap);
   va_end(ap);
}


void userError(const char *message, ...) {
   va_list ap;
   
   va_start(ap, message);
   if(terrorApplicationName) fprintf(stderr, "\r%s: ", terrorApplicationName);
   else fprintf(stderr, "\rerror: ");
   vfprintf(stderr, message, ap);
   va_end(ap);
   exit(1);
}


void fatalError_func2(const char *message, ...) {
   va_list ap;
   
   va_start(ap, message);
   vfprintf(stderr, message, ap);
   va_end(ap);
   exit(1);
}


void fatalError_func1(const char *file, int line, const char *function) {
   fprintf(stderr, "%s(%d): fatal error in function '%s':\n", file, line, function);
}



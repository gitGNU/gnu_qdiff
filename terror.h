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

#ifndef _ngw_terror_h_
#define _ngw_terror_h_


// history:
// 1999:
// 00:00 15 Feb started from tobject
// 00:56 16 Apr exceptions added
// 23:28 13 May TErrnoException added
// 17:03 04 Jun exceptions moved to texception.h and tbaseexception.h


// error reporting
void userWarning(const char *message, ...) __attribute__ ((format(printf,1,2)));
void userError(const char *message, ...) __attribute__ ((noreturn,format(printf,1,2)));


#define fatalError (fatalError_func1(__FILE__,__LINE__,__PRETTY_FUNCTION__),fatalError_func2)
#ifdef _in_terror_cc_
void fatalError_func1(const char *file, int line, const char *function);
#else
void fatalError_func1(const char *file, int line, const char *function) __attribute__ ((noreturn));
#endif
void fatalError_func2(const char *message, ...) __attribute__ ((noreturn,format(printf,1,2)));

#endif


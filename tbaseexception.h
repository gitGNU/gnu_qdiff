/*GPL*START*
 * tbaseexception - basic exceptions (used by tstring)
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

#ifndef _ngw_tbaseexception_h_
#define _ngw_tbaseexception_h_

#include <string.h>

// history:
// 1999:
// 17:02 04 Jun derived from terror.h


// base class of all exceptions 
class TException {};

// general exceptions, also base classes
class TIndexOutOfRangeException: public TException {
 public:
   TIndexOutOfRangeException(int lower, int index, int upper): 
   lower(lower), index(index), upper(upper) {}
   int lower, index, upper;
};

class TZeroBasedIndexOutOfRangeException: public TIndexOutOfRangeException {
 public:
   TZeroBasedIndexOutOfRangeException(int index, int total_num):
   TIndexOutOfRangeException(0, index, total_num-1) {}
};

class TErrnoException: public TException {
 public:
   TErrnoException(int err): err(err) {}
   const char *str() const {return strerror(err);}
   int err;
};

#endif

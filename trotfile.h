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

#ifndef _trotfile_h_
#define _trotfile_h_

#include "terror.h"
#include "ttypes.h"
#include "tstring.h"

class TROTFile {
 public:
   // ctor & dtor
   TROTFile(const char *fname, int numbuf, int bufsize);
   ~TROTFile();

   // readonly access
   uchar operator[] (int i);
   int size() const {return _size;}
   const char *name() const {return fname.data();};
   
 private:
   // internal buffer
   int numbuf;   // number of buffer
   int bufsize;  // size of buffer
   int bufbits;  // bits of buffer size
   int bufmask;  // address mask for buffer (in)
   int nummask;  // address mask for buffer (which)
   int offmask;  // address mask for offset
   int *off;     // offset of buffer
   uchar **buf;  // buffer
   
   // real file
   int _size;    // size of file
   tstring fname; // filename
   FILE *file;   // open file
   
   // private methods
   void loadBuf(int offset, int buffer);
   int intLog2(int i) const;
   bool isPowerOf2(int i) const;
   
   // forbid copy
   TROTFile(const TROTFile& a);
   const TROTFile& operator=(const TROTFile& a);
};

inline uchar TROTFile::operator[] (int i) {
   if(((uint)i) < ((uint)_size)) {
      int offset = i&offmask;
      int buffer = (i >> bufbits) & nummask;
      if(offset!=off[buffer]) loadBuf(offset, buffer);
      return buf[buffer][i & bufmask];
   } else 
     fatalError("operator[]: index out of range! (%d not in [0..%d])\n", 
		i, _size-1);
}

#endif






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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * *GPL*END*/  

#include "trotfile.h"
#include <sys/stat.h>
#include <unistd.h>


TROTFile::TROTFile(const char *filename, int num_buf, int buf_size)
:numbuf(num_buf), bufsize(buf_size), bufbits(0), bufmask(0), nummask(0),
offmask(0), off(new int[numbuf]), 
buf(new uchar *[numbuf]), _size(0), fname(filename), file(0)
{
   bool nonreg = false;
   
   // check numbuf and bufsize
   if((!isPowerOf2(numbuf)) || (!isPowerOf2(bufsize)))
     fatalError("numbuf and bufsize must be powers of two!\n");
   
   // calc masks
   bufmask = bufsize-1;
   offmask = ~bufmask;
   bufbits = intLog2(bufsize);
   nummask = numbuf-1;
   
   // get file existance, type and _size
   struct stat a;
   if(stat(filename, &a)) 
     userError("file '%s' does not exist!\n", filename);
   if(!S_ISREG(a.st_mode)) {
      userWarning("'%s' is not a regular file\n", filename);
      nonreg = true;
   }
   _size = a.st_size;
   
   // open file
   file = fopen(filename, "rb");
   if(file==0) 
     userError("error while opening file '%s' for reading!\n", filename);

   if(nonreg) {
      // get size of nonregular file
      int s;
      char tmp;
      
      for(s=1; s>0; s<<=1) {
	 if((fseek(file, s , SEEK_SET)!=0)||(fread(&tmp, 1, 1, file)!=1)) break;	 
      }
      if(s<=0)
	userError("file '%s' has zero size or is too large\n", filename);
      
      int lo = s/2;
      int hi = s;
      while((lo+1) < hi) { 
	 s = (lo+hi)/2;
	 if((fseek(file, s, SEEK_SET)!=0)||(fread(&tmp, 1, 1, file)!=1)) { 
	    hi = s;
	 } else {
	    lo = s;
	 }
      }
      rewind(file);      

      _size = hi;
      userWarning("assuming size %d (%.1fk, %0.1fm) for file '%s'\n", 
		  _size, double(_size)/1024.0, double(_size)/1024.0/1024.0,
		  filename);     
   }
   
   // alloc buffers
   for(int i=0; i<numbuf; i++) {
      buf[i] = new uchar[bufsize];
      off[i] = -1; // invalidate buffer
   }
   
   
}


TROTFile::~TROTFile() {
   fclose(file);
   for(int i=0; i<numbuf; i++) 
     delete[] buf[i];
   delete[] off;
   delete[] buf;
}


int TROTFile::intLog2(int i) const {
   if(i<=0) fatalError("IntLog2: i must be >0! (was %d)", i);
   int r;
   for(r=0; i!=1; r++, i>>=1) ;
   return r;
}


bool TROTFile::isPowerOf2(int i) const {
   if(i<=0) return false;
   for(; (i&1)==0; i>>=1) ;
   if(i==1) return true;
   else return false;
}


void TROTFile::loadBuf(int offset, int buffer) {
   if(fseek(file, offset, SEEK_SET))
     fatalError("LoadBuf: fseek failed!\n");
   int len = bufsize;
   if(offset==(_size&offmask)) len = _size & bufmask; 
   int r = fread(buf[buffer], 1, len, file);
   if(r != len)
     fatalError("LoadBuf: fread failed!\n");
   off[buffer] = offset;
}

















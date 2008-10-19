/*GPL*START*
 * texception - common exceptions using class tstring
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

#ifndef _ngw_texception_h_
#define _ngw_texception_h_

#include "tbaseexception.h"
#include "tstring.h"


// history:
// 1999:
// 16:58 04 Jun derived from terrno.h


class TFileOperationErrnoException: public TErrnoException {
 public:
   TFileOperationErrnoException(const TString& filename, const TString& operation, int err):
   TErrnoException(err), filename(filename), operation(operation) {}
   TString filename;
   TString operation;
};

#endif



/*GPL*START*
 * 
 * relops - implicit relational operator templates
 * 
 * Copyright (C) 1999 by Johannes Overmann <overmann@iname.com>
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

#ifndef _ngw_trelops_h_
#define _ngw_trelops_h_

// history:
// 1999:
// 13 Apr 23:45 started and finished !=, >, <=, >= works, its all very trivial



// class T needs operator == to get operator !=
// class T needs operator < to get operator >, >= and <=

template<class T> inline bool operator != (const T& a, const T& b) {return !(a == b);}
template<class T> inline bool operator >  (const T& a, const T& b) {return b < a;}
template<class T> inline bool operator <= (const T& a, const T& b) {return !(b < a);}
template<class T> inline bool operator >= (const T& a, const T& b) {return !(a < b);}

#endif  _relops_h_


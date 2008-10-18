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

#ifndef _ngw_tminmax_h_
#define _ngw_tminmax_h_

template<class T> inline T tMin(const T& a, const T& b) {return a<=b?a:b;}
template<class T> inline T tMax(const T& a, const T& b) {return a>=b?a:b;}
template<class T> inline T tAbs(const T& a) {return a>=0?a:-a;}

template<class T> inline bool tOutOfRange(const T& value, 
					 const T& lower, 
					 const T& upper) {
   if((value<lower) || (value>upper)) return true; else return false;}

#endif  _minmax_h_

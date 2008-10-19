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

#ifndef _ngw_tarray_h_
#define _ngw_tarray_h_

#include "terror.h"
#include "texception.h"
#include "tminmax.h"

// history: start unknown (ca. 1997)
// 1998:
// 22 Oct 11:15 isAutomatic/Fixed added
// 22 Oct 12:15 isAutomatic/Fixed made const, const lastElement added
// 1999:
// 15 Feb 15:08 type info removed
// 15 Apr 22:57 fatalErrors replaced by exceptions


/**@name dynamic array with range checking */
/*@{*/
/**
 The TArray template class implements dynamic arrays with range checking.
 Fixed sized and automatic sized (dynamic) arrays are supported.
 No iterator class is needed: Just use an integer, num() and operator[].
 All types are T allowed, but every class T needs: default ctor, operator== and operator=.
 @memo dynamic array with range checking
 */
template<class T>
class TArray {
   // friend: comparison operator:
   friend bool operator == <> (const TArray<T>& a, const TArray<T>& b);

 public:
   
   /**@name constructor & destructor */
   /*@{*/
   /// construct automatic sized array
   TArray(); 
   /// construct fixed sized array
   TArray(int size); // throw(IllegalSize);
   /// copy construction
   TArray(const TArray& a);
   /// single element to array conversion, does not work for int 
//   TArray(const T&);
   /// destructor
   ~TArray()  {delete[] array;}   
   /*@}*/
   
   /**@name size and emptyness */
   /*@{*/
   /// get current number of elements
   int num() const {return _num;}
   /// return true == array is empty
   bool isEmpty() const {return _num == 0;}
   /// return true == array is not empty
   bool isNotEmpty() const {return _num != 0;}
   /// return true == array is not empty
   operator bool() const {return _num != 0;}
   /*@}*/

   /**@name main interface */
   /*@{*/
   /// clear array (num=0)
   void empty() {_num=0;}
   // implicit c-style array conversion
   // operator const T * () const {return array;}
   /// explicit c-style array conversion, dangerous! use with caution!
   T * data() const {return array;}
   /// safe access operator, automatically increases size if automatic array
   T& operator[](int i) {return array[createIndex(i)];} // throw(IndexOutOfRange);
   /// const/readonly access (does not change size)
   const T& operator[](int i) const {return array[checkIndex(i)];} // throw(IndexOutOfRange);
   /// forced read only access (does not change size)
   const T& operator()(int i) const {return array[checkIndex(i)];} // throw(IndexOutOfRange);
   /// canon assignement (by copy)
   const TArray& operator = (const TArray& a);
   /// concat (append another array to the end)
   const TArray& operator += (const TArray& a);
   /// append an element to the end
   const TArray& operator += (const T& a);
   /*@}*/
   
   /**@name misc/aux interface */
   /*@{*/
   /// return index of element t if found, else -1
   int find(const T& t) const;
   /// resize the array, this will either truncate the array or append default constructed elements
   void resize(int size); // throw(IllegalSize);
   /// switch to automatic allocation mode
   void automaticSize() {automatic = true;}
   /// switch to fixed size
   void fixedSize() {automatic = false;}
   /// return true if array has automatic allocation mode
   bool isAutomatic() const {return automatic;}
   /// return true if array has fixed size
   bool isFixed() const {return !automatic;}
   /// garbage collection (internally release unused elements)
   void garbageCollection() {resize(_num);}
   void fit() {resize(_num);}
   /// copy the last element to index i and decrease size by 1 element (this effectively removes the element i from the array, fast)
   void shrinkRemove(int i); // throw(IndexOutOfRange);
   /// remove element i (copy all following one backward, slow)
   void slowRemove(int i); // throw(IndexOutOfRange);
   /// return the last element in the array
   T& lastElement(); // throw(ArrayIsEmpty);
   const T& lastElement() const; // throw(ArrayIsEmpty);
   /// remove the last element
   void killLastElement(); // throw(ArrayIsEmpty);
   /*@}*/
   
   /**@name exceptions */
   /*@{*/
   class ArrayIsEmpty: public TException {};
   class IllegalSize: public TException {
    public:
      IllegalSize(int size): size(size) {}
      int size;
   };
   /*@}*/
   
   
 private: 
   // private methods
   
   // create index i if it does not exist
   int createIndex(int i); // throw(IndexOutOfRange);
   // check if i is a valid index for the size
   int checkIndex(int i) const; // throw(IndexOutOfRange);
   // copy _num elements from shrink to dst
   void arrayCopy(T *dst, T *src, int _num); 
   // resize the private memory area to size
   void resizePrivate(int size);
   
   
   // private data

   // initial size of array
   static const int INITIAL_SIZE;
   // the array itself
   T *array;
   // current number of elements in the array
   int _num;
   // maximal number of elements in the array
   int max;
   // mode of operation
   bool automatic;
};

template<class T>
const int TArray<T>::INITIAL_SIZE = 1; // must be >0


// inline implementation 

template<class T>
inline TArray<T>::TArray(): array(new T[INITIAL_SIZE]), 
                            _num(0), max(INITIAL_SIZE), automatic(true) {}

template<class T>
inline TArray<T>::TArray(int size): array(0), _num(size), 
max(0), automatic(false) 
{
   if(size < 0) throw IllegalSize(size);
   max       = size?size:INITIAL_SIZE;
   array     = new T[max];
}

template<class T>
inline int TArray<T>::createIndex(int i) {
   if((i >= 0) && (i < _num)) return i; // index already exists
   
   // index out of bounds
   if(automatic) {
      if(i < 0) throw TZeroBasedIndexOutOfRangeException(i, _num);
      if(i >= max) resize(tMax(i+1, max*2 + 2));
      _num = i + 1;
      return i;
   }
   else throw TZeroBasedIndexOutOfRangeException(i, _num);
}

template<class T>
inline int TArray<T>::checkIndex(int i) const {
   if((i >=0 ) && (i < _num)) return i;
   else throw TZeroBasedIndexOutOfRangeException(i, _num);
}


template<class T>
inline const TArray<T>& TArray<T>::operator += (const T& elem) {
   if(_num==max) resizePrivate(max*2 + 2);
   array[_num++] = elem;
   return *this;
}


template<class T>
inline void TArray<T>::resize(int size) { // resize the arrays size, perhaps truncate
   if(size<0) throw IllegalSize(size);
   resizePrivate(size);
   _num = size;
}

template<class T>
inline void TArray<T>::shrinkRemove(int i) {
   array[checkIndex(i)] = array[_num-1];
   _num--;
}




// non inline inplementation

template<class T>
void TArray<T>::slowRemove(int i) {
   checkIndex(i);
   arrayCopy(&array[i], &array[i+1], _num-i-1);
   _num--;
}


template<class T>
TArray<T>::TArray(const TArray<T>& a): array(0), _num(a._num), 
max(a._num?a._num:INITIAL_SIZE), automatic(a.automatic) {
   array = new T[max];
   arrayCopy(array, a.array, _num);
}


template<class T>
const TArray<T>& TArray<T>::operator = (const TArray<T>& a) {
   if(&a == this) return *this;
   delete[] array;
   _num      = a._num;
   max       = _num?_num:INITIAL_SIZE;
   automatic = a.automatic;
   array     = new T[max];
   arrayCopy(array, a.array, _num);
   return *this;
}


template<class T>
const TArray<T>& TArray<T>::operator += (const TArray<T>& a) {
   int old = _num;   
   resize(_num + a._num);
   arrayCopy(&array[old], a.array, a._num);
   return *this;
}


template<class T>
int TArray<T>::find(const T& t) const {
   for(int i=0; i<_num; i++) 
     if(array[i] == t) return i;
   // not found
   return -1; 
}


template<class T>
void TArray<T>::resizePrivate(int size) {
   if(size < _num) _num = size; // truncate
   max = size?size:INITIAL_SIZE;
   T* newarray = new T[max];
   arrayCopy(newarray, array, _num);
   delete[] array;
   array = newarray;
}

template<class T>
void TArray<T>::arrayCopy(T *dst, T *src, int n) {
   for(int i = 0; i < n; i++) 
     dst[i] = src[i];
}

template<class T>
T& TArray<T>::lastElement() {
   if(_num > 0) return array[_num-1];
   else throw ArrayIsEmpty();
}

template<class T>
const T& TArray<T>::lastElement() const {
   if(_num > 0) return array[_num-1];
   else throw ArrayIsEmpty();
}

template<class T>
void TArray<T>::killLastElement() {
   if(_num > 0) _num--;
   else throw ArrayIsEmpty();
}



/**@name friends and nonmemebrs */
/*@{*/
/// compare (per element)
template<class T>
bool operator==(const TArray<T>& a, const TArray<T>& b) {
   if(a._num != b._num) return false;
   for(int i = 0; i < a._num; i++) if(a[i] != b[i]) return false;
   return true;
}
/// concat two arrays
template<class T>
inline TArray<T> operator+(const TArray<T>& a, const TArray<T>& b) {
   TArray<T> r(a);
   r += b;
   return r;
}
/*@}*/
/*@}*/

#endif _ngw_tarray_h_



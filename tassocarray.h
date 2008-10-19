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

#ifndef _ngw_tassocarray_h_
#define _ngw_tassocarray_h_
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include "tarray.h"
#include "terror.h"
#include "tstring.h"


// history: start unknown
// 1998:
// 09 Oct 21:00 egcs Weffc++ warning (need init) removed, thanks to kai/stl_pair.h
// 1999:
// 15 Feb 15:08 type info removed
// 19 Apr 22:42 exception InvalidPointer added, fatalErrors removed


// some primes (good hashtable sizes)
static int some_primes[]= {11, 31, 101, 307, 1009, 3001,// usual sizes
     10007, 30011, 100003, 300007, 1000003, 3000017, 10000019, // pretty big
     30000001, 100000007,300000007, 1000000007, 0};// are you nuts?

// every class T needs:
//      defaut ctor
//      copy ctor


// every class Key needs:
//      defaut ctor
//      copy ctor
//      operator==

template<class Key, class T> class TAssocArray;
template<class Key, class T> class TAssocArrayIter;


// pseudo private pair type
template<class Key, class T>
class TAssocArrayPair {
   friend class TAssocArray<Key,T>;
   friend class TAssocArrayIter<Key,T>;
 public:
   TAssocArrayPair(): key(Key()), t(T()) {}
   // comparison to find a key
   bool operator==(const TAssocArrayPair& a) const {return key == a.key;}
 private:
   TAssocArrayPair(const Key& in_key, const T& out_t): key(in_key), t(out_t) {}
   TAssocArrayPair(const Key& in_key): key(in_key), t(T()) {}
   // data
   Key key;
   T t;
}; 


// associative array

template<class Key, class T>
class TAssocArray {
   friend class TAssocArrayIter<Key,T>;
 public:
   // ctor & dtor
   TAssocArray();
   TAssocArray(const TAssocArray& a);
   ~TAssocArray() {delete[] table;}

   // main interface

   // mumber of elements
   int num() const {return _num;}
   // delete all elements
   void empty();
   // access element (read/write)
   T& operator[](const Key& key);
   // access element read only (const)
   const T& operator[](const Key& key) const;
   // forced read only access
   const T& operator()(const Key& key) const;
   // return wether an element with key is contained or not
   bool contains(const Key& key) const;
   // remove element
   void remove(const Key& key);
   // canon assignement (by copy):
   const TAssocArray& operator = (const TAssocArray& a);

   // report hash stats
   void reportHashStats(FILE *f = stdout) const;
   // optimize size
   void optimizeSize();
   
 private:
   // private data
   TArray<TAssocArrayPair<Key,T> > *table; // hashtable
   int size; // size of hashtable
   int _num; // number of elements

   // private methods: add element
   void add(const Key& key, const T& t);
   T& add(const Key& key);
};


// iterator 
template<class Key,class T>
class TAssocArrayIter {
 public:
   static const bool END;
   // ctor & dtor
   TAssocArrayIter(const TAssocArray<Key,T>&a, bool end=false);
   TAssocArrayIter(const TAssocArrayIter& a):
   h(a.h), i(a.i), aa(a.aa) {};
   
   // main interface
   
   // access the current element: value
   T& operator * () const;
   // access the current element: key
   Key& operator ()() const;
   // canon copy
   const TAssocArrayIter& operator = (const TAssocArrayIter&);
   // inc/dec operators
   const TAssocArrayIter& operator ++ () {next(); return *this;}
   TAssocArrayIter operator ++ (int) {TAssocArrayIter a(*this); next(); return a;}
   const TAssocArrayIter& operator -- () {prev(); return *this;}
   TAssocArrayIter operator -- (int) {TAssocArrayIter a(*this); prev(); return a;}
   // pointer valid?
   operator bool() const {return pointerIsValid();}
   // set pointer to first element
   void toFirst();
   // set pointer to last element
   void toLast();
   
   // exceptions
   class InvalidPointer: public TException {};
   
 private:
   // private methods
   void next();                        // seek next element
   void prev();                        // seek previous element
   bool pointerIsValid() const;        // is pointer valid?
   // private data
   int h;                              // current hashclass
   int i;                              // current position in hashclass
   const TAssocArray<Key,T>* aa; // the array this iterator belongs to
};

template<class Key, class T>
const bool TAssocArrayIter<Key,T>::END = true;

// iterator implementation

template<class Key, class T>
inline TAssocArrayIter<Key,T>
::TAssocArrayIter(const TAssocArray<Key,T>& a, bool end): h(0), i(0), aa(&a) {
   if(end) toLast();
   else toFirst();
}


template<class Key,class T>
inline bool TAssocArrayIter<Key,T>::pointerIsValid() const {
   if((h >= 0) && (h < aa->size) && (i >= 0) && (i < aa->table[h].num())) 
     return true;
   else 
     return false;
}


template<class Key,class T>
inline T& TAssocArrayIter<Key,T>::operator * () const {
   if(pointerIsValid()) return aa->table[h][i].t;
   else throw InvalidPointer();
}


template<class Key,class T>
inline Key& TAssocArrayIter<Key,T>::operator ()() const {
   if(pointerIsValid()) return aa->table[h][i].key;
   else throw InvalidPointer();
}


template<class Key,class T>
void TAssocArrayIter<Key,T>::next() {
   if(pointerIsValid()) {
      i++;
      if(i >= aa->table[h].num()) { 
	 i=0;
	 h++;
	 while((h<aa->size) && (aa->table[h].num()==0)) h++;
      }
   }
}

template<class Key,class T>
void TAssocArrayIter<Key,T>::prev() {
   if(pointerIsValid()) {
      i--;
      if(i < 0) {
	 h--;
	 while((h>=0) && (aa->table[h].num()==0)) h--;
	 if(h>=0) i = aa->table[h].num()-1; 
      }
   }
}

template<class Key,class T>
void TAssocArrayIter<Key,T>::toFirst() {
   h=0; 
   i=0;
   while((h<aa->size) && (aa->table[h].num()==0)) h++;
}

template<class Key,class T>
void TAssocArrayIter<Key,T>::toLast() {
   h=aa->size-1; 
   while((h>=0) && (aa->table[h].num()==0)) h--;
   if(h>=0) i = aa->table[h].num()-1; 
}

template<class Key,class T>
inline const TAssocArrayIter<Key,T>& TAssocArrayIter<Key,T>
::operator = (const TAssocArrayIter<Key,T>& a) {
   h = a.h; i = a.i; aa = a.aa;
   return *this;
}


// inline implementation of class TAssocArray

template<class Key, class T>
inline void TAssocArray<Key,T>::add(const Key& key, const T& t) {
   if(_num > size<<1) optimizeSize();
   int hash = hashKeyToInt(key) % unsigned(size);
   table[hash] += TAssocArrayPair<Key,T>(key, t);
   _num++;
}

template<class Key, class T>
inline T& TAssocArray<Key,T>::add(const Key& key) {
   if(_num > size<<1) optimizeSize();
   int hash = hashKeyToInt(key) % unsigned(size);
   table[hash] += TAssocArrayPair<Key,T>(key);
   _num++;
   return (table[hash].lastElement()).t;
}

// non inline implementation

template<class Key, class T>
const TAssocArray<Key,T>& TAssocArray<Key,T>::operator = (const TAssocArray<Key,T>& a) {
   if(&a == this) return *this;
   delete[] table;
   size = a.size;
   _num  = a._num;
   table = new TArray<TAssocArrayPair<Key,T> >[size];
   for(int i=0; i<size; i++) table[i] = a.table[i];
   return *this;
}


template<class Key, class T>
TAssocArray<Key,T>::TAssocArray(const TAssocArray<Key,T>& a):
table(0), size(a.size), _num(a._num) {
   table = new TArray<TAssocArrayPair<Key,T> >[size];
   for(int i=0; i<size; i++) table[i] = a.table[i];
}


template<class Key, class T>
void TAssocArray<Key,T>::optimizeSize() {
   int i;
   
   for(i=0; some_primes[i] && (_num > some_primes[i]); i++) ;
   if(some_primes[i]==0) i--;
   int newsize = some_primes[i];
//   printf("optimizing (_num=%8d,size=%8d,newsize=%8d)\n", _num, size, newsize);
   TArray<TAssocArrayPair<Key,T> > *newtable;
   newtable = new TArray<TAssocArrayPair<Key,T> >[newsize];
   for(i=0; i<size; i++) {
      for(int j=0; j<table[i].num(); j++) {
	 int hash = hashKeyToInt(table[i][j].key) % unsigned(newsize);
	 newtable[hash] += table[i][j];	 
      }
   }
   delete[] table;
   table = newtable;
   size = newsize;
}


template<class Key, class T>
TAssocArray<Key,T>::TAssocArray():table(0), size(some_primes[0]), _num(0) {
   table = new TArray<TAssocArrayPair<Key,T> >[size];
}


template<class Key, class T>
void TAssocArray<Key,T>::empty() {
   delete[] table;
   size = some_primes[0];
   table = new TArray<TAssocArrayPair<Key,T> >[size];
   _num = 0;
}


template<class Key, class T>
T& TAssocArray<Key,T>::operator[](const Key& key) {
   int hash = hashKeyToInt(key) % unsigned(size);
   int i = table[hash].find(key);      
   if(i<0) return add(key);
   else return table[hash][i].t;
}

template<class Key, class T>
const T& TAssocArray<Key,T>::operator[](const Key& key) const {
   int hash = hashKeyToInt(key) % unsigned(size);
   int i = table[hash].find(key);
   return table[hash][i].t;
}

template<class Key, class T>
const T& TAssocArray<Key,T>::operator()(const Key& key) const {
   int hash = hashKeyToInt(key) % unsigned(size);
   int i = table[hash].find(key);
   return table[hash][i].t;
}

template<class Key, class T>
bool TAssocArray<Key,T>::contains(const Key& key) const {
   int hash = hashKeyToInt(key) % unsigned(size);
   return table[hash].find(key) >= 0;
}

template<class Key, class T>
void TAssocArray<Key,T>::remove(const Key& key) {
   int hash = hashKeyToInt(key) % unsigned(size);
   int i = table[hash].find(key);
   table[hash].shrinkRemove(i);
}


template<class Key, class T>
void TAssocArray<Key,T>::reportHashStats(FILE *f) const {
   int min = INT_MAX;
   int minindex = -1;
   int max = INT_MIN;
   int maxindex = -1; 
   double mean = 0.0;
   double var = 0.0;
   
   for(int i=0; i<size; i++) {
      int _num = table[i].num();
      mean += _num;
      if(_num<min) {
	 min = _num;
	 minindex = i;
      }
      if(_num>max) {
	 max = _num;
	 maxindex = i;
      }
   }
   mean /= size;
   for(int i=0; i<size; i++) 
     var += (table[i].num()-mean) * (table[i].num()-mean);
   var = sqrt(var/size);
   fprintf(f, "hashtable size: %d, number of elements: %d\n", size, _num);
   fprintf(f, "shortest list: hashvalue%8d (len=%8d)\n", minindex, min);
   fprintf(f, "longest  list: hashvalue%8d (len=%8d)\n", maxindex, max);
   fprintf(f, "mean length (var): %.1f (+/-%.1f)\n", mean, var); 
}



// some useful hashfunctions for generic types

// string
#if 0
inline int hashKeyToInt(const char *s) {
   if(*s) {
      const char *p = s;
      int i = 42;
      while(*p) i += i*79 + *(p++);
      return i;
   } else return 42;
}
#endif

inline int hashKeyToInt(const TString &s) {
   uint h = s.len()*42;
   const uint *p = (const uint *) *s;
   int l = s.len() / sizeof(uint);
   int i;
   for(i=0; i < l; i++, p++)  h += *p;
   for(i=l*sizeof(uint); i < s.len(); i++) h += s[i];
   return int(h);
}

// float and double: not portable (yet)!
inline int hashKeyToInt(float fl) {
   float f = fl;
   return *((int*)&f);
}

inline int hashKeyToInt(double db) {
   double d = db;
   return ((int*)&d)[0] ^ ((int*)&d)[1];
}

// integer
inline int hashKeyToInt(int u) {return u;}
inline int hashKeyToInt(uint u) {return u;}


#endif // _ngw_tassocarray_h_


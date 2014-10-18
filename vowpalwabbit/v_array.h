/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef VARRAY_H
#define VARRAY_H
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#ifdef _WIN32
#define __INLINE 
#else
#define __INLINE inline
#endif

const size_t erase_point = ~ ((1 << 10) -1);

template<class T> class v_array{
 public:
  T*begin_;
  T*end_;
  T* end_array;
  size_t erase_count;

    T* begin()
    {
        return begin_;
    }

    T* end()
    {
        return end_;
    }

  T last() { return *(end_ -1);}
  T pop() { return *(--end_);}
  bool empty() { return begin_ == end_;}
  void decr() { end_--;}
  v_array() { begin_ = NULL; end_ = NULL; end_array=NULL; erase_count = 0;}
  T& operator[](size_t i) { return begin_[i]; }
  T& get(size_t i) { return begin_[i]; }
  size_t size(){return end_ - begin_;}
  void resize(size_t length, bool zero_everything=false)
    {
      if ((size_t)(end_array- begin_) != length)
	{
	  size_t old_len = end_ - begin_;
	  begin_ = (T *)realloc(begin_, sizeof(T) * length);
	  if ((begin_ == NULL) && ((sizeof(T)*length) > 0)) {
	    std::cerr << "realloc of " << length << " failed in resize().  out of memory?" << std::endl;
	    throw std::exception();
	  }
          if (zero_everything && (old_len < length))
            memset(begin_ +old_len, 0, (length-old_len)*sizeof(T));
	  end_ = begin_ +old_len;
	  end_array = begin_ + length;
	}
    }

  void erase() 
  { if (++erase_count & erase_point)
      {
	resize(end_ - begin_);
	erase_count = 0;
      }
    end_ = begin_;
  }
  void delete_v()
  {
    if (begin_ != NULL)
      free(begin_);
    begin_ = end_ = end_array = NULL;
  }
  void push_back(const T &new_ele)
  {
   if(end_ == end_array)
      resize(2 * (end_array- begin_) + 3);
    *(end_++) = new_ele;
  }
  size_t find_sorted(const T& ele)  //index of the smallest element >= ele, return true if element is in the array
  {
    size_t size = end_ - begin_;
    size_t a = 0;			
    size_t b = size;	
    size_t i = (a + b) / 2;

    while(b - a > 1)
    {
	if(begin_[i] < ele)	//if a = 0, size = 1, if in while we have b - a >= 1 the loop is infinite
		a = i;
	else if(begin_[i] > ele)
		b = i;
	else
		return i;

	i = (a + b) / 2;		
    }

    if((size == 0) || (begin_[a] > ele) || (begin_[a] == ele))		//pusta tablica, nie wchodzi w while
	return a;
    else	//size = 1, ele = 1, begin[0] = 0	
	return b;
 }
 size_t unique_add_sorted(const T &new_ele)//ANNA
 {
   size_t index = 0;
   size_t size = end_ - begin_;
   size_t to_move;

   if(!contain_sorted(new_ele, index))
   {
	if(end_ == end_array)
		resize(2 * (end_array- begin_) + 3);

	to_move = size - index;

	if(to_move > 0)
		memmove(begin_ + index + 1, begin_ + index, to_move * sizeof(T));   //kopiuje to_move*.. bytow z begin_+index do begin_+index+1

	begin_[index] = new_ele;

	end_++;
   }

   return index;
 }
 bool contain_sorted(const T &ele, size_t& index) 
 {
   index = find_sorted(ele);

   if(index == this->size())
	return false;

   if(begin_[index] == ele)
	return true;		

   return false;	
 }
};


#ifdef _WIN32
#undef max
#undef min
#endif

inline size_t max(size_t a, size_t b)
{ if ( a < b) return b; else return a;
}
inline size_t min(size_t a, size_t b)
{ if ( a < b) return a; else return b;
}

template<class T> void copy_array(v_array<T>& dst, v_array<T> src)
{
  dst.erase();
  push_many(dst, src.begin_, src.size());
}

template<class T> void copy_array(v_array<T>& dst, v_array<T> src, T(*copy_item)(T&))
{
  dst.erase();
  for (T*item = src.begin_; item != src.end_; ++item)
    dst.push_back(copy_item(*item));
}

template<class T> void push_many(v_array<T>& v, const T* begin, size_t num)
{
  if(v.end_ +num >= v.end_array)
    v.resize(max(2 * (size_t)(v.end_array - v.begin_) + 3,
		 v.end_ - v.begin_ + num));
  memcpy(v.end_, begin, num * sizeof(T));
  v.end_ += num;
}

template<class T> void calloc_reserve(v_array<T>& v, size_t length)
{
  v.begin_ = (T *)calloc(length, sizeof(T));
  v.end_ = v.begin_;
  v.end_array = v.begin_ + length;
}

template<class T> v_array<T> pop(v_array<v_array<T> > &stack)
{
  if (stack.end_ != stack.begin_)
    return *(--stack.end_);
  else
    return v_array<T>();
}  

template<class T> bool v_array_contains(v_array<T> &A, T x) {
  for (T* e = A.begin_; e != A.end_; ++e)
    if (*e == x) return true;
  return false;
}

#endif  // VARRAY_H__

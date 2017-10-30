/*
  MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
 
  $Id: darts.h,v 1.10 2004/09/20 09:59:16 taku-ku Exp $;

  Copyright (C) 2001-2004 Taku Kudo <taku-ku@is.aist-nara.ac.jp>
  This is free software with ABSOLUTELY NO WARRANTY.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/  
#ifndef _DARTS_H
#define _DARTS_H

#define DARTS_VERSION "0.3"

#include <vector>
#include <cstring>
#include <cstdio>

#ifdef HAVE_ZLIB_H
namespace zlib {
#include <zlib.h>
}
#define SH(p) ((unsigned short)(unsigned char)((p)[0]) | ((unsigned short)(unsigned char)((p)[1]) << 8))
#define LG(p) ((unsigned long)(SH(p)) | ((unsigned long)(SH((p)+2)) << 16))
#endif

namespace Darts {

  template <class T> inline T _max (T x, T y) { return (x > y) ? x : y; }
  template <class T> inline T* _resize (T* ptr, size_t n, size_t l, T v)
  {
    T *tmp = new T [l]; // realloc
    for (size_t i = 0; i < n; ++i) tmp[i] = ptr[i]; // copy
    for (size_t i = n; i < l; ++i) tmp[i] = v;
    delete [] ptr;
    return tmp;
  }

  template <class T>
  class Length { 
  public: size_t operator() (const T *str) const 
    { size_t i; for (i = 0; str[i] != (T)0; ++i) {}; return i; } 
  };

  template <> class Length<char> { 
  public: size_t operator() (const char *str) const  
    { return std::strlen (str); }; 
  };

  template  <class NodeType,  class NodeUType,
	     class ArrayType, class ArrayUType, class LengthFunc = Length<NodeType> >
  class DoubleArrayImpl
  {
  private:

    struct node_t
    {
      ArrayUType code;
      size_t     depth;
      size_t     left;
      size_t     right;
    };

    struct unit_t 
    {
      ArrayType    base;
      ArrayUType   check;
    };

    unit_t         *array;
    size_t         *used;
    size_t         size;
    size_t         alloc_size;
    NodeType       **str;
    size_t         str_size;
    size_t         *len;
    ArrayType      *val;

    unsigned int   progress;
    size_t         next_check_pos;
    int            no_delete;
    int            (*progress_func) (size_t, size_t);

    size_t resize (const size_t new_size)
    {
      unit_t tmp;
      tmp.base = 0;
      tmp.check = 0;
      
      array = _resize (array, alloc_size, new_size, tmp);
      used  = _resize (used,  alloc_size, new_size, (size_t)0);
      alloc_size = new_size;

      return new_size;
    }

    size_t fetch (const node_t &parent, std::vector <node_t> &siblings)
    {
      ArrayUType prev = 0;

      for (size_t i = parent.left; i < parent.right; ++i) {
	if ((len ? len[i] : LengthFunc()(str[i])) < parent.depth) continue;

	const NodeUType *tmp = (NodeUType *)(str[i]);

	ArrayUType cur = 0;
	if ((len ? len[i] : LengthFunc()(str[i])) != parent.depth) 
	  cur = (ArrayUType)tmp[parent.depth] + 1;
	  
	if (prev > cur) throw -3;

	if (cur != prev || siblings.empty()) {
	  node_t tmp_node;
	  tmp_node.depth = parent.depth + 1;
	  tmp_node.code  = cur;
	  tmp_node.left  = i;
	  if (! siblings.empty()) siblings[siblings.size()-1].right = i;

	  siblings.push_back(tmp_node);
	}

	prev = cur;
      }

      if (! siblings.empty())
	siblings[siblings.size()-1].right = parent.right;

      return siblings.size();
    }
     
    size_t insert (const std::vector <node_t> &siblings)
    {
      size_t begin       = 0;
      size_t pos         = _max ((size_t)siblings[0].code + 1, next_check_pos) - 1;
      size_t nonzero_num = 0;
      int    first       = 0;

      if (alloc_size <= pos) resize (pos + 1);

      while (1) {
      next:
	++pos;

	if (array[pos].check) {
	  ++nonzero_num;
	  continue;
	} else if (! first) {
	  next_check_pos = pos;
	  first = 1;
	}

	begin = pos - siblings[0].code;
	if (alloc_size < (begin + siblings[siblings.size()-1].code))
	  resize((size_t)(alloc_size * _max(1.05, 1.0 * str_size / progress)));

	if (used[begin]) continue;

	for (size_t i = 1; i < siblings.size(); ++i)
	  if (array[begin + siblings[i].code].check != 0) goto next;

	break;
      }

      // -- Simple heuristics --
      // if the percentage of non-empty contents in check between the index
      // 'next_check_pos' and 'check' is greater than some constant value (e.g. 0.9),
      // new 'next_check_pos' index is written by 'check'.
      if (1.0 * nonzero_num/(pos - next_check_pos + 1) >= 0.95) next_check_pos = pos;

      used[begin] = 1;
      size = _max (size, begin + (size_t)siblings[siblings.size()-1].code + 1);

      for (size_t i = 0; i < siblings.size(); ++i)
	array[begin + siblings[i].code].check = begin;

      for (size_t i = 0; i < siblings.size(); ++i) {
	std::vector <node_t> new_siblings;

	if (! fetch(siblings[i], new_siblings)) {
	  array[begin + siblings[i].code].base = 
	    val ? (ArrayType)(-val[siblings[i].left]-1) : (ArrayType)(-siblings[i].left-1);

	  if (val && (ArrayType)(-val[siblings[i].left]-1) >= 0) throw -2;

	  ++progress;
	  if (progress_func) (*progress_func) (progress, str_size);

	} else {
	  size_t h = insert(new_siblings);
	  array[begin + siblings[i].code].base = h;
	}
      }

      return begin;
    }

  public:

    typedef ArrayType   value_type;
    typedef NodeType    key_type;

    typedef ArrayType   result_type;  // for compatibility

    struct value_pair_type
    {
      value_type value;
      size_t     length;
    };

    DoubleArrayImpl (): array(0), used(0), size(0), alloc_size(0), no_delete(0) {}
    ~DoubleArrayImpl () { clear(); }

    // helper function
    void setResult (value_type&      x, value_type r, size_t l) { x = r; }
    void setResult (value_pair_type& x, value_type r, size_t l) { x.value = r; x.length = l; }

    int setArray (void *ptr, size_t array_size = 0)
    {
      clear();
      array = (unit_t *)ptr;
      no_delete = 1;
      size = array_size;
      return 1;
    }

    void *getArray ()
    {
      return (void *)array;
    }

    void clear ()
    {
      if (! no_delete) delete [] array;
      delete [] used;
      array      = 0;
      used       = 0;
      alloc_size = 0;
      size       = 0;
      no_delete  = 0;
    }
    
    size_t getUnitSize() { return sizeof(unit_t); };
    
    size_t getSize() { return size; };

    size_t getNonzeroSize ()
    {
      size_t result = 0;
      for (size_t i = 0; i < size; ++i)
	if (array[i].check) ++result;
      return result;
    }

    int build (size_t        _str_size,
	       key_type      **_str,
	       size_t        *_len = 0,
	       value_type   *_val = 0,
	       int (*_progress_func)(size_t, size_t) = 0)
    {
      try {
	if (!_str_size || ! _str) return 0;

	progress_func  = _progress_func;
	str            = _str;
	len            = _len;
	str_size       = _str_size;
	val            = _val;
	progress       = 0;

	resize (1024 * 10);

	array[0].base = 1;
	next_check_pos = 0;

	node_t root_node;
	root_node.left  = 0;
	root_node.right = _str_size;
	root_node.depth = 0;

	std::vector <node_t> siblings;
	fetch (root_node, siblings);
	insert (siblings);

	size += sizeof (ArrayType);
	if (size > alloc_size) resize (size);

	delete [] used;
	used  = 0;
	return 0;
      }

      catch (int &e) {
	delete [] used;
	used  = 0;
	clear ();
	return e;
      }
      
      // swallow all errors
      catch (...) {
	delete [] used;
	used  = 0;
	clear ();
	return -1;
      }
    }

    int open (const char *file,
	      const char *mode = "rb",
	      size_t offset = 0,
	      size_t _size = 0)
    {
      std::FILE *fp = std::fopen(file, mode);
      if (! fp) return -1;
      if (std::fseek (fp, offset, SEEK_SET) != 0) return -1;

      if (! _size) { 
	if (std::fseek (fp, 0L,     SEEK_END) != 0) return -1;
	_size = std::ftell (fp);
	if (std::fseek (fp, offset, SEEK_SET) != 0) return -1;
      }
      
      clear();

      size = _size;
      size /= sizeof(unit_t);
      array  = new unit_t [size];
      if (size != std::fread ((unit_t *)array,  sizeof(unit_t), size, fp)) return -1;
      std::fclose (fp);

      return 0;
    }

    int save (const char *file,
	      const char *mode = "wb",
	      size_t offset = 0)
    {
      if (! size) return -1;
      std::FILE *fp = std::fopen(file, mode);
      if (! fp) return -1;
      if (size != std::fwrite((unit_t *)array,  sizeof(unit_t), size, fp)) return -1;
      std::fclose (fp);
      return 0;
    }

#ifdef HAVE_ZLIB_H
    int gzopen (const char *file,
    	        const char *mode = "rb",
	        size_t offset = 0,
	        size_t _size = 0)
    {
      std::FILE *fp  = std::fopen (file, mode);
      if (! fp) return -1;
      clear();
      
      size = _size;
      if (! size) {
        if (-1L != (long)std::fseek (fp, (-8), SEEK_END)) {
	  char buf [8];
	  if (std::fread ((char*)buf, 1, 8, fp) != sizeof(buf)) {
	    std::fclose(fp);
	    return -1;
	  }
	  size = LG (buf+4);
	  size /= sizeof (unit_t);
        }
      }
      std::fclose(fp);

      if (! size) return -1;

      zlib::gzFile gzfp = zlib::gzopen(file, mode);
      if (! gzfp) return -1;
      array = new unit_t [size];
      if (zlib::gzseek (gzfp, offset, SEEK_SET) != 0) return -1;
      zlib::gzread (gzfp, (unit_t *)array,  sizeof(unit_t) * size);	
      zlib::gzclose (gzfp);
      return 0;
    }

    int gzsave (const char *file, const char *mode = "wb", size_t offset = 0)
    {
      zlib::gzFile gzfp = zlib::gzopen (file, mode);
      if (!gzfp) return -1;
      zlib::gzwrite (gzfp, (unit_t *)array,  sizeof(unit_t) * size);
      zlib::gzclose (gzfp);
      return 0;
    }
#endif    
    
    value_type exactMatchSearch (const key_type *key,
				  size_t len = 0, 
				  size_t node_pos = 0)
    {
      if (! len) len = LengthFunc() (key);

      register ArrayType  b = array[node_pos].base;
      register ArrayUType p;

      for (register size_t i = 0; i < len; ++i) {
	p = b + (NodeUType)(key[i]) + 1;
	if ((ArrayUType)b == array[p].check) b = array[p].base;
	else return -1;
      }

      p = b;
      ArrayType n = array[p].base;
      if ((ArrayUType)b == array[p].check && n < 0) return -n-1;

      return -1;
    }

    template <class T> size_t commonPrefixSearch (const key_type *key,
						  T* result,
						  size_t result_len,			       
						  size_t len = 0,
						  size_t node_pos = 0)
    {
      if (! len) len = LengthFunc() (key);
      
      register ArrayType  b   = array[node_pos].base;
      register size_t     num = 0;
      register ArrayType  n;
      register ArrayUType p;

      for (register size_t i = 0; i < len; ++i) {
	p = b; // + 0;
	n = array[p].base;
	if ((ArrayUType) b == array[p].check && n < 0) {
	  if (num < result_len) setResult (result[num], -n-1, i); // result[num] = -n-1;
	  ++num;
	}

	p = b + (NodeUType)(key[i]) + 1;
	if ((ArrayUType) b == array[p].check) b = array[p].base;
	else return num;
      }

      p = b;
      n = array[p].base;

      if ((ArrayUType)b == array[p].check && n < 0) {
	if (num < result_len) setResult(result[num], -n-1, len);
	++num;
      }

      return num;
    }

    value_type traverse (const key_type *key, size_t &node_pos, size_t &key_pos, size_t len = 0)
    {
      if (! len) len = LengthFunc() (key);

      register ArrayType  b = array[node_pos].base;
      register ArrayUType p;       

      for (; key_pos < len; ++key_pos) {
        p = b + (NodeUType)(key[key_pos]) + 1;
        if ((ArrayUType)b == array[p].check) { node_pos = p; b = array[p].base; }
        else return -2; // no node
      }

      p = b;
      ArrayType n = array[p].base;
      if ((ArrayUType)b == array[p].check && n < 0) return -n-1;

      return -1; // found, but no value
    }
  };

#if 4 == 2
  typedef Darts::DoubleArrayImpl<char, unsigned char, short, unsigned short> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1  
#endif

#if 4 == 4 && ! defined (DARTS_ARRAY_SIZE_IS_DEFINED)
  typedef Darts::DoubleArrayImpl<char, unsigned char, int, unsigned int> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1
#endif

#if 4 == 4 && ! defined (DARTS_ARRAY_SIZE_IS_DEFINED)
  typedef Darts::DoubleArrayImpl<char, unsigned char, long, unsigned long> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1
#endif

#if 4 == 8 && ! defined (DARTS_ARRAY_SIZE_IS_DEFINED)
  typedef Darts::DoubleArrayImpl<char, unsigned char, long long, unsigned long long> DoubleArray;
#endif
}
#endif

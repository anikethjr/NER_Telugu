/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: svm.cpp,v 1.17 2005/09/05 14:50:59 taku Exp $;

  Copyright (C) 2000-2004 Taku Kudo <taku-ku@is.aist-nara.ac.jp>
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

#include "common.h"
#include "mmap.h"
#include "darts.h"
#include "param.h"
#include "yamcha.h"
#include <cmath>
#include <algorithm>

namespace YamCha 
{
#define  _YAMCHA_INIT_SVM  dot_buf(0), dot_cache(0), result_(0), result(0), model(0), clist(0), \
                          kernel_type (0), solver_type(0), isoov(0), param_degree(0),\
                          msize(0), csize(0), dsize(0), ndsize(0), dasize(0), svsize(0), tsize(0), fsize(0), \
                          sv(0), alpha(0)

  static inline char *read_ptr (char **ptr, size_t size) 
  {
    char *r = *ptr;
    *ptr += size;
    return r;
  }

  template <class T> static inline void read_static (char **ptr, T& value)
  {
    char *r = read_ptr (ptr, sizeof (T));
    memcpy (&value, r, sizeof (T));
  }

  class SVM::Impl
  {
  private:

    enum { PKB, PKI, PKE };

    struct model_t {
      unsigned int pos;
      unsigned int neg;
      double b;
    };

    Param                   param;
    Mmap<char>              mmap;
    Darts::DoubleArray      da;
    Darts::DoubleArray      eda;

    unsigned int*           dot_buf;
    double*                 dot_cache;
    double*                 result_;
    Result*                 result;
    model_t*                model;
    char **                 clist;

    char *                  kernel_type;
    unsigned int            solver_type;
    unsigned int            isoov;
    unsigned int            param_degree;     
    unsigned int            msize;
    unsigned int            csize;
    unsigned int            dsize;
    unsigned int            ndsize;
    unsigned int            dasize;
    unsigned int            svsize;
    unsigned int            tsize;
    unsigned int            fsize;
    int*                    sv;
    int*                    idx;
    double*                 alpha;

    std::string             _what;
    std::string             prof;

    void pki_classify (size_t, char **);
    void pke_classify (size_t, char **);

  public:

    Impl ():  _YAMCHA_INIT_SVM {};

    Impl (const char *filename): _YAMCHA_INIT_SVM 
    {
      if (! open (filename)) throw std::runtime_error (_what);
    }
    ~Impl () { close (); }

    int         getProfileInt    (const char *key) { return param.getProfileInt (key); }
    const char *getProfileString (const char *key)
    {
      prof = param.getProfileString (key);
      return prof.c_str();
    }

    size_t getClassSize     () { return csize; }
    const char** getClassList     () { return (const char**)clist; }
    const char*  what() { return _what.c_str(); }

    bool close ();
    bool open (const char *);
    Result* classify (size_t, char**);
  };

  bool SVM::Impl::close () 
  {
    delete [] model;
    delete [] clist;
    delete [] result_;
    delete [] result;
    delete [] dot_buf;
    delete [] dot_cache;

    dot_buf = 0;
    dot_cache = 0;
    result_ = 0;
    result = 0;
    model = 0;                
    clist = 0;
    solver_type = 0;
    isoov = 0;
    param_degree = 0;     
    msize = 0;
    csize = 0;
    dsize = 0;
    ndsize = 0;
    dasize = 0;
    tsize = 0;
    sv = 0;
    idx = 0;
    alpha = 0;

    return true;
  }

  bool SVM::Impl::open (const char *filename)
  {
    try {
      if (! mmap.open (filename)) throw std::runtime_error (mmap.what());
      char *ptr = mmap.begin ();

      // kernel specfic param.
      char *version = read_ptr (&ptr, 32);

      // check version
      if (std::atof(version) != MODEL_VERSION) 
	 throw std::runtime_error ("invalid model version.\nrecomple model file. e.g.,\n% yamcha-mkmodel foo.txtmodel.gz foo.model\n");

      read_static<unsigned int>(&ptr, solver_type);
      read_static<unsigned int>(&ptr, isoov);

      // kernel
      double param_g, param_r, param_s;
      kernel_type = read_ptr  (&ptr, 32);
      read_static<unsigned int>(&ptr, param_degree);
      read_static<double>      (&ptr, param_g);
      read_static<double>      (&ptr, param_r);
      read_static<double>      (&ptr, param_s);

      // model parameters
      read_static<unsigned int>(&ptr, msize);
      read_static<unsigned int>(&ptr, csize);
      read_static<unsigned int>(&ptr, dsize);
      read_static<unsigned int>(&ptr, ndsize);
      read_static<unsigned int>(&ptr, dasize);
      read_static<unsigned int>(&ptr, svsize);
      read_static<unsigned int>(&ptr, tsize);
      read_static<unsigned int>(&ptr, fsize);
      ptr += sizeof (unsigned int) * 2; // reserved erea

      if (param_degree > 3 && solver_type == PKE) 
	throw std::runtime_error ("param_degree is invalid");

      // read model prameters 
      unsigned int param_size;
      read_static<unsigned int>(&ptr, param_size);
      char *param_str = read_ptr (&ptr, param_size);

      unsigned int pos = 0;
      while (pos < param_size) {
	char *key =  (param_str + pos);
	while (param_str[pos++] != '\0') {};
	char *value = param_str + pos;
	param.setProfile (key, value);
	while (param_str[pos++] != '\0') {};
      }

      // class_, list of fixied record (32)
      result  = new Result [csize];
      clist   = new char * [csize];
      for (size_t i = 0; i < csize; i++) {
	clist[i] = result[i].name = read_ptr(&ptr, 32);
      }

      if (isoov == 1) csize--;

      // result_
      result_ = new double [msize];

      // model
      model = new model_t [msize];
      for (size_t i = 0; i < msize; i++) {
	read_static<unsigned int>(&ptr, model[i].pos);
	read_static<unsigned int>(&ptr, model[i].neg);
	read_static<double>(&ptr, model[i].b);
      }

      // model specfic
      switch (solver_type) {

      case PKI: {

	da.setArray (ptr);  ptr += dasize;
	sv    = (int    *)ptr;  ptr += (sizeof (int)    * tsize);
	idx   = (int    *)ptr;  ptr += (sizeof (int)    * fsize);
	alpha = (double *)ptr;  ptr += (sizeof (double) * fsize);

	dot_buf = new unsigned int [svsize];
	dot_cache = new double [ndsize+1];
	for (size_t i = 0; i <= ndsize; i++) 
	  dot_cache[i] = pow (param_s * i  + param_r, (int)param_degree);
      }
	break;
      
      case PKE: {

	da.setArray  (ptr);     ptr += dasize;
	eda.setArray (ptr);     ptr += tsize;
	idx   = (int *)ptr;     ptr += (sizeof (int)    * fsize);
	alpha = (double *)ptr;  ptr += (sizeof (double) * fsize);
	dot_buf = new unsigned int [ndsize+1];
      }
	break;

      default:
	std::runtime_error ("unknown solover type");
	break;
      }

      if ((unsigned int)(ptr - mmap.begin ()) != mmap.size ()) {
	//	std::cout << (unsigned int)(ptr - mmap.begin ()) << " " <<  mmap.size () << std::endl;
	throw std::runtime_error ("size of model file is invalid.");
      }

      return true;
    }

    catch (std::exception &e) {
      _what = std::string ("SVM::open(): ") + e.what ();
      close ();
      throw std::runtime_error (_what);
      return false;
    }
  }

  void SVM::Impl::pki_classify (size_t argc, char **argv)
  {
    std::fill (dot_buf, dot_buf + svsize, 0);

    for (size_t i = 0; i < argc; ++i) {
      int n = da.exactMatchSearch (argv[i]);
      if (n < 0) continue;
      for (; sv[n] != -1; ++n) ++dot_buf[sv[n]];
    } 

    size_t i = 0;
    for (size_t j = 0 ;; j++) {
      if (idx[j] == -1) {
	if (++i == msize) break;
      } else {
	result_[i] += dot_cache[dot_buf[idx[j]]] * alpha[j];
      }
    } 
    
    return;
  }

  void SVM::Impl::pke_classify (size_t argc, char **argv)
  {
#define CALC_W \
  {  \
     for (int oc = -1; oc < idx[r]; ++r) { \
     result_[idx[r]] += alpha[r]; \
     oc = idx[r]; \
   } }
  
    int r;
    size_t size = 0;

    for (size_t i = 0; i < argc; ++i) {
      if ((r = da.exactMatchSearch (argv[i])) != -1) {
	dot_buf[size] = r;
	size++;
      }
    }

    std::sort (dot_buf, dot_buf + size);

    size_t p = 0;

    switch (param_degree) {

    case 1: {

      for (size_t i1 = 0; i1 < size; ++i1) {
	r = eda.exactMatchSearch ((char *)(dot_buf + i1), 4);
	if (r != -1) CALC_W;
      }
    }
      break;

    case 2: {

      for (size_t i1 = 0; i1 < size; ++i1) {
	size_t pos1 = 0; 
	p = 0;
	r = eda.traverse ((char *)(dot_buf + i1), pos1, p, 4);
	if (r == -2) continue;
	if (r >= 0) CALC_W;
	for (size_t i2 = i1+1; i2 < size; ++i2) {
	  size_t pos2 = pos1;
	  p = 0;
	  r = eda.traverse ((char *)(dot_buf + i2), pos2, p, 4);
	  if (r >= 0) CALC_W;
	}
      }
    }
      break;

    case 3: {

      for (size_t i1 = 0; i1 < size; ++i1) {
	size_t pos1 = 0;  
	p = 0;
	r = eda.traverse ((char *)(dot_buf + i1), pos1, p, 4);
	if (r == -2) continue;
	if (r >= 0) CALC_W;
	for (size_t i2 = i1+1; i2 < size; ++i2) {
	  size_t pos2 = pos1;
	  p = 0;
	  r = eda.traverse ((char *)(dot_buf + i2), pos2, p, 4);
	  if (r == -2) continue;
	  if (r >= 0) CALC_W;
	  for (size_t i3 = i2+1; i3 < size; ++i3) {
	    size_t pos3 = pos2;
	    p = 0;
	    r = eda.traverse ((char *)(dot_buf + i3), pos3, p, 4);
	    if (r >= 0) CALC_W;
	  }
	}
      }
    }
      break;

    default:
      break;
    }

    return;
  }
  
  Result *SVM::Impl::classify (size_t argc, char **argv)
  {
    for (size_t i = 0; i < msize; i++) result_[i] = -model[i].b;
    for (size_t i = 0; i < csize; i++) result[i].dist = result[i].score = 0.0;

    switch (solver_type) {
    case PKI: pki_classify (argc, argv); break;
    case PKE: pke_classify (argc, argv); break;
    default:
      _what =  "SVM::classify(): unknown solver type";
      return 0;
    }

    if (isoov) {
      for (size_t i = 0; i < msize; i++) {
	result[model[i].pos].score = result_[i];
	result[model[i].pos].dist  = result_[i];
      }
    } else {
      for (size_t i = 0; i < msize; i++) {
	result[result_[i] >= 0 ? model[i].pos : model[i].neg].score++;  // score is votes
	result[model[i].pos].dist += result_[i];
	result[model[i].neg].dist -= result_[i];
      }
    }

    return result;
  }

  // wrapper for SVM::Impl
  SVM::SVM  (): _impl(new Impl) {};
  SVM::SVM  (const char* filename): _impl(new Impl (filename)) {};
  SVM::~SVM () { delete _impl; }

  bool         SVM::open             (const char *file) { return _impl->open (file); }
  bool         SVM::close            ()                 { return _impl->close(); }
  Result*      SVM::classify         (size_t argc, char** argv) { return _impl->classify (argc, argv); }
  int          SVM::getProfileInt    (const char *key)  { return _impl->getProfileInt    (key); }
  const char*  SVM::getProfileString (const char *key)  { return _impl->getProfileString (key); }
  const char** SVM::getClassList()                      { return _impl->getClassList (); }
  size_t SVM::getClassSize()                      { return _impl->getClassSize (); }
  const char*  SVM::what()                              { return _impl->what (); }
}

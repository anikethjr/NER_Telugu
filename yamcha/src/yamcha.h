/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: yamcha.h,v 1.22 2005/09/05 14:50:59 taku Exp $;

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

#ifndef _YAMCHA_H
#define _YAMCHA_H

struct _yamcha_result_t {
  char   *name;
  double score; // votes
  double dist;  // distance from the hyperplane
};

/* C interface  */
#ifdef __cplusplus
#include <cstdio>
#include <iosfwd>
#else
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
#  ifdef DLL_EXPORT
#    define YAMCHA_DLL_EXTERN  __declspec(dllexport)
#  else
#    define YAMCHA_DLL_EXTERN  __declspec(dllimport)
#  endif
#endif
   
#ifndef YAMCHA_DLL_EXTERN
#  define YAMCHA_DLL_EXTERN extern
#endif 

  typedef struct _yamcha_t        yamcha_t;
  typedef struct _yamcha_svm_t    yamcha_svm_t;
  typedef struct _yamcha_result_t yamcha_result_t;

  YAMCHA_DLL_EXTERN yamcha_svm_t *   yamcha_svm_new                 (char*);
  YAMCHA_DLL_EXTERN void             yamcha_svm_destroy             (yamcha_svm_t*);
  YAMCHA_DLL_EXTERN char *           yamcha_svm_strerror            (yamcha_svm_t*);
  YAMCHA_DLL_EXTERN size_t     yamcha_svm_get_class_size      (yamcha_svm_t*);
  YAMCHA_DLL_EXTERN char **          yamcha_svm_get_class_list      (yamcha_svm_t*);
  YAMCHA_DLL_EXTERN char *           yamcha_svm_get_profile_string  (yamcha_svm_t*, char *);
  YAMCHA_DLL_EXTERN int              yamcha_svm_get_profile_int     (yamcha_svm_t*, char *);
  YAMCHA_DLL_EXTERN yamcha_result_t* yamcha_svm_classify            (yamcha_svm_t*, int, char **);

  YAMCHA_DLL_EXTERN int              yamcha_do       (int, char **);
  YAMCHA_DLL_EXTERN yamcha_t*        yamcha_new      (int, char **);
  YAMCHA_DLL_EXTERN void             yamcha_destroy  (yamcha_t*);
  YAMCHA_DLL_EXTERN char *           yamcha_strerror (yamcha_t*);
				     
  YAMCHA_DLL_EXTERN char *           yamcha_get_tag         (yamcha_t*, size_t);
  YAMCHA_DLL_EXTERN char *           yamcha_get_context     (yamcha_t*, size_t, size_t);
  YAMCHA_DLL_EXTERN size_t     yamcha_get_class_size  ();
  YAMCHA_DLL_EXTERN char **          yamcha_get_class_list  ();
  YAMCHA_DLL_EXTERN double           yamcha_get_class_score (yamcha_t*, size_t, size_t);
				     
  YAMCHA_DLL_EXTERN int              yamcha_add         (yamcha_t*, int, char **);
  YAMCHA_DLL_EXTERN int              yamcha_add2        (yamcha_t*, char *);
  YAMCHA_DLL_EXTERN size_t     yamcha_get_size    (yamcha_t*);
  YAMCHA_DLL_EXTERN size_t     yamcha_get_row     (yamcha_t*);
  YAMCHA_DLL_EXTERN size_t     yamcha_get_column  (yamcha_t*);
  YAMCHA_DLL_EXTERN int              yamcha_parse       (yamcha_t*);
  YAMCHA_DLL_EXTERN int              yamcha_clear       (yamcha_t*);
				     
  YAMCHA_DLL_EXTERN int              yamcha_add_feature  (yamcha_t*, char *);
  //  YAMCHA_DLL_EXTERN int              yamcha_set_selector (yamcha_t*, int (*) (yamcha_t*, int));
				     
  YAMCHA_DLL_EXTERN char *           yamcha_sparse_tostr  (yamcha_t*, char *);
  YAMCHA_DLL_EXTERN char *           yamcha_sparse_tostr2 (yamcha_t*, char *, size_t);
  YAMCHA_DLL_EXTERN char *           yamcha_sparse_tostr3 (yamcha_t*, char *, size_t, char *, size_t);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace YamCha 
{
  typedef struct _yamcha_result_t Result;

  class SVM
  {
  private:
    class Impl;
    Impl *_impl;

  public:
    SVM  ();
    SVM  (const char *);
    ~SVM ();

    bool              open (const char *);
    bool              close ();
    Result*           classify         (size_t, char**);
    int               getProfileInt    (const char *);
    const char*       getProfileString (const char *) ;
    size_t      getClassSize     ();
    const char**      getClassList     ();
    const char*       what();
  };

  class Chunker
  {
  private:
    class Impl;
    Impl *_impl;

  public:
    bool          open        (int,  char**);
    bool          open        (const char*);
    bool          close       ();
    bool          clear       ();
    size_t  addFeature  (char *);
    bool          setSelector (int (*) (Chunker *, int));
    size_t  add         (size_t, const char **);
    size_t  add         (const char*);

    size_t  size        ();
    size_t  row         ();
    size_t  column      ();
    const char*   getTag      (size_t);
    const char*   getContext  (size_t, size_t);
    size_t  getClassSize     ();
    const char**  getClassList     ();
    double        getClassScore    (size_t, size_t);

    bool          parse       (std::istream &, std::ostream&);
    bool          parse       ();
    int           parse       (int, char**);
    const char*   parse       (const char*, size_t = 0);
    const char*   parse       (const char*, size_t, char *, size_t);
    const char* what();

    Chunker  ();
    Chunker  (int, char**);
    Chunker  (const char*);
    ~Chunker ();
  };
}

#endif

#endif

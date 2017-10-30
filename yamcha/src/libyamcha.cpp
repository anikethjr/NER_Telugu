/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: libyamcha.cpp,v 1.5 2005/09/05 14:50:59 taku Exp $;

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

#include "yamcha.h"
#include <string>
#include <stdexcept>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static std::string errorStr;

struct _yamcha_t {
   int allocated;
   YamCha::Chunker *ptr;
};

struct _yamcha_svm_t {
   int allocated;
   YamCha::SVM *ptr;
};

#ifdef _WIN32
#include <windows.h>
BOOL __stdcall DllMain( HINSTANCE, DWORD dwReason, void* )
{
   return( TRUE );
}
#endif

int yamcha_do (int argc, char **argv)
{
  YamCha::Chunker chunker;
  return chunker.parse (argc, argv);
}

yamcha_svm_t * yamcha_svm_new (char* file)
{
  yamcha_svm_t *c = new yamcha_svm_t;
  YamCha::SVM *ptr = new YamCha::SVM;
  if (! c || ! ptr) {
    errorStr = std::string ("yamcha_new(): bad alloc");
    return 0;
  }
  c->allocated = 0;

  if (! ptr->open (file)) {
    errorStr = std::string ("yamcha_new(): ")  + ptr->what ();
    delete ptr;
    delete c;  
    return 0;
  }

  c->ptr = ptr;
  c->allocated = 1;
  return c;
}

char* yamcha_svm_strerror (yamcha_svm_t *c)
{
   if (! c || ! c->allocated) return const_cast<char *>(errorStr.c_str ());
   return const_cast<char *>(c->ptr->what());
}

void yamcha_svm_destroy (yamcha_svm_t *c)
{
  if (c && c->allocated) {
     delete c->ptr;
     delete c;
  }
  c = 0;
}

#define YAMCHA_SVM_CHECK_FIRST_ARG(a,c,t) \
 if (!(c) || ! (c)->allocated) { \
    errorStr = std::string (a) + ": first argment seems to be invalid"; \
    return 0; \
 } \
 YamCha::SVM *(t) = (c)->ptr;


size_t   yamcha_svm_get_class_size      (yamcha_svm_t* c)
{
  YAMCHA_SVM_CHECK_FIRST_ARG("yamcha_svm_get_class_size",c,t);
  return static_cast<size_t>(t->getClassSize());
}

char**         yamcha_svm_get_class_list      (yamcha_svm_t* c)
{
  YAMCHA_SVM_CHECK_FIRST_ARG("yamcha_svm_get_class_list",c,t);
  return const_cast<char**>(t->getClassList());
}

char *         yamcha_svm_get_profile_string  (yamcha_svm_t* c, char *k)
{
  YAMCHA_SVM_CHECK_FIRST_ARG("yamcha_svm_get_profile_string",c,t);
  return const_cast<char*>(t->getProfileString(k));
}

int            yamcha_svm_get_profile_int     (yamcha_svm_t* c, char *k)
{
  YAMCHA_SVM_CHECK_FIRST_ARG("yamcha_svm_get_profile_int",c,t);
  return static_cast<int>(t->getProfileInt(k));
}

yamcha_result_t * yamcha_svm_classify (yamcha_svm_t* c, int n, char **v)
{
  return static_cast<yamcha_result_t *>(c->ptr->classify (n, v));
}

yamcha_t*      yamcha_new     (int argc, char **argv)
{
  yamcha_t *c = new yamcha_t;
  YamCha::Chunker *ptr = new YamCha::Chunker;
  if (! c || ! ptr) {
    errorStr = std::string ("yamcha_new(): bad alloc");
    return 0;
  }
  c->allocated = 0;

  if (! ptr->open (argc, argv)) {
    errorStr = std::string ("yamcha_new(): ")  + ptr->what ();
    delete ptr;
    delete c; 
    return 0;
  }

  c->ptr = ptr;
  c->allocated = 1;
  return c;
}

yamcha_t*      yamcha_new2     (char *arg)
{
  yamcha_t *c = new yamcha_t;
  YamCha::Chunker *ptr = new YamCha::Chunker;
  if (! c || ! ptr) {
    errorStr = std::string ("yamcha_new2(): bad alloc");
    return 0;
  }
  c->allocated = 0;
  if (! ptr->open (arg)) {
    errorStr = std::string ("yamcha_new2(): ")  + ptr->what ();      
    delete ptr;
    delete c; 
    return 0;
  }

  c->ptr = ptr;
  c->allocated = 1;
  return c;
}

char* yamcha_strerror (yamcha_t *c)
{
   if (! c || ! c->allocated) return const_cast<char *>(errorStr.c_str ());
   return const_cast<char *>(c->ptr->what());
}

void yamcha_destroy (yamcha_t *c)
{
  if (c && c->allocated) {
     delete c->ptr;
     delete c;
  }
  c = 0;
}

#define YAMCHA_CHECK_FIRST_ARG(a,c,t) \
 if (! (c) || ! (c)->allocated) { \
    errorStr = std::string (a) + ": first argment seems to be invalid"; \
    return 0; \
 } \
 YamCha::Chunker *(t) = (c)->ptr;

char *         yamcha_get_tag         (yamcha_t* c, size_t i)
{
  // no check
  return const_cast<char*>(c->ptr->getTag(i));
}

char *         yamcha_get_context     (yamcha_t* c, size_t i, size_t j)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_context",c,t);
  return const_cast<char*>(t->getContext(i,j));
}

size_t   yamcha_get_class_size  (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_class_size",c,t);
  return t->getClassSize();
}

double         yamcha_get_class_score (yamcha_t* c, size_t i, size_t j)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_class_score",c,t);
  return static_cast<double>(t->getClassScore(i,j));
}

int            yamcha_add         (yamcha_t* c, int n, char **v)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_add",c,t);
  return static_cast<int>(t->add(n,const_cast<const char**>(v)));
}

int            yamcha_add2        (yamcha_t* c, char *v)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_add2",c, t);
  return static_cast<int>(t->add(v));
}

size_t   yamcha_get_size    (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_size",c,t);
  return static_cast<size_t>(t->size());
}

size_t   yamcha_get_row     (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_row",c,t);
  return static_cast<size_t>(t->row());
}

size_t   yamcha_get_column  (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_get_column",c,t);
  return static_cast<size_t>(t->column());
}

int            yamcha_parse       (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_parse",c,t);
  return static_cast<int>(t->parse());
}

int            yamcha_clear       (yamcha_t* c)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_clear",c,t);
  return static_cast<int>(t->clear());
}

int            yamcha_add_feature  (yamcha_t* c, char *f)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_add_feature",c,t);
  return static_cast<int>(t->addFeature(f));
}

/* int            yamcha_set_selector (yamcha_t* c, int (*f) (yamcha_t* c, int)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_set_selector",c,t);
  return static_cast<int>(t->setSelector(yamcha_set_selector));
} */

char *         yamcha_sparse_tostr  (yamcha_t* c, char *s)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_sparse_tostr",c,t);
  return const_cast<char*>(t->parse(s));
}

char *         yamcha_sparse_tostr2 (yamcha_t* c, char *s, size_t len)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_sparse_tostr2",c,t);
  return const_cast<char*>(t->parse(s,len));
}

char *         yamcha_sparse_tostr3 (yamcha_t* c, char *s, size_t len, char *out, size_t len2)
{
  YAMCHA_CHECK_FIRST_ARG("yamcha_sparse_tostr3",c,t);
  return const_cast<char*>(t->parse(s,len,out,len2));
}


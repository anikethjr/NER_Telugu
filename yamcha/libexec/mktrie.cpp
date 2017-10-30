/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: mktrie.cpp,v 1.4 2004/09/20 09:59:16 taku-ku Exp $;

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

#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <string>
#include <strstream>
#include <algorithm>
#include <iterator>
#include "darts.h"

using namespace std;

#define MAXLEN 81920

int progress_bar (size_t current, size_t total) 
{
  static char bar[] = "*******************************************";
  static int scale = sizeof(bar) - 1;
  static int prev = 0;

  int cur_percentage  = (int)(100.0 * current/total);
  int bar_len         = (int)(1.0   * current*scale/total);

  if (prev != cur_percentage) {
     fprintf(stderr, "Making TRIE        : %3d%% |%.*s%*s| ", cur_percentage, bar_len, bar, scale - bar_len, "");
     if (cur_percentage == 100)  fprintf(stderr,"\n");
     else                        fprintf(stderr,"\r");
     fflush(stderr);
  }
   
  prev = cur_percentage;

  return 1;
};

template <class T, class Iterator> 
void tokenize (const char *str, Iterator iterator) 
{
  std::istrstream is (str, std::strlen(str));
  std::copy (std::istream_iterator <T> (is), std::istream_iterator <T> (), iterator);
}

struct feature {
  unsigned char *f;  // feature;
  int           id;
  unsigned int len;  // length
};

struct cmp {
  bool operator () (const feature& f1, const feature& f2) 
  {
    unsigned char *p1 = f1.f;
    unsigned char *p2 = f2.f; 
    unsigned int l = std::min (f1.len, f2.len);
    for (unsigned int i = 0; i < l; i++) {
      if ((unsigned int)p1[i] > (unsigned int)p2[i]) return false;
      else if ((unsigned int)p1[i] < (unsigned int)p2[i]) return true;
    }
    return f1.len < f2.len;
  }
};

int main (int argc, char **argv)
{
  if (argc < 3) {
     std::cerr << "Usage " << argv[0] << " File Index\n";
     exit (-1);
  }

  std::string file  = argv[argc-2];
  std::string index = argv[argc-1];
  std::istream *is;
 
  if (file == "-") is = &std::cin;
  else             is = new std::ifstream (file.c_str());
   
  if (! *is) {
     std::cerr << "FATAL: cannot open " << file << std::endl;
     return -1;
  }

  char buf [8192];   
  std::vector<feature>     fv;
  std::vector<std::string> column;
   
  while (is->getline (buf, 8192)) {
    column.clear();
    tokenize<std::string> ((const char*)buf, std::back_inserter(column));
    if (column.empty()) continue;
    unsigned int *tmp = new unsigned int [column.size()-1];
    for (unsigned int i = 0; i < (column.size()-1); i++) 
       tmp[i] = atoi (column[i+1].c_str());
    feature f;
    f.f = (unsigned char *)tmp; // cast
    f.id = atoi (column[0].c_str());
    f.len = 4*(column.size()-1);
    fv.push_back (f);
  }
  
  if (file != "-") delete is; // close;

  std::sort (fv.begin(), fv.end(), cmp());
  size_t *len   = new size_t [fv.size()];
  Darts::DoubleArray::value_type *var = new Darts::DoubleArray::value_type [fv.size()];
  unsigned char **ptr = new unsigned char * [fv.size()];

  for (unsigned int i = 0; i < fv.size(); i++) {
    len[i] = (size_t)fv[i].len;
    ptr[i] = fv[i].f;
    var[i] = fv[i].id;
  }
   
  Darts::DoubleArray da;
  da.build ((size_t)fv.size(), (char **)ptr, len, var, progress_bar);

  delete [] len;
  delete [] var;
  delete [] ptr;

  if (0 != da.save (index.c_str())) {
    std::cerr << "FATAL: cannot save " << argv[2] << std::endl;
    return -1;
  }

  if (file != "-") delete is;
}

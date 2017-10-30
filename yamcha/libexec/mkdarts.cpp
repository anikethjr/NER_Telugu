/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: mkdarts.cpp,v 1.7 2004/03/12 17:12:13 taku-ku Exp $;

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
#include<bits/stdc++.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "darts.h"

using namespace std;

int progress_bar (size_t current, size_t total)
{
  static char bar[] = "*******************************************";
  static int scale = sizeof(bar) - 1;
  static int prev = 0;

  int cur_percentage  = (int)(100.0 * current/total);
  int bar_len         = (int)(1.0   * current*scale/total);

  if (prev != cur_percentage) {
    printf("Making Double-Array: %3d%% |%.*s%*s| ", cur_percentage, bar_len, bar, scale - bar_len, "");
    if (cur_percentage == 100)  printf("\n");
    else                        printf("\r");
    fflush(stdout);
  }
   
  prev = cur_percentage;

  return 1;
};

int main (int argc, char **argv)
{
  using namespace std;
   
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " File Index" << std::endl;
    return 0; 
  }

  std::string file  = argv[argc-2];
  std::string index = argv[argc-1];
  std::istream *is;
 
  if (file == "-") is = &std::cin;
  else             is = new std::ifstream (file.c_str());

  if (! *is) {
    std::cerr << "Cannot open: " << file << std::endl;
    return -1; 
  }

  char buf[8192];
  std::vector <char *> str;
  std::vector <int> id;
  
  while (is->getline (buf, 8192)) {
     unsigned int p = 0;
     while (isspace(buf[p])) { ++p; }
     int i = std::atoi (&buf[p]);
     while (! isspace(buf[p])) { ++p; }
     while (isspace(buf[p])) { ++p; }
     char *tmp = new char [strlen (buf+p) + 1];
     strcpy (tmp, buf+p);
     str.push_back (tmp);
     id.push_back (i);
  }

  Darts::DoubleArray da;
  if (da.build (str.size(), &str[0], 0, &id[0], &progress_bar) < 0) {
    std::cerr << "FATAL: cannot build Double-Array" << std::endl;
    return -1; 
  }
   
  if (da.save  (index.c_str()) == -1) {
    std::cerr << "FATAL: cannot open: " << index << std::endl;
    return -1; 
  }
   
  for (unsigned int i = 0; i < str.size(); i++) delete [] str[i];
  if (file != "-") delete is;
   
  return 0; 
}

/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: param.h,v 1.19 2004/03/08 07:33:54 taku-ku Exp $;

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

#ifndef _MECAB_PARAM_H
#define _MECAB_PARAM_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace YamCha {
   
  struct Option 
  {
    const char *name;
    char        short_name;
    const char *default_value;
    const char *arg_description;
    const char *description;
  };

  class Param 
  {
  private:
    std::map    <std::string, std::string> conf;
    std::vector <std::string> rest;
    std::string systemName;
    std::string _what;

  public:
    bool open    (int,  char**, const Option *);
    bool open    (const char*,  const Option *);
    bool load    (const char*);
    void clear   (); 
    const std::vector <std::string>& getRestArg () { return rest; };

    const char* getSystemName () { return systemName.c_str(); };
    const char* what ()          { return _what.c_str(); };

    void help    (std::ostream&, const Option *);
    void version (std::ostream&, const Option *);

    const std::string getProfileString (const char*, bool = false);
    int               getProfileInt    (const char*, bool = false);
    void              setProfile       (const char*, const char* value, bool = true);
    void              setProfile       (const char*, int, bool = true);
   
    Param ()  {};
    ~Param () {};
  };
}

#endif


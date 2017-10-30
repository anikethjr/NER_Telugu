/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: feature_index.h,v 1.8 2005/09/05 14:50:59 taku Exp $;

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
#ifndef _YAMCHA_FEATUREINDEX_H
#define _YAMCHA_FEATUREINDEX_H

#include <vector>
#include <string>

namespace YamCha 
{
  class FeatureIndex
  {
  public:
    bool setFeature (const std::string&, const std::string&, const std::string&);
    bool setFeature (const std::string&, int);
    size_t getColumnSize ();
    std::vector < std::pair <int, int> > features;
    std::vector < std::pair <int, int> > bow_features;
    std::vector <int> tags;
  };
}

#endif



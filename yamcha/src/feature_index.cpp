/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: feature_index.cpp,v 1.7 2005/09/05 14:50:59 taku Exp $;

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
#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <set>
#include "feature_index.h"
#include "common.h"

namespace YamCha 
{
  static inline bool parse_start_end (const std::string &src, 
				      int &start, 
				      int &end, 
				      int end_default)
  {
    char *ptr = (char *)src.c_str();

    start = str2int (ptr);

    size_t pos = src.find ("..");
    if (pos == std::string::npos) { // not found ..
      end = start;
      return true;
    } else if (pos == (src.size()-2)) { // foud but .. style
      end = end_default;
      if (end < start) return false;
      return true;
    } else {
      ptr += (pos + 2);
      end = str2int (ptr);
      if (end < start) return false; 
    }
     
    return true;
  }

  size_t FeatureIndex::getColumnSize () 
  {
    size_t c = 0;
    for (size_t i = 0; i < features.size(); i++) 
      c = _max (features[i].second + 1, static_cast<int>(c));
    return c;
  }

  bool FeatureIndex::setFeature(const std::string &feature, 
				const std::string &bow_feature,
				const std::string &tag)
  {
    try {

      features.clear();
      tags.clear();

      std::vector <std::string> tmp;

      tokenize(feature, "\t ", tmp);
      for (size_t i = 0; i < tmp.size(); i++) {
	std::vector <std::string> tmp2;
	if (tokenize(tmp[i],":",tmp2) != 2) throw std::runtime_error (feature);
	features.push_back(std::make_pair<int,int>(str2int (tmp2[0].c_str()), str2int (tmp2[1].c_str())));
      }

      tokenize(bow_feature, "\t ", tmp);
      for (size_t i = 0; i < tmp.size(); i++) {
	std::vector <std::string> tmp2;
	if (tokenize(tmp[i],":",tmp2) != 2) throw std::runtime_error (feature);
	bow_features.push_back(std::make_pair<int,int>(str2int (tmp2[0].c_str()), str2int (tmp2[1].c_str())));
      }

      tokenize (tag, "\t ", tmp);
      for (size_t i = 0; i < tmp.size(); i++) {
	int row = str2int (tmp[i].c_str());
	tags.push_back(row);
      }

      return true;
    }

    catch (std::exception &e) {
      throw std::runtime_error (std::string ("FeatureIndex::setFeature() format error: ") + e.what ());
      return false;
    }
  }

  bool FeatureIndex::setFeature(const std::string& str, int max_col)
  {
    try {
       
      std::vector <std::string> list, rclist, item;
      int start, end;

      std::set <std::pair <int, int> > feature_set;
      std::set <std::pair <int, int> > bow_feature_set;       
      std::set <int> tag_set;

      features.clear();
      tags.clear();

      if (! tokenize (str,"\t ",list)) throw std::runtime_error (str);

      for (std::vector<string>::iterator lit  = list.begin(); lit != list.end(); ++lit) {

	int size = tokenize(*lit, ":", rclist);

	if (size == 3 && (rclist[0] == "F" || rclist[0] == "f" || rclist[0] == "B" || rclist[0] == "b")) {

	  std::set <int> tmp_row, tmp_col;

	  if (! tokenize (rclist[1], ",", item)) throw std::runtime_error (*lit);

	  for (std::vector<std::string>::iterator it = item.begin(); it != item.end(); ++it) {
	    if (! parse_start_end (*it, start, end, 0)) throw std::runtime_error (*lit);
	    for (int j = start; j <= end; j++) tmp_row.insert (j);
	  }

	  if (! tokenize(rclist[2], ",", item)) throw std::runtime_error (*lit);

	  for (std::vector<std::string>::iterator it = item.begin(); it != item.end(); ++it) {
	    if (! parse_start_end (*it, start, end, max_col - 1) || start < 0 || max_col - 1 < end) 
	      throw std::runtime_error (*lit);
	    for (int j = start; j <= end; j++) tmp_col.insert (j);
	  }

	  if (rclist[0] == "F" || rclist[0] == "f") {
	     for (std::set<int>::iterator rit = tmp_row.begin(); rit != tmp_row.end(); ++rit) 
	       for (std::set<int>::iterator cit = tmp_col.begin(); cit != tmp_col.end(); ++cit)
		 feature_set.insert (std::make_pair <int, int>( *rit, *cit ) );
	  } else {
	     for (std::set<int>::iterator rit = tmp_row.begin(); rit != tmp_row.end(); ++rit) 
	       for (std::set<int>::iterator cit = tmp_col.begin(); cit != tmp_col.end(); ++cit)
		 bow_feature_set.insert (std::make_pair <int, int>( *rit, *cit ) );
	  }

	} else if (size == 2 && (rclist[0] == "T" || rclist[0] == "t")) {
	  if (! tokenize (rclist[1],",",item)) throw std::runtime_error (*lit);
	  for (std::vector<std::string>::iterator it = item.begin(); it != item.end(); ++it) {
	    if (! parse_start_end (*it, start, end, 0) || start * end <= 0 ) 
	      throw std::runtime_error (*lit);
	    for (int j = start; j <= end; j++) tag_set.insert (j);
	  }
	} else if (*lit == " " || *lit == "") {
	  // do nothing
	} else {
	  throw std::runtime_error (*lit);
	}
      }

      /*      for (std::set<std::pair<int, int> >::iterator it = feature_set.begin(); it != feature_set.end(); ++it) 
	if (bow_features.find (*it) !=  bow_features.end()) throw std::runtime_error ("duplicated feature templates are found"); 

      for (std::set<std::pair<int, int> >::iterator it = bow_feature_set.begin(); it != bow_feature_set.end(); ++it) 
      if (features.find (*it) !=  features.end()) throw std::runtime_error ("duplicated feature templates are found");  */

      for (std::set<std::pair<int, int> >::iterator it = feature_set.begin(); it != feature_set.end(); ++it) 
	features.push_back (*it);
       
      for (std::set<std::pair<int, int> >::iterator it = bow_feature_set.begin(); it != bow_feature_set.end(); ++it) 
	bow_features.push_back (*it);

      for (std::set<int>::iterator it = tag_set.begin(); it != tag_set.end(); ++it) 
	tags.push_back(*it);
    }

    catch (std::exception &e) {
      throw std::runtime_error (std::string ("FeatureIndex::setFeature() format error: ") + e.what ());
      return false;
    }

    return true;
  }
}

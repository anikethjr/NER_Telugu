/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: chunkersub.h,v 1.10 2005/09/05 14:50:59 taku Exp $;

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

namespace YamCha {

#ifdef _YAMCHA_PARSE_DETAIL
  bool Chunker::Impl::parseDetail ()
#else 
  bool Chunker::Impl::parseNormal ()
#endif
  {

    reverse ();

#ifdef _YAMCHA_PARSE_DETAIL
    dist.resize (size());
#endif

    for (size_t i = 0; i < size(); i++) {

      size_t size = select (i); // feature selection
      Result *result = svm.classify (size, features);

      double       max_score = - 1e+36;
      size_t max_id = 0;

      if (is_partial && (context[i].size() - column_size) >= 1) { 

	std::map <std::string, bool> c;
	for (size_t j = column_size; j < context[i].size(); j++) 
	  c[context[i][j]] = true;

	for (size_t n = 0; n < class_size; n++) {

#ifdef _YAMCHA_PARSE_DETAIL
	  dist[i].push_back (std::make_pair <char*, double>
			       (result[n].name, result[n].dist));
#endif

	  if (! c[result[n].name]) continue;

	  if (max_score < result[n].score) {
	    max_score = result[n].score;
	    max_id = n; 
	  }
	}

      } else {

	for (size_t n = 0; n < class_size; n++) {

#ifdef _YAMCHA_PARSE_DETAIL
	  dist[i].push_back (std::make_pair <char*, double>
			     (result[n].name, result[n].dist));
#endif
	  
	  if (max_score < result[n].score) { 
	    max_score = result[n].score;
	    max_id = n; 
	  }
	}
      }

      tag.push_back (result[max_id].name);
    }

    reverse ();

    return true;
  }

#ifdef _YAMCHA_PARSE_DETAIL
  std::ostream& Chunker::Impl::writeDetail (std::ostream &os)
#else 
  std::ostream& Chunker::Impl::writeNormal (std::ostream &os)
#endif
  {
    for (size_t i = 0; i < size(); i++) {

      size_t c = is_partial ? column_size: context[i].size();

      for (size_t j = 0; j < c; j++) os << context[i][j] << '\t';

      os << tag[i];

#ifdef _YAMCHA_PARSE_DETAIL
      for (size_t j = 0; j < class_size; j++) 
	os << '\t' << dist[i][j].first << '/' << dist[i][j].second;
#endif
      os << '\n';
    }

    os << eos_string << std::endl;

    return os;
  }
}


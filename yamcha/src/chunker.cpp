/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: chunker.cpp,v 1.23 2005/09/05 14:50:59 taku Exp $;

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
#include <stdexcept>
#include <string>
#include <strstream>
#include <map>
#include <algorithm>
#include <functional>
#include <fstream>
#include <string.h>
#include "feature_index.h"
#include "param.h"
#include "yamcha.h"
#include "common.h"

using namespace std;

namespace YamCha 
{
#define _INIT_CHUNKER is_reverse (0), is_write_header (0), \
                      is_partial (0), is_verbose(0), mode(0), \
                      column_size(0), class_size(0), features (0), \
                      features_size(0), selector_func (0), ostrs(0)

#define CHUNKER_ERROR  std::ostrstream os; \
                      os << "Tagger::open(): " << param.what () << "\n\n" \
                         <<  COPYRIGHT << "\ntry '--help' for more information.\n" << std::ends; \
                      _what = os.str(); os.freeze (false);

  static const Option long_options[] = 
  {
    {"model",          'm', 0, "FILE", "use FILE as model file" },
    {"feature",        'F', 0, "PAT",  "use PAT as the feature template pattern"},
    {"eos-string" ,    'e', 0, "STR",  "use STR as sentence-boundary marker" },
    {"verbose",        'V', 0, 0,      "verbose mode" },     
    {"candidate",      'C', 0, 0,      "partial chunking model"} ,
    {"backward",       'B', 0, 0,      "select features from the end of sentence" },
    {"output",         'o', 0, "FILE", "use FILE as output file" },     
    {"version",        'v', 0, 0,      "show the version and exit" },
    {"help",           'h', 0, 0,      "show this help and exit" },
    {0,0,0,0,0}
  };

  class Chunker::Impl
  {
  private:

    FeatureIndex feature_index;
    SVM           svm;
    bool          is_reverse;
    bool          is_write_header;
    bool          is_partial;
    bool          is_verbose;
    int           mode;
    size_t  column_size;
    size_t  class_size;
    char**        features;
    size_t  features_size;
    int           (*selector_func) (Chunker *, int i);
    std::ostrstream *ostrs;
    std::string   eos_string;
    std::string   feature;
    std::vector  < std::vector <std::string> > context;
    std::vector  <std::string> tag;
    std::vector  <std::string> bos;
    std::vector  <std::string> eos;
    std::vector  < std::vector <std::pair <char*, double> > > dist;
    std::string   _what;

    std::string&  getFeature     (int, int);
    void          reverse        ();
    size_t  select         (int);
    bool          parseDetail    ();
    bool          parseNormal    ();
    bool          parseSelect    ();

    bool          open           (Param &);
    std::ostream& writeDetail    (std::ostream&);
    std::ostream& writeNormal    (std::ostream&);
    std::ostream& writeSelect    (std::ostream&);
    std::ostream& write          (std::ostream &) ;
    std::istream& read           (std::istream &);

  public:

    Chunker       *self;

    bool          open        (int,  char**);
    bool          open        (const char*);
    bool          close       ();
    bool          clear       ();
    size_t  addFeature  (char *);
    bool          setSelector (int (*) (Chunker *, int i));
    const char*   getTag      (size_t i)                 { return tag[i].c_str(); }
    const char*   getContext  (size_t i, size_t j) { return context[i][j].c_str(); };
    size_t  getClassSize () { return svm.getClassSize (); }
    const char ** getClassList () { return svm.getClassList (); }
    double        getClassScore (size_t i, size_t j) { return dist.empty() ? 0.0 : dist[i][j].second; };
    size_t  add         (size_t, const char **);
    size_t  add         (const char*);
    size_t  size        () { return context.size (); }
    size_t  row         () { return context.size (); }
    size_t  column      () { return column_size;     }
    bool          parse       (std::istream&, std::ostream&);     
    bool          parse       ();
    int           parse       (int, char**);
    const char*   parse       (const char*, size_t = 0);
    const char*   parse       (const char*, size_t, char *, size_t);

    const char* what() { return _what.c_str(); }

    Impl(): _INIT_CHUNKER {};
    Impl (int argc, char** argv): _INIT_CHUNKER 
    { 
      if (! open (argc, argv)) throw std::runtime_error (_what);
    }

    Impl (const char* arg): _INIT_CHUNKER 
    { 
      if (! open (arg)) throw std::runtime_error (_what);
    }

    ~Impl () 
    {
      close (); 
      if (ostrs) { ostrs->freeze (false); delete ostrs; }
    }
  };

  bool Chunker::Impl::open (int argc, char **argv)
  {
    Param param;

    if (! param.open (argc, argv, long_options)) {
      CHUNKER_ERROR;
      return false;
    }

    return open (param);
  }

  bool Chunker::Impl::open (const char *arg)
  {
    Param param;

    if (! param.open (arg, long_options)) {
      CHUNKER_ERROR;
      return false;
    }

    return open (param);
  }

  bool Chunker::Impl::open (Param &param)
  {
    try {

      if (param.getProfileInt ("help")) {
        std::ostrstream ostrs;
        param.help (ostrs, long_options);
	ostrs << std::ends; 
        std::runtime_error e (ostrs.str());
	ostrs.freeze (false);
	throw e; 
      }

      if (param.getProfileInt ("version")) {
        std::ostrstream ostrs;
        param.version (ostrs, long_options);
	ostrs << std::ends; 	 
        std::runtime_error e (ostrs.str());
	ostrs.freeze (false);
	throw e; 
      }

      close ();

      feature           = param.getProfileString ("feature");
      is_partial        = param.getProfileInt    ("candidate");
      is_verbose        = param.getProfileInt    ("verbose");
      eos_string        = param.getProfileString ("eos-string");
      std::string model = param.getProfileString ("model");

      if (! model.empty()) {

	mode = 0;
	
	if (! svm.open (model.c_str())) throw std::runtime_error (svm.what());
	
	feature_index.setFeature (svm.getProfileString ("features"), 
				  svm.getProfileString ("bow_features"),
				  svm.getProfileString ("tag_features"));

	column_size = svm.getProfileInt ("column_size");
	if (column_size == 0) column_size = feature_index.getColumnSize ();
	if (column_size == 0) throw std::runtime_error (std::string ("column size is 0 or unknown: ") + model);

	const char *tmp = svm.getProfileString("parsing_direction");
	if (strcmp (tmp, "backward") == 0) is_reverse = true;

	class_size = svm.getClassSize ();

      } else if (! feature.empty()) {
	
	mode       = 1;
	is_reverse = param.getProfileInt ("backward");
	
      } else {
	throw std::runtime_error ("unknown action mode");
      }

      features = new char * [MAX_FEATURE_LEN];
      for (size_t i = 0; i < MAX_FEATURE_LEN; i++) 
	features[i] = new char [MAX_STR_LEN];

      return true;
    }

    catch (std::exception &e) {
      _what = std::string ("Chunker::open(): ") + e.what ();
      throw std::runtime_error (_what);
      return false;
    }
  }

  int Chunker::Impl::parse (int argc, char **argv)
  {
    try {
   
      Param param;
    
      if (! param.open (argc, argv, long_options)) {
	CHUNKER_ERROR;
	throw std::runtime_error (_what);
      }

      if (param.getProfileInt ("help")) {
	param.help (std::cout, long_options);
	return EXIT_SUCCESS;
      }

      if (param.getProfileInt ("version")) {
	param.version (std::cout, long_options);
	return EXIT_SUCCESS;
      }

      if (! open (param)) throw std::runtime_error (_what);

      std::ostream *ofs = &std::cout;
      std::string outputFileName = param.getProfileString ("output");

      if (! outputFileName.empty()) {
	ofs = new std::ofstream (outputFileName.c_str());
	if (! *ofs) throw std::runtime_error (outputFileName + ", no such file or directory");
      }
     
      const std::vector <std::string>& rest = param.getRestArg ();
     
      if (rest.size()) {
	for (size_t i = 0; i < rest.size(); i++) {
	  std::ifstream ifs (rest[i].c_str ());
	  if (!ifs) throw std::runtime_error (rest[i] + ", no such file or directory");
	  while (parse (ifs, *ofs)) {};
	}
      } else {
	while (parse (std::cin, *ofs)) {};
      }
  
      if (ofs != &std::cout) delete ofs;

      return EXIT_SUCCESS;
    }

    catch (std::exception &e) {
      std::cerr << "FATAL: " << e.what () << std::endl;
      return EXIT_FAILURE;
    }
  }

  bool Chunker::Impl::close () 
  {
    if (features) {
      for (size_t i = 0; i < MAX_FEATURE_LEN; i++) delete [] features[i];
      delete [] features;
    }
    features = 0;
    features_size = 0;

    is_reverse      = false;
    is_write_header = false;
    is_partial      = false;
    is_verbose      = false;
    mode            = 0;
    selector_func   = 0;
    class_size      = 0;

    clear ();

    return true;
  }

  bool Chunker::Impl::clear ()
  {
    tag.clear();
    context.clear();
    dist.clear ();
    features_size = 0;
    return true;
  }

  std::string& Chunker::Impl::getFeature(int i, int j)
  {
    if (i < 0) {

      for (int k = - static_cast<int>(bos.size())-1; k >= i; k--) {
	char buf [32];
	std::ostrstream os (buf, 32);
	os << k << "__BOS__" << std::ends;
	bos.push_back(std::string(buf));
      }

      return bos[-i-1];

    } else if (i >= static_cast<int>(context.size())) {

      for (int k = 1 + eos.size(); k <= (i - static_cast<int>(context.size()) + 1); k++) {
	char buf [32];
	std::ostrstream os (buf, 32);
	os << '+' << k << "__EOS__" << std::ends;
	eos.push_back (std::string(buf));
      }

      return eos[i-context.size()];

    } else {

      return context[i][j];

    }
  }

  size_t Chunker::Impl::select (int i)
  {
    features_size = 0;
    if (selector_func) (*selector_func) (self, i);

    size_t l = features_size;

    for (size_t j = 0; j < feature_index.features.size(); j++) {
      std::ostrstream os (features[l], MAX_STR_LEN);
      os << "F:";
      if (feature_index.features[j].first >= 0) os << '+';
      os << feature_index.features[j].first
	 << ':'  << feature_index.features[j].second 
	 << ':'  <<  getFeature (i + feature_index.features[j].first, 
				 feature_index.features[j].second) << std::ends;
      l++;
    }

    for (size_t j = 0; j < feature_index.bow_features.size(); j++) {
      std::vector <std::string> tmp;
      std::string s = getFeature (i + feature_index.bow_features[j].first,
				  feature_index.bow_features[j].second);
      tokenize (s, ",", tmp);
      for (size_t k = 0; k < tmp.size(); ++k) {
	 std::ostrstream os (features[l], MAX_STR_LEN);
	 os << "B:";
	 if (feature_index.bow_features[j].first >= 0) os << '+';
	 os << feature_index.bow_features[j].first
	   << ':'  << feature_index.bow_features[j].second 
	   << ':'  << tmp[k] << std::ends;
	 l++;
      }
    }

    for (size_t j = 0; j < feature_index.tags.size(); j++) {
      int k = i + feature_index.tags[j];
      if (k >= 0) {
	std::ostrstream os (features[l], MAX_STR_LEN);
	os << "T:" << feature_index.tags[j] << ':' << tag[k] << std::ends;
	l++;
      }
    }

    return l;
  }

  void Chunker::Impl::reverse()
  {
    if (! is_reverse) return;
    std::reverse (context.begin(), context.end());
    std::reverse (tag.begin(),     tag.end());
    std::reverse (dist.begin(),    dist.end());
  }

  bool Chunker::Impl::setSelector (int (*func)(Chunker *, int))
  {
    selector_func  = func;
    return true;
  }

  size_t Chunker::Impl::addFeature (char *s)
  {
    strncpy (features[features_size], s, MAX_STR_LEN);
    features_size++;
    return features_size;
  }

  size_t Chunker::Impl::add (size_t argc, const char **argv)
  {
    std::vector <std::string> column;
    for (size_t i = 0; i < argc; ++i) column.push_back (argv[i]);
    context.push_back (column);
    return context.size ();
  }

  size_t Chunker::Impl::add (const char *line)
  {
    std::vector <std::string> column;
    size_t s = tokenize (line, "\t ", column);
    if (column_size == 0) column_size = s;
    for (; s < column_size; s++) column.push_back ("");
    context.push_back (column);
    return context.size ();
  }

  std::istream& Chunker::Impl::read (std::istream &is)
  {
    try {

      clear();

      std::string line;

      for (;;) {

	if (! std::getline (is, line)) {
	  is.clear (std::ios::eofbit|std::ios::badbit);
	  return is;
	}

	if (line == "\t" || line == "" || line == "EOS") break;
	add (line.c_str());
      }

      return is;
    }

    catch (std::exception &e) {
      _what = std::string ("Chunker::read(): ") + e.what ();
      is.clear (std::ios::eofbit|std::ios::badbit);
      return is;
    }
  }

  std::ostream& Chunker::Impl::write (std::ostream &os)
  {
    try {
      switch (mode) {
      case 0: return is_verbose ? writeDetail (os) : writeNormal (os);
      case 1: return writeSelect (os);
      }
      return os;
    }

    catch (std::exception &e) {
      _what = std::string ("Chunker::write(): ") + e.what ();
      os.clear (std::ios::eofbit|std::ios::badbit);
      return os;
    }
  }

  const char *Chunker::Impl::parse (const char *str, size_t len)
  {
    if (!str) {
      _what = "Parser::parse(): NULL string is given";
      return 0;
    }

    if (! ostrs) ostrs = new std::ostrstream ();
    else { ostrs->freeze (false); ostrs->seekp (0, ios::beg); }

    std::istrstream is (str, len ? len : strlen (str));

    if (! parse (is, *ostrs)) return 0;
    *ostrs << std::ends;
    return ostrs->str();
  }

  const char *Chunker::Impl::parse (const char *str, size_t len, char *out, size_t len2)
  {
    if (!str) {
      _what = "Parser::parse(): NULL string is given";
      return 0;
    }

    std::ostrstream os (out, len2);
    std::istrstream is (str, len ? len : strlen (str));
    if (! parse (is, os)) return 0;
    os << std::ends;
    return out;
  }

  bool Chunker::Impl::parse (std::istream &is, std::ostream &os)
  {
    if (! read (is)) return false;
    if (! parse())   return false;
    write (os);
    return true;
  }

  bool Chunker::Impl::parse ()
  {
    try {

      switch (mode) {
      case 0: return  is_verbose ? parseDetail () : parseNormal ();
      case 1: return parseSelect ();
      }

      return true;
    }

    catch (std::exception &e) {
      _what = std::string ("Chunker::parse(): ") + e.what ();
      throw std::runtime_error (_what);
      return false;
    }
  }

  bool Chunker::Impl::parseSelect ()
  {
    if (column_size <= 1) 
      throw std::runtime_error ("answer tags are not defined");

    for (size_t i = 0; i < size(); i++) 
      tag.push_back (context[i][column_size-1]); // push last column

    reverse ();

    return true;
  }

  std::ostream& Chunker::Impl::writeSelect (std::ostream &os) 
  {
    if (! is_write_header) {

      if (column_size <= 1) 
	throw std::runtime_error ("answer tags are not defined");

      feature_index.setFeature (feature, column_size-1);

      os << "Version: "           << VERSION << std::endl;
      os << "Package: "           << PACKAGE << std::endl;
      os << "Parsing_Direction: " << (is_reverse ? "backward" : "forward") << std::endl;
      os << "Feature_Parameter: " << feature << std::endl;
      os << "Column_Size: "       << column_size-1 << std::endl; // NOTE: must -1; last colum is ANSWER

      os << "Tag_Features:";
      for (size_t i = 0; i < feature_index.tags.size(); i++) 
	os << ' ' << feature_index.tags[i];
      os << std::endl;

      os << "Features:";
      for (size_t i = 0; i < feature_index.features.size(); i++) 
	os << ' ' << feature_index.features[i].first << ":" << feature_index.features[i].second;
      os << std::endl;

      os << "BOW_Features:";
      for (size_t i = 0; i < feature_index.bow_features.size(); i++) 
	os << ' ' << feature_index.bow_features[i].first << ":" << feature_index.bow_features[i].second;
      os << std::endl << std::endl;

      is_write_header = true;
    }

     for (size_t i = 0; i < size(); i++) {
      os << tag[i];
      size_t size = select (i);
      for (size_t j = 0; j < size; j++) os << ' ' << features[j];
      os << std::endl;
    }

    os << std::endl;

    return os;
  }
}

#define _YAMCHA_PARSE_DETAIL
#include "chunkersub.h"
#undef _YAMCHA_PARSE_DETAIL
#include "chunkersub.h"

namespace YamCha {

  Chunker::Chunker  (): _impl(new Impl()) { _impl->self = this; };
  Chunker::Chunker  (int argc, char** argv) : _impl(new Impl(argc, argv)) { _impl->self = this; };
  Chunker::Chunker  (const char* argv):       _impl(new Impl (argv))      { _impl->self = this; };
  Chunker::~Chunker () { delete _impl; }

  bool          Chunker::open        (int argc,  char** argv)         { return _impl->open(argc, argv); };
  bool          Chunker::open        (const char *argv)               { return _impl->open(argv); };
  bool          Chunker::close       ()                               { return _impl->close(); };
  bool          Chunker::clear       ()                               { return _impl->clear(); };
  size_t  Chunker::addFeature  (char *s)                        { return _impl->addFeature(s); };
  bool          Chunker::setSelector (int (*func) (Chunker*, int))    { return _impl->setSelector(func); };
  const char*   Chunker::getTag      (size_t i)                 { return _impl->getTag(i); };
  const char*   Chunker::getContext  (size_t i, size_t j) { return _impl->getContext(i,j); };
  size_t  Chunker::add         (size_t argc, const char **argv) { return _impl->add(argc, argv); };
  size_t  Chunker::getClassSize  ()                             { return _impl->getClassSize(); };
  const char**  Chunker::getClassList  ()                             { return _impl->getClassList(); };
  double        Chunker::getClassScore (size_t i, size_t j) { return _impl->getClassScore(i, j); };
  size_t  Chunker::add         (const char*  s)                 { return _impl->add(s); };
  size_t  Chunker::size        ()                               { return _impl->size(); };
  size_t  Chunker::row         ()                               { return _impl->row(); };
  size_t  Chunker::column      ()                               { return _impl->column(); };
  bool          Chunker::parse       (std::istream &is, std::ostream &os) { return _impl->parse(is, os); };
  bool          Chunker::parse       ()                               { return _impl->parse(); };
  int           Chunker::parse       (int argc, char** argv)          { return _impl->parse(argc, argv); };
  const char*   Chunker::parse       (const char *s, size_t l)  { return _impl->parse(s, l); };
  const char*   Chunker::parse       (const char *s, size_t l, char *s2, size_t l2)  { return _impl->parse(s, l, s2, l2); };
  const char*   Chunker::what        ()                               { return _impl->what(); };
}

 

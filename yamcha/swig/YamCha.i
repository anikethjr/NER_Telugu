%module YamCha
%include exception.i
%include arg.i

%{
#include <stdexcept>
#include "yamcha.h"

namespace SWIG_YamCha {
    
  class Chunker
  {
  private:
    yamcha_t *c;

  public:
    Chunker (int argc, char *argv[]): c(0)
    {
      if (! (c = yamcha_new (argc, argv))) throw yamcha_strerror (c);
    }

    ~Chunker ()
    {
      yamcha_destroy (c); c = 0;
    }

    char *parse (char *p)
    {
      return parseToString (p);
    }

    char *parseToString (char *p)
    {
      char *r = yamcha_sparse_tostr (c, p);
      if (!r) throw yamcha_strerror (c);
      return r;
    }
  };
}
%}

%exception {
  try { $action }
  catch (std::exception &e) { SWIG_exception (SWIG_RuntimeError, (char*)e.what()); }
  catch (char *e) { SWIG_exception (SWIG_RuntimeError, e); }
  catch (const char *e) { SWIG_exception (SWIG_RuntimeError, (char*)e); }
}

namespace SWIG_YamCha {
  class Chunker 
  {
    public:
    Chunker (int argc, char *argv[]);
    ~Chunker ();
    char *parse (char *);
    char *parseToString (char *);
  };
}


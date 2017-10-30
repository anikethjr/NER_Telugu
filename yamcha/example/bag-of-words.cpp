#include <yamcha.h>
#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// $Id: bag-of-words.cpp,v 1.3 2002/10/21 12:03:06 taku-ku Exp $;

/*
 bag-of-words of define call-back function

 compile: 
  c++ `yamcha-config --cflags` bag-of-words.cpp -o bag-of-words `yamcha-config --libs` 

 How to run:
 - Training: use YAMCHA parameter, e.g.,

    make CORPUS=foo MODEL=bar YAMCHA=/somewhere/bag-of-words FEATURE="T:-2..-1" train

 - Testing: same as yamcha
   /somewhere/bag-of-words -m MODEL < test
*/


/*
 * Add bag-of-words features
 * 
ordered                           complied             AFTER
ordered                           called               AFTER
called                            said                 SIMULTANEOUS
in,Iraq,and,Kuwait                in,both,countries    DURING
increase                          number               BEGUN_BY
under,international,quarantine    power-grab           BEGUN_BY
power-grab                        eternal,merger       IDENTITY
attack                            Replied              AFTER
*/


template <class Iterator>
  static inline unsigned int tokenize (char *str, char *del, Iterator out, unsigned int max)
{
   char *stre = str + strlen (str);
   char *dele = del + strlen (del);
   unsigned int size = 0;
   
   while (size < max) {
      char *n = std::find_first_of (str, stre, del, dele);
      *n = '\0';
      *out++ = str;
      ++size;      
      if (n == stre) break;
      str = n + 1;
    }
    
   for (unsigned int i = size; i < max; ++i) *out++ = "";
   return size;
}

int addBOW (YamCha::Chunker *chunker, int pos)
{
  char tmp[8192];
  char tmp2[8192];   
  char* col[512];

  unsigned int column = chunker->column ();
  unsigned int row    = chunker->row    ();

  for (int r = -2; r <= 2; ++r) { // -2 .. 2
    for (unsigned int c = 0; c < column; ++c) { // column
       int _r = r + pos;
       const char *str = 0;
       if (_r == -2)                str = "__BOS2__";
       else if (_r == -1)           str = "__BOS1__"; 
       else if (_r == (int)row)	    str = "__EOS1__";
       else if (_r == (int)(row+1)) str = "__EOS2__";
       else                         str = chunker->getContext (_r, c);

       strncpy (tmp, str, 8192);
       unsigned int s = tokenize (tmp, ",", col, 512); // split by ","
       for (unsigned int i = 0; i < s; ++i) {
	  snprintf (tmp2, 8192, "B:%d:%d:%s", r, c, col[i]);
	  chunker->addFeature (tmp2);
       }
    }
  }
   
  return 1; 
}

int main (int argc, char **argv) 
{
  try {

    // make instance YamCha::Chunker
    YamCha::Chunker p (argc, argv);

    // set call-back function
    p.setSelector (&addBOW);

    // parse
    while (p.parse (std::cin, std::cout)) {};

    return 0;
  }
   
  catch (std::exception &e) {
    std::cerr << e.what () << std::endl;
    return -1;
  }
}

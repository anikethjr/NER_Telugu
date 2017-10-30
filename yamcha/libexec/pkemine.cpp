/*
  YamCha -- Yet Another Multipurpose CHunk Annotator
 
  $Id: pkemine.cpp,v 1.2 2004/03/12 17:12:13 taku-ku Exp $;

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

#include <iostream>
#include <fstream>
#include <strstream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <string.h>
#include <stdlib.h>

static double WEIGHT[4][5] = {
  {0,0,0,0,0}, // 0
  {1,1,0,0,0}, // 1
  {1,3,2,0,0}, // 2
  {1,7,12,6,0} // 3
};

static double MAX_WEIGHT[5] = { 0,1,3,12 };

using namespace std;

class PKEMine {

private:

  std::vector <std::vector <unsigned int> >  transaction;
  std::vector <unsigned int> pattern;
  std::vector <double> w;
  std::vector <unsigned int> cid;

  std::vector <double> w2;
  std::vector <double> sigma_pos;   
  std::vector <double> sigma_neg;
  std::vector <double> mu_pos;   
  std::vector <double> mu_neg;
  std::vector <unsigned int> sup;
  
  double sigma;
  unsigned int minsup;
  unsigned int maxpat;
  unsigned int csize;
  std::ostream *os;

  void prune (std::vector <std::pair<unsigned int, int> > &projected)
  {
    std::fill (mu_pos.begin(),  mu_pos.end(),  0.0);
    std::fill (mu_neg.begin(),  mu_neg.end(),  0.0);
    std::fill (w2.begin(),      w2.end(),      0.0);
    std::fill (sup.begin(),     sup.end(),     0);

    for (unsigned int i = 0; i < projected.size(); i++) {

      w2[cid[projected[i].first]] += WEIGHT[maxpat][pattern.size()] * w[projected[i].first];

      ++sup[cid[projected[i].first]]; // support

      if (w[projected[i].first] > 0) 
	 mu_pos[cid[projected[i].first]] += MAX_WEIGHT[maxpat] * w[projected[i].first];
       else
	 mu_neg[cid[projected[i].first]] += MAX_WEIGHT[maxpat] * w[projected[i].first];
    }

    bool output = false;

    for (unsigned int i = 0; i < csize; ++i) {
      
      if (sup[i] < minsup || (mu_pos[i] < sigma_pos[i] && mu_neg[i] > sigma_neg[i])) {
	sup[i] = 0; // delete flg;
	continue;
      }

      // output vector
      if (w2[i] <= sigma_neg[i] || w2[i] >= sigma_pos[i]) {
	output = true;
	*os << i << ":" << w2[i] << " ";
      }
    }

    // output feature
    if (output) {
      for (unsigned int i = 0; i < pattern.size(); ++i)	*os << " " << pattern[i];
      *os << std::endl;
    }

    std::vector <std::pair<unsigned int, int> > tmp;
    for (unsigned int i = 0; i < projected.size(); ++i) {
      if (sup[cid[projected[i].first]] != 0) tmp.push_back (projected[i]);
    }

    projected = tmp;
  }

  void project (std::vector <std::pair<unsigned int, int> > &projected)
  {
    if (pattern.size() >= maxpat || projected.empty()) return;

    std::map <unsigned int, std::vector <std::pair<unsigned int, int> > > counter;
  
    for (unsigned int i = 0; i < projected.size(); ++i) {
      unsigned int id    = projected[i].first;
      int pos            = projected[i].second;
      unsigned int size  = transaction[id].size();
      for (unsigned int j = pos + 1; j < size; ++j) 
	counter[transaction[id][j]].push_back (std::make_pair<unsigned int, int>(id, j));
    }

    for (std::map <unsigned int , std::vector <std::pair<unsigned int, int> > >::iterator 
	   l = counter.begin (); l != counter.end (); ++l) {
      pattern.push_back (l->first);
      prune   (l->second);
      project (l->second);
      pattern.resize (pattern.size()-1);
    }
  }

public:

  PKEMine (double       _sigma  = 0.005,
	   unsigned int _minsup = 1,
	   unsigned int _maxpat = 0xffffffff):
    sigma(_sigma), minsup(_minsup), maxpat (_maxpat) {}

  ~PKEMine () {};

  std::ostream& run (std::istream &is, std::ostream &_os)
  {
    os = &_os;
    os->setf(std::ios::fixed, std::ios::floatfield);
    os->precision(16);

    csize = 0;
    std::vector <std::pair<unsigned int, int> > root;
    std::vector <unsigned int> tmp;
    std::string line;

    while (std::getline (is, line)) {
       std::istrstream istrs ((char *)line.c_str());
       unsigned int _cid;
       istrs >> _cid;
       csize = std::max (_cid, csize);
       
       double _w;
       istrs >> _w;
       
       tmp.clear ();       
       unsigned int item;
       while (istrs >> item) tmp.push_back (item);
       
       cid.push_back(_cid);       
       w.push_back (_w); 
       transaction.push_back (tmp);
    }

    ++csize;

    w2.resize        (csize);
    sigma_pos.resize (csize);
    sigma_neg.resize (csize);
    mu_pos.resize    (csize);
    mu_neg.resize    (csize);
    sup.resize       (csize);

    // destoroy pos_num/neg_num
    {
       std::vector<unsigned int> pos_num (csize);
       std::vector<unsigned int> neg_num (csize);
       std::fill (pos_num.begin(), pos_num.end(), 0);
       std::fill (neg_num.begin(), neg_num.end(), 0);

       for (unsigned int i = 0; i < transaction.size(); i++) {
	  root.push_back (std::make_pair<unsigned int, int>(i, -1));
	  if (w[i] > 0) ++pos_num[cid[i]];
	  else ++neg_num[cid[i]];
       }
       
       for (unsigned int i = 0; i < csize; ++i) {
	  sigma_pos[i] =  1.0 * sigma * pos_num[i] / (pos_num[i] + neg_num[i]);
	  sigma_neg[i] = -1.0 * sigma * neg_num[i] / (pos_num[i] + neg_num[i]);
       }
    }

    project (root); 
    return *os;
  }
};

int main (int argc, char **argv)
{
  double       sigma = 0.0005;
  unsigned int minsup = 2;
  unsigned int maxpat = 2;

  if (argc < 6) {
    std::cerr << "Usage: " << argv[0] << "sigma minsup maxpat File Index" << std::endl;
    return 0; 
  }

  sigma               = atof (argv[1]);
  minsup              = atoi (argv[2]);
  maxpat              = atoi (argv[3]);
  std::string infile  = argv[4];
  std::string outfile = argv[5];

  if (maxpat < 1 || maxpat > 3) {
    std::cerr << "FATAL: maxpat should be between 1 .. 3" << std::endl;
    return 1;
  }
   
  std::istream *is;
 
  if (infile == "-") is = &std::cin;
  else               is = new std::ifstream (infile.c_str());
   
  std::ofstream os (outfile.c_str());

  if (! *is) {
    std::cerr << "FATAL: Cannot open: " << infile << std::endl;
    return -1; 
  }
   
   if (! os) {
    std::cerr << "FATAL: Cannot open: " << outfile << std::endl;
    return -1; 
  }
   
   
  PKEMine pkemine (sigma, minsup, maxpat);
  pkemine.run  (*is, os);
   
  if (infile == "-") delete is; 

  return 0;
}

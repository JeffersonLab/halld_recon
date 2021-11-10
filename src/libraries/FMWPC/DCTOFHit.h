// $Id$
//
//    File: DCTOFHit.h
// Created: Tue Oct 26 18:11:17 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFHit_
#define _DCTOFHit_

#include "JANA/JObject.h"
#include "JANA/JFactory.h"

class DCTOFHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DCTOFHit);

  int bar; ///< bar number
  int end; ///< top=0,bottom=1;
  float dE; ///< attenuated energy deposition in GeV
  double t; ///< time in ns
   
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%d", bar);
    AddString(items, "end", "%d", end);
    AddString(items, "dE", "%12.4f", dE);
    AddString(items, "t", "%12.4f", t);
  }
};

#endif // _DCTOFHit_


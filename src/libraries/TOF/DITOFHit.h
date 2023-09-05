// $Id$
//
//    File: DITOFHit.h
// Created: Fri Sep  1 15:37:38 EDT 2023
// Creator: staylor (on Linux ifarm1802.jlab.org 3.10.0-1160.95.1.el7.x86_64 x86_64)
//

#ifndef _DITOFHit_
#define _DITOFHit_

#include "JANA/JObject.h"
#include "JANA/JFactory.h"

class DITOFHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DITOFHit);

  float dE; ///< energy deposition in GeV
  double t; ///< time in ns
  double x,y;
   
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "dE", "%12.4f", dE);
    AddString(items, "t", "%12.4f", t);
    AddString(items, "x", "%12.4f", x);
    AddString(items, "y", "%12.4f", y);
  }
};


#endif // _DITOFHit_


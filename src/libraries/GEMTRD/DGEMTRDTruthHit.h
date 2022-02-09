// $Id$
//
//    File: DGEMTRDTruthHit.h
// Created: Mon Feb  7 16:40:23 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DGEMTRDTruthHit_
#define _DGEMTRDTruthHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DGEMTRDTruthHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DGEMTRDTruthHit);

  int plane;
  double x,y;
  double q,t;
	
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "plane", "%4d", plane);
    AddString(items, "x", "%f", x);
    AddString(items, "y", "%f", y);
    AddString(items, "t", "%f", z);
    AddString(items, "q", "%f", q);
  }
		
};

#endif // _DGEMTRDTruthHit_


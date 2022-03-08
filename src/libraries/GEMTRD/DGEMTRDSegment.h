// $Id$
//
//    File: DGEMTRDSegment.h
// Created: Wed Feb  9 13:19:03 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DGEMTRDSegment_
#define _DGEMTRDSegment_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DGEMTRDSegment:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DGEMTRDSegment);
  
  double x,y;
  double dxdz,dydz;
		
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "x", "%f", x);
    AddString(items, "y", "%f", y);
    AddString(items, "dxdz", "%f", dxdz);
    AddString(items, "dydz", "%f", dydz);
  }
  
};

#endif // _DGEMTRDSegment_


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

  int layer;
  double x,y,t;
  double dxdz,dydz;
  double var_x,var_tx,cov_xtx;
		
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "layer", "%d", layer);
    AddString(items, "t", "%3.1f", t);
    AddString(items, "x", "%5.3f", x);
    AddString(items, "y", "%5.3f", y);
    AddString(items, "dxdz", "%f", dxdz);
    AddString(items, "dydz", "%f", dydz);
    AddString(items, "var_x", "%f", var_x);
    AddString(items, "var_tx", "%f", var_tx);
    AddString(items, "cov_xtx", "%f", cov_xtx);
  }
  
};

#endif // _DGEMTRDSegment_


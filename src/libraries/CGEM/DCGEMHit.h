// $Id$
//
//    File: DCGEMHit.h
// Created: Tue Jun 16 07:07:54 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DCGEMHit_
#define _DCGEMHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

class DCGEMHit:public JObject{
 
 public:
  JOBJECT_PUBLIC(DCGEMHit);

  DCGEMHit(){}
  
  int layer;   // 1-8
  float dE;    // energy in GeV
  float x;
  float y;
  float z;
  float t;     // time in ns
  
  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "layer", "%d", layer);
    AddString(items, "dE(keV)", "%3.1f", dE*1.0E6);
    AddString(items, "t", "%3.3f", t);
    AddString(items, "x", "%3.1f", x);
    AddString(items, "y", "%3.1f", y);
    AddString(items, "z", "%3.1f", z);
  }
  
};

#endif // _DCGEMHit_


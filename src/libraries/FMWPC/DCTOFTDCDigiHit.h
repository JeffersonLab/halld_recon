// $Id$
//
//    File: DCTOFTDCDigiHit.h
// Created: Fri Mar 18 09:09:21 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
/// Container class for raw CAEN TDC hits for CTOF paddles

#ifndef _DCTOFTDCDigiHit_
#define _DCTOFTDCDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DCTOFTDCDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DCTOFTDCDigiHit);
   
  int bar;        ///< bar number
  int end;        ///< TOP/BOTTOM 0/1
  uint32_t time;  ///< hit time

  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%d", bar);
    AddString(items, "end", "%d", end);
    AddString(items, "time", "%d", time);
  }
  
};

#endif // _DCTOFTDCDigiHit_


// $Id: DDIRCLEDRef.h
//
/// File:    DDIRCLEDRef.h
//

#ifndef _DDIRCLEDRef_
#define _DDIRCLEDRef_

#include "JANA/JObject.h"
#include "JANA/JFactory.h"

class DDIRCLEDRef:public jana::JObject{
  
 public:
  JOBJECT_PUBLIC(DDIRCLEDRef);
 
  float amp;           // Signal max Amplitude, ADC counts 
  float integral;      // Signal integral
  float t_fADC;   // time from adc
  float t_TDC;  // time from tdc
  float t; // walk corrected time
  bool has_fADC;
  bool has_TDC;
  
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "integral", "%12.4f", integral);
    AddString(items, "amp", "%12.4f", amp);
    AddString(items, "t", "%12.4f", t);
    AddString(items, "t_TDC","%12.4f",t_TDC);
    AddString(items, "t_fADC","%12.4f",t_fADC);
  }
};

#endif // _DDIRCLEDRef_


// $Id: DDIRCLEDRef.h
//
/// File:    DDIRCLEDRef.h
//

#ifndef _DDIRCLEDRef_
#define _DDIRCLEDRef_

#include "JANA/JObject.h"

class DDIRCLEDRef: public JObject{
  
 public:
  JOBJECT_PUBLIC(DDIRCLEDRef);
 
  float amp;           // Signal max Amplitude, ADC counts 
  float integral;      // Signal integral
  float t_fADC;   // time from adc
  float t_TDC;  // time from tdc
  float t; // walk corrected time
  bool has_fADC;
  bool has_TDC;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(integral, "integral", "%12.4f");
    summary.add(amp, "amp", "%12.4f");
    summary.add(t, "t", "%12.4f");
    summary.add(t_TDC, "t_TDC", "%12.4f");
    summary.add(t_fADC, "t_fADC", "%12.4f");
  }
};

#endif // _DDIRCLEDRef_


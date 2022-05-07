// $Id$
//
//    File: DCTOFDigiHit.h
// Created: Fri Mar 18 09:08:07 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
/// Container class for raw FADC hits for CTOF paddles

#ifndef _DCTOFDigiHit_
#define _DCTOFDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DCTOFDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DCTOFDigiHit);
  
  int bar;     ///< bar number: 1-4
  int end;     ///< TOP/BOTTIOM 0/1
  uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
  uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
  uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
  uint32_t QF;             ///<  Quality Factor from FPGA algorithms
  uint32_t nsamples_integral;    ///<  number of samples used in integral 
  uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
  uint32_t pulse_peak;           ///<  maximum ADC value in pulse peak (pedestal is NOT subtracted)

  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%d", bar);
    AddString(items, "end", "%d", end);
    AddString(items, "pulse_integral", "%d", pulse_integral);
    AddString(items, "pulse_peak", "%d", pulse_peak);
    AddString(items, "pulse_time", "%d", pulse_time);
    AddString(items, "pedestal", "%d", pedestal);
    AddString(items, "QF", "%d", QF);
    AddString(items, "nsamples_integral", "%d", nsamples_integral);
    AddString(items, "nsamples_pedestal", "%d", nsamples_pedestal);
  }
  
};

#endif // _DCTOFDigiHit_


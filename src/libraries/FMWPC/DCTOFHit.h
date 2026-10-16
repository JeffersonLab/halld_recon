// $Id$
//
//    File: DCTOFHit.h
// Created: Tue Oct 26 18:11:17 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFHit_
#define _DCTOFHit_

#include "JANA/JObject.h"

class DCTOFHit:public JObject{
 public:
  JOBJECT_PUBLIC(DCTOFHit);

  int bar; ///< bar number
  int end; ///< top=0,bottom=1;
  float dE; ///< attenuated energy deposition in GeV
  double t; ///< time in ns
  double t_adc;///< time in ns from FADC
   
  void Summarize(JObjectSummary& summary) const {
    summary.add(bar, "bar", "%d");
    summary.add(end, "end", "%d");
    summary.add(dE, "dE", "%12.4f");
    summary.add(t, "t", "%12.4f");
    summary.add(t_adc, "t_adc", "%12.4f");
  }
};

#endif // _DCTOFHit_


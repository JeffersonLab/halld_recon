// $Id$
//
//    File: DPSCPair.h
// Created: Tue Mar 24 21:35:49 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#ifndef _DPSCPair_
#define _DPSCPair_

#include <JANA/JObject.h>
#include "DPSCHit.h"
using std::pair;

class DPSCPair: public JObject{
 public:
  JOBJECT_PUBLIC(DPSCPair);

  pair<const DPSCHit*,const DPSCHit*> ee;	// first:North(left); second:South(right)		
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(ee.first->module, "module_lhit", "%d");
    summary.add(ee.second->module, "module_rhit", "%d");
    summary.add(ee.first->time_tdc, "t_tdc_lhit", "%f");
    summary.add(ee.second->time_tdc, "t_tdc_rhit", "%f");
    summary.add(ee.first->time_fadc, "t_fadc_lhit", "%f");
    summary.add(ee.second->time_fadc, "t_fadc_rhit", "%f");
    summary.add(ee.first->integral, "integral_lhit", "%f");
    summary.add(ee.second->integral, "integral_rhit", "%f");
    summary.add(ee.first->pulse_peak, "pulse_peak_lhit", "%f");
    summary.add(ee.second->pulse_peak, "pulse_peak_rhit", "%f");
  }
		
};

#endif // _DPSCPair_


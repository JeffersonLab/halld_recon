// $Id$
//
//    File: DPSCHit.h
// Created: Wed Oct 15 16:45:33 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _DPSCHit_
#define _DPSCHit_

#include <JANA/JObject.h>
#include "DPSGeometry.h"

class DPSCHit: public JObject{
 public:
  JOBJECT_PUBLIC(DPSCHit);

  DPSGeometry::Arm arm;   // North(left): 0, South(right): 1
  int module;
  double t;
  double integral;
  double pulse_peak;
  double time_tdc;
  double time_fadc;
  double npe_fadc;
  bool has_fADC,has_TDC;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(arm, "arm", "%d");
    summary.add(module, "module", "%d");
    summary.add(t, "t(ns)", "%f");
    summary.add(time_tdc, "time_tdc(ns)", "%f");
    summary.add(time_fadc, "time_fadc(ns)", "%f");
    summary.add(integral, "integral", "%f");
    summary.add(pulse_peak, "pulse_peak", "%f");
    summary.add(npe_fadc, "npe_fadc", "%f");
    summary.add((int)has_fADC, "has_fADC", "%d");
    summary.add((int)has_TDC, "has_TDC", "%d");
  }
};

#endif // _DPSCHit_


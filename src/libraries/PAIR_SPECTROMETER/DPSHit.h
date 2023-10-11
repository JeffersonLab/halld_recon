// $Id$
//
//    File: DPSHit.h
// Created: Wed Oct 15 16:45:01 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _DPSHit_
#define _DPSHit_

#include <JANA/JObject.h>
#include "DPSGeometry.h"

class DPSHit: public JObject{
 public:
  JOBJECT_PUBLIC(DPSHit);

  DPSGeometry::Arm arm;   // North(left): 0, South(right): 1
  int column;
  double E;  
  double t;
  double integral;
  double pulse_peak;
  double npix_fadc;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(arm, "arm", "%d");
    summary.add(column, "column", "%d");
    summary.add(E, "E(GeV)", "%f");
    summary.add(t, "t(ns)", "%f");
    summary.add(integral, "integral", "%f");
    summary.add(pulse_peak, "pulse_peak", "%f");
    summary.add(npix_fadc, "npix_fadc", "%f");
  }
};

#endif // _DPSHit_


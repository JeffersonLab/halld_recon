// $Id$
//
//    File: DCDCHit.h
// Created: Thu Jun  9 10:22:37 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DCDCHit_
#define _DCDCHit_

#include <JANA/JObject.h>

class DCDCHit: public JObject{
 public:
  JOBJECT_PUBLIC(DCDCHit);
  
  int ring;
  int straw;
  float q;
  float amp;
  float t;
  float d;
  int QF;
  int itrack;
  int ptype;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(ring, "ring", "%d");
    summary.add(straw, "straw", "%d");
    summary.add(q, "q", "%10.4e");
    summary.add(amp, "amp", "%10.4e");
    summary.add(t, "t", "%6.1f");
    summary.add(d, "d", "%10.4e");
    summary.add(itrack, "itrack", "%d");
    summary.add(ptype, "ptype", "%d");
    summary.add(QF, "QF", "%d");
 }
};

#endif // _DCDCHit_


// $Id$
//
//    File: DFCALHit.h
// Created: Thu Jun  9 10:29:52 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DFCALHit_
#define _DFCALHit_

#include <JANA/JObject.h>
#include <DANA/DObjectID.h>

class DFCALHit: public JObject{
	
public:
	
  JOBJECT_PUBLIC(DFCALHit);
	
  DFCALHit(){}

  oid_t id = reinterpret_cast<oid_t>(this);
  int row;
  int column;
  float x;
  float y;
  float E;
  float t;
  float intOverPeak;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(row, "row", "%4d");
    summary.add(column, "column", "%4d");
    summary.add(x, "x(cm)", "%3.1f");
    summary.add(y, "y(cm)", "%3.1f");
    summary.add(E*1000.0, "E(MeV)", "%2.3f");
    summary.add(t, "t(ns)", "%2.3f");
    summary.add(intOverPeak, "integral over peak", "%2.3f");
  }
};

#endif // _DFCALHit_


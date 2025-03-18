// $Id$

#ifndef _DPSCTruthHit_
#define _DPSCTruthHit_

#include <JANA/JObject.h>

class DPSCTruthHit: public JObject{
 public:
  JOBJECT_PUBLIC(DPSCTruthHit);
  
  float dEdx;
  bool primary;
  int track;
  int itrack;
  int ptype;
  float x;
  float y;
  float z;
  float t;
  int column;
		
  void Summarize(JObjectSummary& summary) const override {
    summary.add(track, "track", "%d");
    summary.add(itrack, "itrack", "%d");
    summary.add(primary, "primary", "%d");
    summary.add(ptype, "ptype", "%d");
    summary.add(dEdx*1.0E3, "dEdx(MeV/cm)", "%1.3f");
    summary.add(t, "t", "%3.2f");
    summary.add(x, "x", "%3.1f");
    summary.add(y, "y", "%3.1f");
    summary.add(z, "z", "%3.1f");
    summary.add(column, "column", "%d");
  }
};

#endif // _DPSCTruthHit_


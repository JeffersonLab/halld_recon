// -----------------------------------------
// DGEMHit.h
// -----------------------------------------

#ifndef DGEMHIT_H_
#define DGEMHIT_H_

#include <JANA/JObject.h>

class DGEMHit: public JObject {

public:
  JOBJECT_PUBLIC (DGEMHit);

  float t;	      // detection time
  float pulse_height; // matching terminology in FDCHit
  int   plane;        // plane number
  int   strip;        // strip number

  void Summarize(JObjectSummary& summary) const override {
    summary.add(t, "t", "%1.3f");
    summary.add(pulse_height, "pulse_height", "%1.3f");
    summary.add(plane, "plane", "%d");
    summary.add(strip, "strip", "%d");
  }
};

#endif /* DGEMHIT_H_ */

// -----------------------------------------
// DTRDHit.h
// -----------------------------------------

#ifndef DTRDHIT_H_
#define DTRDHIT_H_

#include <JANA/JObject.h>

class DTRDHit: public JObject {

public:
  JOBJECT_PUBLIC (DTRDHit);

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

#endif /* DTRDHIT_H_ */

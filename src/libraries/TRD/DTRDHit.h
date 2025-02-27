// -----------------------------------------
// DTRDHit.h
// -----------------------------------------

#ifndef DTRDHIT_H_
#define DTRDHIT_H_

#include <JANA/JObject.h>

class DTRDHit: public JObject {

public:
  JOBJECT_PUBLIC (DTRDHit);

  int   plane;        // plane number  (X=1, Y=2)
  int   strip;        // strip number  (different numbers for X and Y planes, from CCDB)

  float t;	      // detection time
  float pulse_height; // signal amplitude
  float pedestal;  // need pedestal for raw data analysis
  float q;
  

  void Summarize(JObjectSummary& summary) const override {
    summary.add(t, "t", "%1.3f");
    summary.add(pulse_height, "pulse_height", "%1.3f");
    summary.add(q, "q", "%1.3f");
    summary.add(plane, "plane", "%d");
    summary.add(strip, "strip", "%d");

  }
};

#endif /* DTRDHIT_H_ */

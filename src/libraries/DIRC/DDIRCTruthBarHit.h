// -----------------------------------------
// DDIRCTruthBarHit.h
// created on: 05.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DDIRCTRUTHBARHIT_H_
#define DDIRCTRUTHBARHIT_H_

#include <JANA/JObject.h>

class DDIRCTruthBarHit: public JObject {

public:
  JOBJECT_PUBLIC (DDIRCTruthBarHit);

  float x, y, z;     //  coordinate where ch. track hits the radiator
  float px, py, pz;  //  components of the track momentum
  float t;	     // time
  float E;	     // energy

  int pdg;           // PDG of the particle
  int bar;           // index of the bar
  int track;         // index of the MC track

  void Summarize(JObjectSummary& summary) const override {
    summary.add(x, "x", "%1.3f");
    summary.add(y, "y", "%1.3f");
    summary.add(z, "z", "%1.3f");
    summary.add(px, "px", "%1.3f");
    summary.add(py, "py", "%1.3f");
    summary.add(pz, "pz", "%1.3f");
    summary.add(t, "t", "%1.3f");
    summary.add(E, "E", "%1.3f");
    summary.add(pdg, "pdg", "%d");
    summary.add(bar, "bar", "%d");
    summary.add(track, "track", "%d");
  }
};

#endif /* DDIRCTRUTHBARHIT_H_ */

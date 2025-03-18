// -----------------------------------------
// DDIRCTruthPmtHit.h
// created on: 05.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DDIRCTRUTHPMTHIT_H_
#define DDIRCTRUTHPMTHIT_H_

#include <JANA/JObject.h>

class DDIRCTruthPmtHit: public JObject {

public:
  JOBJECT_PUBLIC (DDIRCTruthPmtHit);

  float x, y, z;   // hit position
  float t;	   // detection time
  float t_fixed;   // fixed pathlength time
  float E;	   // poton energy
  int   ch;        // PMT channel of the hit
  int   key_bar;   // key of the corresponding bar hit
  int64_t path;	   // path id of the photon in the optical box 
  int refl;	   // number of reflection in the optical box
  bool bbrefl;     // reflected off far end mirror of bar box
  int track;       // index of the MC track

  void Summarize(JObjectSummary& summary) const override {
    summary.add(x, "x", "%1.3f");
    summary.add(y, "y", "%1.3f");
    summary.add(z, "z", "%1.3f");
    summary.add(t, "t", "%1.3f");
    summary.add(t_fixed, "t_fixed", "%1.3f");
    summary.add(E, "E", "%1.3f");
    summary.add(ch, "ch", "%d");
    summary.add(key_bar, "key_bar", "%d");
    summary.add(path, "path", "%ld");
    summary.add(refl, "refl", "%d");
    summary.add(bbrefl ? 1 : 0, "bbrefl", "%d");
  }
};

#endif /* DDIRCTRUTHPMTHIT_H_ */

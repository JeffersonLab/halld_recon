// -----------------------------------------
// DDIRCTruthPmtHit.h
// created on: 05.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DDIRCTRUTHPMTHIT_H_
#define DDIRCTRUTHPMTHIT_H_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

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
  
  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "x", "%1.3f", x);
    AddString(items, "y", "%1.3f", y);
    AddString(items, "z", "%1.3f", z);
    AddString(items, "t", "%1.3f", t);
    AddString(items, "t_fixed", "%1.3f", t_fixed);
    AddString(items, "E", "%1.3f", E);
    AddString(items, "ch", "%d", ch);
    AddString(items, "key_bar", "%d", key_bar);
    AddString(items, "path", "%ld", path);
    AddString(items, "refl", "%d", refl);
    AddString(items, "bbrefl", "%d", bbrefl ? 1 : 0);
  }
};

#endif /* DDIRCTRUTHPMTHIT_H_ */

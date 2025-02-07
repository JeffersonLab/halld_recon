// -----------------------------------------
// DTRDHit.h
// -----------------------------------------

#ifndef DTRDHIT_H_
#define DTRDHIT_H_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

class DTRDHit: public JObject {

public:
  JOBJECT_PUBLIC (DTRDHit);

  int   plane;        // plane number  (X=1, Y=2)
  int   strip;        // strip number  (different numbers for X and Y planes, from CCDB)

  float t;	      // detection time
  float pulse_height; // signal amplitude
  //float q;
  

  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "plane", "%d", plane);
    AddString(items, "strip", "%d", strip);
    AddString(items, "t", "%1.3f", t);
    AddString(items, "pulse_height", "%1.3f", pulse_height);
  }
};

#endif /* DTRDHIT_H_ */

// -----------------------------------------
// DGEMHit.h
// -----------------------------------------

#ifndef DGEMHIT_H_
#define DGEMHIT_H_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

class DGEMHit: public JObject {

public:
  JOBJECT_PUBLIC (DGEMHit);

  float t;	      // detection time
  float pulse_height; // matching terminology in FDCHit
  int   plane;        // plane number
  int   strip;        // strip number

  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "t", "%1.3f", t);
    AddString(items, "pulse_height", "%1.3f", pulse_height);
    AddString(items, "plane", "%d", plane);
    AddString(items, "strip", "%d", strip);
  }
};

#endif /* DGEMHIT_H_ */

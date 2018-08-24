// -----------------------------------------
// DDIRCPmtHit.h
// -----------------------------------------

#ifndef DDIRCPMTHIT_H_
#define DDIRCPMTHIT_H_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

class DDIRCPmtHit: public JObject {

public:
  JOBJECT_PUBLIC (DDIRCPmtHit);

  float t;	   // detection time
  int   ch;        // PMT channel of the hit
  
  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "t", "%1.3f", t);
    AddString(items, "ch", "%d", ch);
  }
};

#endif /* DDIRCPMTHIT_H_ */

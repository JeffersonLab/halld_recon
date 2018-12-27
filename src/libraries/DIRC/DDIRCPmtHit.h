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
  float tot;       // time-over-threshold
  int   ch;        // PMT channel of the hit
 
  inline void setTime( double time ) { t = time;}
  inline void setTOT( double timeOverThreshold ) { tot = timeOverThreshold;}
  inline void setChannel( int channel ) { ch = channel;}
 
  void toStrings(vector<pair<string, string> >&items) const {
    AddString(items, "t", "%1.3f", t);
    AddString(items, "tot", "%1.3f", tot);
    AddString(items, "ch", "%d", ch);
  }
};

#endif /* DDIRCPMTHIT_H_ */

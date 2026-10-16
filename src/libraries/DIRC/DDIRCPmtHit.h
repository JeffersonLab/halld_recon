// -----------------------------------------
// DDIRCPmtHit.h
// -----------------------------------------

#ifndef DDIRCPMTHIT_H_
#define DDIRCPMTHIT_H_

#include <JANA/JObject.h>

class DDIRCPmtHit: public JObject {

public:
  JOBJECT_PUBLIC (DDIRCPmtHit);

  float t;	   // detection time
  float tot;       // time-over-threshold
  int   ch;        // PMT channel of the hit
 
  inline void setTime( double time ) { t = time;}
  inline void setTOT( double timeOverThreshold ) { tot = timeOverThreshold;}
  inline void setChannel( int channel ) { ch = channel;}

  void Summarize(JObjectSummary& summary) const override {
    summary.add(t, "t", "%1.3f");
    summary.add(tot, "tot", "%1.3f");
    summary.add(ch, "ch", "%d");
  }
};

#endif /* DDIRCPMTHIT_H_ */

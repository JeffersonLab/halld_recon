// $Id$
//
//    File: DPSCTDCDigiHit.h
// Created: Wed Oct 15 16:46:32 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _DPSCTDCDigiHit_
#define _DPSCTDCDigiHit_

#include <JANA/JObject.h>

class DPSCTDCDigiHit : public JObject {
 public:
  JOBJECT_PUBLIC(DPSCTDCDigiHit);

  int counter_id;
  uint32_t time;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(counter_id, NAME_OF(counter_id), "%d");
    summary.add(time, NAME_OF(time), "%d");
  }
};

#endif // _DPSCTDCDigiHit_


// $Id$
//
//    File: DCppPid.h
// Created: Thu Mar  5 02:05:32 PM EST 2026
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-611.30.1.el9_7.x86_64 x86_64)
//

#ifndef _DCppPid_h_
#define _DCppPid_h_

#include <JANA/JObject.h>

class DCppPid : public JObject {
 public:
   JOBJECT_PUBLIC(DCppPid);
   
   
   void Summarize(JObjectSummary& summary) const override {
     
   }
};


#endif // _DCppPid_h_


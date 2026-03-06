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
   
   int fmwpc1m; ///< Multiplicity of chamber 1
   int fmwpc2m; ///< Multiplicity of chamber 2
   int fmwpc3m; ///< Multiplicity of chamber 3
   int fmwpc4m; ///< Multiplicity of chamber 4
   int fmwpc5m; ///< Multiplicity of chamber 5
   int fmwpc6m; ///< Multiplicity of chamber 6

   void Summarize(JObjectSummary& summary) const override {
     summary.add(fmwpc1m,"fmwpc1m","%4d");
     summary.add(fmwpc2m,"fmwpc2m","%4d");
     summary.add(fmwpc3m,"fmwpc3m","%4d");
     summary.add(fmwpc4m,"fmwpc4m","%4d");
     summary.add(fmwpc5m,"fmwpc5m","%4d");
     summary.add(fmwpc6m,"fmwpc6m","%4d");
   }
};


#endif // _DCppPid_h_


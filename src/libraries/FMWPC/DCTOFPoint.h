// $Id$
//
//    File: DCTOFPoint.h
// Created: Thu Oct 28 07:48:04 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFPoint_
#define _DCTOFPoint_

#include <JANA/JObject.h>
#include <DVector3.h>

class DCTOFPoint:public JObject{
 public:
  JOBJECT_PUBLIC(DCTOFPoint);

  int bar; ///< bar number
  double dE; ///< Energy deposition in GeV
  double t; ///< Time in ns 
  DVector3 pos; ///< point position vector, in cm
		
  // This method is used primarily for pretty printing
  void Summarize(JObjectSummary& summary) const override {
    summary.add(bar, "bar", "%4d");
    summary.add(dE, "dE", "%f");
    summary.add(t, "t", "%f");
    summary.add(pos.x(), "x", "%f");
    summary.add(pos.y(), "y", "%f");
    summary.add(pos.z(), "z", "%f");
  }
		
};

#endif // _DCTOFPoint_


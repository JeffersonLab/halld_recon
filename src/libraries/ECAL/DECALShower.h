// $Id$
//
//    File: DECALShower.h
// Created: Tue Mar 25 10:43:23 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#ifndef _DECALShower_h_
#define _DECALShower_h_

#include <JANA/JObject.h>
#include <DVector3.h>
#include <TMatrixFSym.h>

struct DECALShower : public JObject {
 public:
  JOBJECT_PUBLIC(DECALShower);
  
  DECALShower(){};

  double E,t;
  DVector3 pos;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(E, "E(GeV)", "%f");
    summary.add(t, "t(ns)", "%f");
    summary.add(pos.x(), "x(cm)", "%f");
    summary.add(pos.y(), "y(cm)", "%f");
    summary.add(pos.z(), "z(cm)", "%f");
  }
};

#endif // _DECALShower_h_


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
  double E1E9,E9E25;
  DVector3 pos;
  TMatrixFSym ExyztCovariance;
  bool isNearBorder;
  int nBlocks;

  float EErr() const { return sqrt(ExyztCovariance(0,0)); }
  float xErr() const { return sqrt(ExyztCovariance(1,1)); }
  float yErr() const { return sqrt(ExyztCovariance(2,2)); }
  float zErr() const { return sqrt(ExyztCovariance(3,3)); }
  float tErr() const { return sqrt(ExyztCovariance(4,4)); }
  float XYcorr() const {
    if (xErr()>0 && yErr()>0) return ExyztCovariance(1,2)/xErr()/yErr();
    else return 0;
  }
  float XZcorr() const {
    if (xErr()>0 && zErr()>0) return ExyztCovariance(1,3)/xErr()/zErr();
    else return 0;
  }
  float YZcorr() const {
    if (yErr()>0 && zErr()>0) return ExyztCovariance(2,3)/yErr()/zErr();
    else return 0;
  }
  float EXcorr() const {
    if (EErr()>0 && xErr()>0) return ExyztCovariance(0,1)/EErr()/xErr();
    else return 0;
  }
  float EYcorr() const {
    if (EErr()>0 && yErr()>0) return ExyztCovariance(0,2)/EErr()/yErr();
    else return 0;
  }
  float EZcorr() const {
    if (EErr()>0 && zErr()>0) return ExyztCovariance(0,3)/EErr()/zErr();
    else return 0;
  }
  float ZTcorr() const {
    if (zErr()>0 && tErr()>0) return ExyztCovariance(3,4)/zErr()/tErr();
    else return 0;
  }
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(E, "E(GeV)", "%f");
    summary.add(E1E9, "E1/E9", "%f");
    summary.add(E9E25, "E9/E25", "%f");
    summary.add(t, "t(ns)", "%f");
    summary.add(pos.x(), "x(cm)", "%f");
    summary.add(pos.y(), "y(cm)", "%f");
    summary.add(pos.z(), "z(cm)", "%f");
    summary.add(EErr(), "dE", "%5.3f");
    summary.add(tErr(), "dt", "%5.3f");
    summary.add(xErr(), "dx", "%5.3f");
    summary.add(yErr(), "dy", "%5.3f");
    summary.add(zErr(), "dz", "%5.3f");
    summary.add(EZcorr(), "EZcorr", "%5.3f");
    summary.add(XYcorr(), "XYcorr", "%5.3f");
    summary.add(XZcorr(), "XZcorr", "%5.3f");
    summary.add(YZcorr(), "YZcorr", "%5.3f");
    summary.add(ZTcorr(), "ZTcorr", "%5.3f");    
    summary.add(nBlocks,"Number of blocks","%d");
    summary.add(isNearBorder,"Near border?","%d");
  }
};

#endif // _DECALShower_h_


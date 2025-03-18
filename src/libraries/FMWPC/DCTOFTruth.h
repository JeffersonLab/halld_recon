// $Id$
//
//    File: DCTOFTruth.h
// Created: Wed Nov  3 20:54:15 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFTruth_
#define _DCTOFTruth_

#include "JANA/JObject.h"

class DCTOFTruth:public JObject{
 public:JOBJECT_PUBLIC(DCTOFTruth);
  
  int track;         ///<  track index
  int itrack;        ///< MCThrown track index
  int primary;       ///<  0: secondary, 1: primary
  float x, y, z;     ///<  true point of intersection
  float px,py,pz;    ///<  momentum of the particle
  float t;           ///<  true time
  float E;           ///<  energy of the particle
  int ptype;         ///<  GEANT particle type
  
  /// \fn void Summarize(JObjectSummary& summary) const
  /// method used by hd_dump to print this container data for each MC thrown hit
  void Summarize(JObjectSummary& summary)const{
    summary.add(track, "track", "%d");
    summary.add(itrack, "itrack", "%d");
    summary.add(primary, "primary", "%d");
    summary.add(ptype, "ptype", "%d");
    summary.add(x, "x", "%1.3f");
    summary.add(y, "y", "%1.3f");
    summary.add(z, "z", "%1.3f");
    summary.add(t, "t", "%1.3f");
    summary.add(px, "px", "%1.3f");
    summary.add(py, "py", "%1.3f");
    summary.add(pz, "pz", "%1.3f");
    summary.add(E, "E", "%1.3f");
  }
};

#endif // _DCTOFTruth_


// $Id$
//
//    File: DTRDTruthPoint.h
// Created: Tue Jan 14 03:47:43 PM EST 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-427.42.1.el9_4.x86_64 x86_64)
//

#ifndef _DTRDTruthPoint_
#define _DTRDTruthPoint_

#include <JANA/JObject.h>
using namespace std;

class DTRDTruthPoint: public JObject{
public:
  JOBJECT_PUBLIC(DTRDTruthPoint);                        /// DANA identifier

  int track;         ///<  track index
  int itrack;        ///< MCThrown track index
  int primary;       ///<  0: secondary, 1: primary
  float x, y, z;     ///<  true point of intersection
  float px,py,pz;    ///<  momentum of the particle
  float t;           ///<  true time
  float E;           ///<  energy of the particle
  int ptype;         ///<  GEANT particle type
  
  /// \fn void toStrings(vector<pair<string,string> > &items)const
  /// method used by hd_dump to print this container data for each MC thrown hit

  void Summarize(JObjectSummary& summary) const override {
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

#endif // _DTRDTruthPoint_


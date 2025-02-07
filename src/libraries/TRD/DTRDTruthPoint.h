// $Id$
//
//    File: DTRDTruthPoint.h
// Created: Tue Jan 14 03:47:43 PM EST 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-427.42.1.el9_4.x86_64 x86_64)
//

#ifndef _DTRDTruthPoint_
#define _DTRDTruthPoint_

#include <JANA/JObject.h>
using namespace jana;

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

  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "track", "%d", track);
    AddString(items, "itrack", "%d", itrack);
    AddString(items, "primary", "%d", primary);
    AddString(items, "ptype", "%d", ptype);
    AddString(items, "x", "%1.3f", x);
    AddString(items, "y", "%1.3f", y);
    AddString(items, "z", "%1.3f", z);
    AddString(items, "t", "%1.3f", t);
    AddString(items, "px", "%1.3f", px);
    AddString(items, "py", "%1.3f", py);
    AddString(items, "pz", "%1.3f", pz);
    AddString(items, "E", "%1.3f", E);
  }
};

#endif // _DTRDTruthPoint_


// $Id$
//
/*! **File**: DSCTruthHit.h
 *+ Created: Wed Feb  7 10:53:46 EST 2007
 *+ Creator: davidl (on Linux megrez.jlab.org 2.6.9-42.0.2.ELsmp x86_64)
 *+ Purpose: Container class holding "truth" information from MC data 
*/

/// \addtogroup SCDetector

/*! \file DSCTruthHit.h
 * Container class holding MC truth hits in the  Start Counter Detector made
 * by thrown tracks.
 */

#ifndef _DSCTruthHit_
#define _DSCTruthHit_

#include <JANA/JObject.h>

class DSCTruthHit:public JObject{
 public:
  JOBJECT_PUBLIC(DSCTruthHit);
  
  float dEdx; ///< particle energy loss in counter
  bool primary; ///< MC generator primary particle
  int track;  ///< MC track number
  int itrack; ///< MC track ID number
  int ptype;  ///< MC generated particle type
  float r;    ///< radial hit position
  float phi;  ///< phi hit position
  float z;    ///< z hit position
  float t;    ///< time of hit
  int sector; ///< sector that got hit
  
  void Summarize(JObjectSummary& summary) const {
    summary.add(track, "track", "%d");
    summary.add(itrack, "itrack", "%d");
    summary.add(primary, "primary", "%d");
    summary.add(ptype, "ptype", "%d");
    summary.add(dEdx*1.0E3, "dEdx(MeV/cm)", "%1.3f");
    summary.add(t, "t", "%3.2f");
    summary.add(r, "r", "%3.1f");
    summary.add(phi, "phi", "%1.3f");
    summary.add(z, "z", "%3.1f");
    summary.add(sector, "sector", "%d");
  }
};

#endif // _DSCTruthHit_


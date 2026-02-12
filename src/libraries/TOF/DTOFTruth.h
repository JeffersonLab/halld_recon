// $Id$
//
/*! **File**: DTOFTruth.h
 *+ Created: Mon Oct 17 13:58:02 EST 2005
 *+ Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
 *+ Purpose: Container class hodling MC thrown data as truth hits in the TOF
*/

/// \addtogroup TOFDetector

/*! \file DTOFTruth.h
 * Container class holding MC thrown this in TOF
 */


#ifndef _DTOFTruth_
#define _DTOFTruth_

#include <JANA/JObject.h>

class DTOFTruth:public JObject{
  
 public:
  JOBJECT_PUBLIC(DTOFTruth);
  
  int track;         ///<  track index
  int itrack;        ///< MCThrown track index
  int primary;       ///<  0: secondary, 1: primary
  float x, y, z;     ///<  true point of intersection
  float px,py,pz;    ///<  momentum of the particle
  float t;           ///<  true time
  float E;           ///<  energy of the particle
  int ptype;         ///<  GEANT particle type
  
  /// \fn void Summarize(JObjectSummary& summary)const
  /// method used by hd_dump to print this container data for each MC thrown hit.
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

#endif // _DTOFTruth_


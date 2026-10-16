//
/*! **File**: DTPOLTruthHit.h
 *+ Created: unknown
 *+ Creator: unknown
 *+ Purpose: Container class holdinig the TPOL "truth" hit from MC thrown events. These hits
 * are only generated for MC data.
*/

/// \addtogroup TPOLDetector

/*! \file DTPOLTruthHit.h
 * container class hodling MC thrown data for TPOL
 */


#ifndef _DTPOLTruthHit_
#define _DTPOLTruthHit_

#include <JANA/JObject.h>

class DTPOLTruthHit:public JObject{
 public:
  JOBJECT_PUBLIC(DTPOLTruthHit);
  
  float dEdx;    ///< energy loss in the silicon
  bool primary;  ///< type of genrated particle, if primary
  int track;     ///< track number
  int itrack;    ///< track ID
  int ptype;     ///< particle type
  float r;       ///< radius of hit
  float phi;     ///< phi of hit
  float z;       ///< z position of hit 
  float t;       ///< time of hit
  int sector;    ///< sector number
  
  /// \fn   void Summarize(JObjectSummary& summary) const
  /// used by hd_dump to print all TPOL Sector truth Hit object MC data for an event.
  void Summarize(JObjectSummary& summary)const{
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

#endif // _DTPOLTruthHit_


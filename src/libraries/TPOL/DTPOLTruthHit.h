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
#include <JANA/JFactory.h>

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
  
  /// \fn   void toStrings(vector<pair<string,string> > &items)const
  /// used by hd_dump to print all TPOL Sector truth Hit object MC data for an event.
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "track", "%d", track);
    AddString(items, "itrack", "%d", itrack);
    AddString(items, "primary", "%d", primary);
    AddString(items, "ptype", "%d", ptype);
    AddString(items, "dEdx(MeV/cm)", "%1.3f", dEdx*1.0E3);
    AddString(items, "t", "%3.2f", t);
    AddString(items, "r", "%3.1f", r);
    AddString(items, "phi", "%1.3f", phi);
    AddString(items, "z", "%3.1f", z);
    AddString(items, "sector", "%d", sector);
  }
};

#endif // _DTPOLTruthHit_



#ifndef DTRDSTRIPCLUSTER_H
#define DTRDSTRIPCLUSTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "DTRDHit.h"
#include <JANA/JObject.h>
using namespace jana;

#include <DVector3.h>

#define HIT_TIME_DIFF_MIN 10.0   // update this!

class DTRDStripCluster : public JObject {
 public:
  JOBJECT_PUBLIC(DTRDStripCluster);		/// DANA identifier
  
  vector<const DTRDHit*> members; ///< DTRDHits that make up this cluster
  int num_hits;               ///< number of hits (needed for raw analysis)
  int plane;			      ///< GEM X=1, GEM Y=2 
  float q_tot;		          ///< total energy/charge deposited in the cluster
		
  DVector3 pos;
  DVector3 width;
  DVector3 length;
		
  /// Return a sensible string representation of this object
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Nmembers", "%d", members.size());
    AddString(items, "plane", "%d", plane);
    AddString(items, "q_tot", "%f", q_tot);
  }
};

#endif //DTRDSTRIPCLUSTER_H


#ifndef DTRDSTRIPCLUSTER_H
#define DTRDSTRIPCLUSTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "DTRDHit.h"
#include <JANA/JObject.h>
using namespace jana;

#define HIT_TIME_DIFF_MIN 10.0

class DTRDStripCluster : public JObject {
 public:
  JOBJECT_PUBLIC(DTRDStripCluster);		/// DANA identifier
  
  vector<const DTRDHit*> members; ///< DTRDHits that make up this cluster
  int plane;			  ///< WIRE STRIP=1, GEM X=2, GEM Y=3
  float q_tot;		          ///< total energy/charge deposited in the cluster
		
  /// Return a sensible string representation of this object
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Nmembers", "%d", members.size());
    AddString(items, "plane", "%d", plane);
    AddString(items, "q_tot", "%f", q_tot);
  }
};

#endif //DTRDSTRIPCLUSTER_H

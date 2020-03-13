
#ifndef DGEMSTRIPCLUSTER_H
#define DGEMSTRIPCLUSTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "DGEMHit.h"
#include <JANA/JObject.h>
using namespace jana;

#define HIT_TIME_DIFF_MIN 10.0

class DGEMStripCluster : public JObject {
 public:
  JOBJECT_PUBLIC(DGEMStripCluster);		/// DANA identifier
  
  vector<const DGEMHit*> members; ///< DGEMHits that make up this cluster
  int plane;			  ///< GEM PLANE
  float q_tot;		          ///< total energy/charge deposited in the cluster
		
  /// Return a sensible string representation of this object
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Nmembers", "%d", members.size());
    AddString(items, "plane", "%d", plane);
    AddString(items, "q_tot", "%f", q_tot);
  }
};

#endif //DGEMSTRIPCLUSTER_H

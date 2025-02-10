
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
  double t_avg;               ///< average time for hits in cluster
		
  DVector3 pos;               /// center of cluster
  DVector3 width;
  DVector3 length;
		
  /// Return a sensible string representation of this object
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Nmembers", "%d", members.size());
    AddString(items, "plane", "%d", plane);
    AddString(items, "q_tot", "%f", q_tot);
    AddString(items, "t_avg", "%f", t_avg);
    AddString(items, "x", "%f", pos.x());
    AddString(items, "y", "%f", pos.y());
    AddString(items, "z", "%f", pos.z());
  }
};

#endif //DTRDSTRIPCLUSTER_H

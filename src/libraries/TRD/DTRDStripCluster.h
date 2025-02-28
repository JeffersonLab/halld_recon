
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
  double q_tot;		          ///< total energy/charge deposited in the cluster
  double t_avg;               ///< average time for hits in cluster
		
  DVector3 pos;               ///< position of the cluster in the plane
  DVector3 pos_max;           ///< position of the maximum energy hit in the cluster

  double q_max;
  double t_max;
 

  // can be removed if we don't use raw data clustering
  DVector3 width;             ///< width of the cluster in the plane
  DVector3 length;            ///< length of the cluster in the plane 
		
  /// Return a sensible string representation of this object
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Nmembers", "%d", members.size());
    AddString(items, "plane", "%d", plane);
    AddString(items, "q_tot", "%f", q_tot);
    AddString(items, "t_avg", "%f", t_avg);
    AddString(items, "x", "%f", pos.x());
    AddString(items, "y", "%f", pos.y());
    AddString(items, "z", "%f", pos.z());
    AddString(items, "q_max", "%f", q_max);
    AddString(items, "t_max", "%f", t_max);
    AddString(items, "x_max", "%f", pos_max.x());
    AddString(items, "y_max", "%f", pos_max.y());
    AddString(items, "z_max", "%f", pos_max.z());
  }
};

#endif //DTRDSTRIPCLUSTER_H

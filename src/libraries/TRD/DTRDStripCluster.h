
#ifndef DTRDSTRIPCLUSTER_H
#define DTRDSTRIPCLUSTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "DTRDHit.h"
#include <JANA/JObject.h>

#include <DVector3.h>

#define HIT_TIME_DIFF_MIN 10.0   // update this!

class DTRDStripCluster : public JObject 
{
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
		
  void Summarize(JObjectSummary& summary) const override {
    summary.add(members.size(), "Nmembers", "%d");
    summary.add(plane, "plane", "%d");
    summary.add(q_tot, "q_tot", "%f");
    summary.add(t_avg, "t_avg", "%f");
    summary.add(pos.x(), "x", "%f");
    summary.add(pos.y(), "y", "%f");
    summary.add(pos.z(), "z", "%f");
  }

};

#endif //DTRDSTRIPCLUSTER_H

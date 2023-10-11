
#ifndef DGEMSTRIPCLUSTER_H
#define DGEMSTRIPCLUSTER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "DGEMHit.h"
#include <JANA/JObject.h>

#define HIT_TIME_DIFF_MIN 10.0

class DGEMStripCluster : public JObject {
 public:
  JOBJECT_PUBLIC(DGEMStripCluster);		/// DANA identifier
  
  std::vector<const DGEMHit*> members; ///< DGEMHits that make up this cluster
  int plane;			  ///< GEM PLANE
  float q_tot;		          ///< total energy/charge deposited in the cluster
		
  /// Return a sensible string representation of this object
  void Summarize(JObjectSummary& summary) const override {
    summary.add(members.size(), "Nmembers", "%d");
    summary.add(plane, "plane", "%d");
    summary.add(q_tot, "q_tot", "%f");
  }
};

#endif //DGEMSTRIPCLUSTER_H

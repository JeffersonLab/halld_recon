// $Id$
//
//    File: DECALCluster.h
// Created: Tue Mar 25 10:45:18 AM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

#ifndef _DECALCluster_h_
#define _DECALCluster_h_

#include <JANA/JObject.h>

class DECALCluster:public JObject{
 public:
  JOBJECT_PUBLIC(DECALCluster);  
  DECALCluster(){};

  enum cluster_status_t{
    SHOWER_FOUND,  // Normal clusterization result
    EXTRA_PEAK, // extra peak found in set of hits
    SPLIT_CLUSTER, // Additional shower candidate added 
  };

  double E,Efit,t,x,y;
  int status,channel_Emax;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(x, "x [cm]", "%f");
    summary.add(y, "y [cm]", "%f");
    summary.add(E, "E [GeV]", "%f");
    summary.add(Efit, "E(fit) [GeV]", "%f");
    summary.add(t, "t [ns]", "%f");
    summary.add(status,"status","%d");
  }
};


#endif // _DECALCluster_h_


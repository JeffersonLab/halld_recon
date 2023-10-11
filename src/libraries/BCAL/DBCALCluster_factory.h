#ifndef _DBCALCluster_factory_
#define _DBCALCluster_factory_

/*
 *  DBCALCluster_factory.h
 *
 *  Created by Matthew Shepherd on 3/12/11.
 *
 */

#include <JANA/JFactoryT.h>

#include "BCAL/DBCALHit.h"
#include "BCAL/DBCALCluster.h"
#include "BCAL/DBCALUnifiedHit.h"
#include "BCAL/DBCALGeometry.h"
#include "TRACKING/DTrackWireBased.h"

#include "TF1.h"

class DBCALCluster_factory : public JFactoryT< DBCALCluster > {
  
public:
 
  DBCALCluster_factory();
  ~DBCALCluster_factory(){}
  
private:

  void Process(const std::shared_ptr<const JEvent>& event) override;	
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  
  void clearPoints();
  
  // these routines combine points and clusters together

  vector<DBCALCluster*> clusterize( vector< const DBCALPoint* > points, vector< const DBCALPoint* > usedPoints,  vector< const DBCALUnifiedHit* > hits, vector< const DTrackWireBased* > tracks ) const;
  void merge( vector<DBCALCluster*>& clusters, double point_reatten_E ) const;

  // This routine removes a point from its original cluster and adds it to its closest cluster if applicable.
  void recycle_points( vector<const DBCALPoint*> usedPoints, vector<DBCALCluster*>& clusters ) const; 

  // these are the routines used for testing whether things should be
  // combined -- right now very basic, but can be fine tuned in the future

  bool overlap( const DBCALCluster& highEClust,
                const DBCALCluster& lowEClust ) const;
  
  bool overlap( const DBCALCluster& clust,
                const DBCALPoint* point ) const;
 
  bool overlap_charged( const DBCALCluster& clust, 
			const DBCALPoint* point, float tracked_phi ) const;
 
  bool overlap( const DBCALCluster& clust, 
                const DBCALUnifiedHit* hit ) const; 
  
  uint32_t BCALCLUSTERVERBOSE;
  float m_mergeSig;
  float m_moliereRadius;
  float m_clust_hit_timecut;
  float m_timeCut;
  double m_z_target_center;

  const DBCALGeometry *m_BCALGeom;

  vector<double> effective_velocities;
  vector< vector<double > > attenuation_parameters;

  const DTrackFitter *fitter;
  
  /*
  TF1* sep_inclusion_curve;
  TF1* dtheta_inclusion_curve;
  TF1* dphi_inclusion_curve;
  TF1* C1_parm;
  TF1* C2_parm;
  */
  void Init() override;
  void Finish() override;
  
};

#endif 


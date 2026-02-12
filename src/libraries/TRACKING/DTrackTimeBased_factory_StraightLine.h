// $Id$
//
//    File: DTrackTimeBased_factory_StraightLine.h
// Created: Wed Mar 13 10:00:17 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DTrackTimeBased_factory_StraightLine_
#define _DTrackTimeBased_factory_StraightLine_

#include <JANA/JFactoryT.h>
#include "DTrackTimeBased.h"
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackWireBased.h>
#include <PID/DParticleID.h>

class DTrackTimeBased_factory_StraightLine:public JFactoryT<DTrackTimeBased>{
 public:
  DTrackTimeBased_factory_StraightLine(){
  	SetTag("StraightLine");
  };
  ~DTrackTimeBased_factory_StraightLine(){};

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void FilterDuplicates(void);
  void GetStartTime(const DTrackWireBased *track,
		    vector<const DSCHit*>&sc_hits,
		    vector<const DTOFPoint*>&tof_points,
		    vector<const DBCALShower*>&bcal_showers,	
		    vector<const DFCALShower*>&fcal_showers,
		    double &t0,DetectorSystem_t &t0_detector) const;

  int DEBUG_LEVEL=0;
  double CDC_MATCH_CUT,FDC_MATCH_CUT;

  // outer detector geom info
  double dFCALz,dTOFz,dDIRCz;

  // start counter geom info
  vector<vector<DVector3> >sc_dir; // direction vector in plane of plastic
  vector<vector<DVector3> >sc_pos;
  vector<vector<DVector3> >sc_norm;
  double SC_BARREL_R,SC_END_NOSE_Z,SC_PHI_SECTOR1;
  
  const DParticleID* dPIDAlgorithm;
  DTrackFitter *fitter;
  DTrackFinder *finder;

};

#endif // _DTrackTimeBased_factory_StraightLine_


// $Id$
//
//    File: DTrackCandidate_factory.h
// Created: Mon Jul 18 15:23:04 EDT 2005
// Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
//

#ifndef _DTrackCandidate_factory_
#define _DTrackCandidate_factory_


#include <JANA/JFactoryT.h>


#include <TH2F.h>
#include <TH1F.h>
#include "DTrackCandidate.h"
#include <DVector3.h>
#include "CDC/DCDCTrackHit.h"
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"
#include "FDC/DFDCSegment.h"
#include "DHelicalFit.h"
#include "DMagneticFieldStepper.h"
#include "PID/DParticleID.h"

class DMagneticFieldMap;
class DParticleID;

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="CORE.png" width="100">
///	</A>
/// \endhtmlonly

/// Form complete list of DTrackCandidate objects using the lists formed
/// from the CDC and FDCCathodes candidate factories (DTrackCandidate_factory_CDC
/// and DTrackCandidate_factory_FDCCathodes). 
///
/// Track finding starts by looking for candidates independently in the CDC
/// and FDC. The results of those first passes are used as input here where
/// a single list is made containijng all candidates.
///
/// This will attempt to identify any candidates that should be merged into a
/// single candidate, mainly if a both a CDC and FDC candidate were found for
/// the same track.
///
/// In addition, stray CDC hits that did not belong to any candidate are
/// merged into existing candidates if possible.

class DTrackCandidate_factory:public JFactoryT<DTrackCandidate>{
 public:
  DTrackCandidate_factory(){
    DEBUG_HISTS=false;
    //DEBUG_HISTS=true;
  };
  ~DTrackCandidate_factory(){};

 protected:
  void Init() override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  jerror_t DoRefit(DHelicalFit &fit,
		   vector<const DFDCSegment *>segments,
		   vector<const DCDCTrackHit *>cdchits,
		   double &Bz);
  void GetPositionAndMomentum(const DFDCSegment *segment,DVector3 &pos,
			      DVector3 &mom) const;
  jerror_t GetPositionAndMomentum(DHelicalFit &fit,double Bz,
				  const DVector3 &origin,DVector3 &pos,
				  DVector3 &mom) const;
  void GetPositionAndMomentum(double z,const DHelicalFit &fit,
			      double Bz,DVector3 &pos,DVector3 &mom) const;
  void GetPositionAndMomentum(const DHelicalFit &fit,double Bz,DVector3 &pos,
			      DVector3 &mom) const; 
  void UpdatePositionAndMomentum(DHelicalFit &fit,double Bz,
				 const DVector3 &origin,DVector3 &pos,
				 DVector3 &mom) const;
  
 private:
  const DMagneticFieldMap *bfield;
  bool dIsNoFieldFlag;
  DMagneticFieldStepper *stepper;
  const DParticleID* dParticleID;

  vector<const DTrackCandidate*>cdctrackcandidates;
  vector<const DTrackCandidate*>fdctrackcandidates; 
  vector<const DCDCTrackHit*>mycdchits;
  vector<DTrackCandidate *>trackcandidates;

  int DEBUG_LEVEL,MIN_NUM_HITS;
  bool DEBUG_HISTS,ADD_VERTEX_POINT;
  TH2F *match_dist,*match_dist_vs_p;
//  TH2F *match_center_dist2;

  double FactorForSenseOfRotation;
  DVector3 cdc_endplate;
  double endplate_rmax;
  double TARGET_Z;
  int MAX_NUM_TRACK_CANDIDATES; //used to avoid memory spikes: if this # is exceeded, delete all tracks //to disable, set = -1!!

  vector<vector<DVector3> >sc_pos;
  vector<vector<DVector3> >sc_norm;

};

#endif // _DTrackCandidate_factory_


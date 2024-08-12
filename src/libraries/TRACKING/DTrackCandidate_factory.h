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

  double DocaToHelix(const DCDCTrackHit *hit,double q,const DVector3 &pos,
		     const DVector3 &mom);
  double GetSenseOfRotation(DHelicalFit &fit,const DFDCPseudo *fdchit,const DVector3 &pos);
  jerror_t DoRefit(DHelicalFit &fit,
		   vector<const DFDCSegment *>segments,
		   vector<const DCDCTrackHit *>cdchits,
		   double &Bz);
  void ProjectHelixToZ(const double z,const double q,const DVector3 &mom,
		       DVector3 &pos);

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
  bool CheckZPosition(const DTrackCandidate *fdccan) const;
 
  // Various methods for matching CDC and FDC candidates
  bool MatchMethod1(const DTrackCandidate *fdccan,
		    vector<unsigned int> &cdc_forward_ids,
		    vector<DVector3>&cdc_endplate_projections,
		    vector<int>&cdc_forward_matches
		    );
  bool MatchMethod2(const DTrackCandidate *fdccan,const DTrackCandidate *cdccan
		    );
  bool MatchMethod3(const DTrackCandidate *cdccan,vector<int> &forward_matches
		    );  
  bool MatchMethod4(const DTrackCandidate *srccan,vector<int> &forward_matches,
		    int &num_fdc_cands_remaining);
  bool MatchMethod5(DTrackCandidate *can,  
		    vector<const DCDCTrackHit *>&cdchits,
		    vector<int> &forward_matches);
  void MatchMethod6(DTrackCandidate *can, 
		    vector<const DFDCPseudo *>&fdchits,
		    vector<unsigned int>&used_cdc_hits,  
		    unsigned int &num_unmatched_cdcs
		    );
  bool MatchMethod7(DTrackCandidate *srccan,vector<int> &forward_matches,
		    int &num_fdc_cands_remaining);
  bool MatchMethod8(const DTrackCandidate *cdccan,vector<int> &forward_matches);
  bool MatchMethod9(unsigned int src_index,const DTrackCandidate *srccan, 
		    const DFDCSegment *segment,
		    vector<const DTrackCandidate*>&cands,
		    vector<int> &forward_matches);
  bool MatchMethod10(unsigned int src_index,const DTrackCandidate *srccan, 
		     const DFDCSegment *segment,
		     vector<const DTrackCandidate*>&cands,
		     vector<int> &forward_matches);
  bool MatchMethod11(double q,DVector3 &mypos,DVector3 &mymom,
		     DHelicalFit &fit2,const DFDCSegment *segment1,
		     const DFDCSegment *segment2);
  bool MatchMethod12(DTrackCandidate *srccan,vector<int> &forward_matches,
		     int &num_fdc_cands_remaining);  
  bool MatchMethod13(unsigned int src_index,const DTrackCandidate *srccan, 
		     const DFDCSegment *segment,
		     vector<const DTrackCandidate*>&cands,
		     vector<int> &forward_matches);
  bool TryToFlipDirection(vector<const DSCHit *>&scihits,
			  DVector3 &mom,DVector3 &pos) const;
  bool MatchStraySegments(vector<int> &forward_matches,
			  int &num_fdc_cands_remaining);
  bool MakeCandidateFromMethod1(double theta,
				vector<const DFDCSegment *>&segments,
				const DTrackCandidate *cdccan);
 
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


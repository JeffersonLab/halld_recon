// $Id$
//
//    File: DTrackCandidate_factory_FDCCathodes.h
// Created: Tue Nov  6 13:37:08 EST 2007
// Creator: staylor (on Linux ifarml1.jlab.org 2.4.21-47.0.1.ELsmp i686)
//

#ifndef _DTrackCandidate_factory_FDCCathodes_
#define _DTrackCandidate_factory_FDCCathodes_

#include <JANA/JFactory.h>
#include "DTrackCandidate.h"
#include <DTrackFinder.h>
#include <DMatrix.h>
#include "FDC/DFDCSegment_factory.h"
#include "HDGEOMETRY/DMagneticFieldMap.h"
#include "DMagneticFieldStepper.h"
#include <TH1F.h>
#include <TH2F.h>
#include "DHelicalFit.h"

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="CORE.png" width="100">
///	</A>
/// \endhtmlonly

/// Find track candidates in the Forward Drift Chambers (FDC).
/// This will try to form candidates from hits found in the FDC.
/// Roughly, clusters of hits in single packages (6 wire planes)
/// are found and fit. The fit is propagated to adjacent packages
/// to link them up with hit clusters there. Eventually, all hits
/// are used to fit using a Reimann method to obtain the parameters
/// of the candidate.
///
/// These candidates will be merged with those from the FDC in
/// the DTrackCandidate_factory class.

class DTrackCandidate_factory_FDCCathodes:public JFactory<DTrackCandidate>{
 public:
  DTrackCandidate_factory_FDCCathodes(){
    DEBUG_HISTS = false;
    //DEBUG_HISTS = true;
  };
  ~DTrackCandidate_factory_FDCCathodes(){};
  const char* Tag(void){return "FDCCathodes";}
  
 private:  
  const DMagneticFieldMap *bfield;
  DMagneticFieldStepper *stepper;
		
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  jerror_t GetPositionAndMomentum(const DFDCSegment *segment);
  jerror_t GetPositionAndMomentum(DVector3 &pos,DVector3 &mom);
  jerror_t GetPositionAndMomentum(vector<const DFDCSegment *>segments,
				  DVector3 &pos,DVector3 &mom); 
  jerror_t GetPositionAndMomentum(const DFDCSegment *segment,
				  DVector3 &pos,DVector3 &mom);
  
  double GetCharge(const DVector3 &pos,const DFDCSegment *segment);
  double GetCharge(const DVector3 &pos,vector<const DFDCSegment *>segments);

  double DocaSqToHelix(const DFDCPseudo *hit);
  double DocaSqToHelix(const DFDCSegment *segment1,const DFDCSegment *segment2) const;
  double DocaSqToHelix(const DTrackCandidate *candidate,const DFDCPseudo *hit) const;

  DFDCSegment *GetTrackMatch(DFDCSegment *segment,vector<DFDCSegment*>package,
			     unsigned int &match_id);
  void LinkSegments(unsigned int pack1,vector<DFDCSegment *>packages[4],
		    vector<pair<const DFDCSegment*,const DFDCSegment*> >&paired_segments, vector<vector<int> >&is_paired); 
  double Match(double p);
  double MatchR(double rc) const;

  bool GetTrackMatch(double q,DVector3 &pos,DVector3 &mom,
		     const DFDCSegment *segment);
  bool LinkStraySegment(const DFDCSegment *segment);
  bool LinkSegmentsHough(vector<pair<unsigned int,unsigned int> >&unused_segements,
			 vector<DFDCSegment *>packages[4],
			 vector<vector<int> >&is_paired,
			 vector<vector<const DFDCSegment *>>&mytracks);
  void MakeCandidate(vector<const DFDCSegment *>&mytrack);
  void DoHelicalFit(vector<const DFDCSegment *>&mytrack,DHelicalFit &fit);


  bool DEBUG_HISTS,USE_FDC,ADD_VERTEX_POINT;

  TH2F *match_dist_fdc,*match_center_dist2;
 
  vector<double>z_wires;
  double TARGET_Z,BEAM_VAR,FDC_HOUGH_THRESHOLD;
  
  double FactorForSenseOfRotation;
  
  // Fit parameters
  double xc,yc,rc,q,tanl;
  
  // Parameters at the end of the segment
  double xs,ys,zs;
  double p,cosphi,sinphi,twokappa,cotl;

  DTrackFinder *finder;

  // For profiling
  bool PROFILE_TIME;
  unsigned int NUM_PROFILE_FDC_CANDIDATES;
  double cumulative_time=0.,cumulative_events=0.;
};

inline double DTrackCandidate_factory_FDCCathodes::Match(double p){
  double cut=5.5/p;
  if (cut>9.0) cut=9.0;
  if (cut<5.) cut=5.0;
  return cut;
}
inline double DTrackCandidate_factory_FDCCathodes::MatchR(double rc) const {
  double cut=200.0/rc;
  if (cut>9.0) cut=9.0;
  if (cut<5.) cut=5.0;
  return cut;
}

#endif // _DTrackCandidate_factory_FDCCathodes_

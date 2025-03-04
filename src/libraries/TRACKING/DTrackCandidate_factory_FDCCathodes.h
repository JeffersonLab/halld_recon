// $Id$
//
//    File: DTrackCandidate_factory_FDCCathodes.h
// Created: Tue Nov  6 13:37:08 EST 2007
// Creator: staylor (on Linux ifarml1.jlab.org 2.4.21-47.0.1.ELsmp i686)
//

#ifndef _DTrackCandidate_factory_FDCCathodes_
#define _DTrackCandidate_factory_FDCCathodes_

#include <JANA/JFactoryT.h>
#include "DTrackCandidate.h"
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

class DTrackCandidate_factory_FDCCathodes:public JFactoryT<DTrackCandidate>{
 public:
  DTrackCandidate_factory_FDCCathodes(){
  	SetTag("FDCCathodes");
    DEBUG_HISTS = false;
    //DEBUG_HISTS = true;
  };
  ~DTrackCandidate_factory_FDCCathodes(){};

 private:  
  const DMagneticFieldMap *bfield;
  DMagneticFieldStepper *stepper;
		
  //void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  jerror_t GetPositionAndMomentum(const DFDCSegment *segment);
  jerror_t GetPositionAndMomentum(DVector3 &pos,DVector3 &mom);
  jerror_t GetPositionAndMomentum(vector<const DFDCSegment *>segments,
				  DVector3 &pos,DVector3 &mom); 
  jerror_t GetPositionAndMomentum(const DFDCSegment *segment,
				  DVector3 &pos,DVector3 &mom);
  
  double GetCharge(const DVector3 &pos,const DFDCSegment *segment);
  double GetCharge(const DVector3 &pos,vector<const DFDCSegment *>segments);

  double DocaSqToHelix(const DFDCPseudo *hit);
  DFDCSegment *GetTrackMatch(DFDCSegment *segment,vector<DFDCSegment*>package,
			     unsigned int &match_id);
  void LinkSegments(unsigned int pack1,vector<DFDCSegment *>packages[4],
		    vector<pair<const DFDCSegment*,const DFDCSegment*> >&paired_segments, vector<vector<int> >&is_paired); 
  double Match(double p);

  bool GetTrackMatch(double q,DVector3 &pos,DVector3 &mom,
		     const DFDCSegment *segment);
  bool LinkStraySegment(const DFDCSegment *segment);
  bool LinkSegmentsHough(vector<pair<unsigned int,unsigned int> >&unused_segements,
			 vector<DFDCSegment *>packages[4],
			 vector<vector<int> >&is_paired);


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
};

inline double DTrackCandidate_factory_FDCCathodes::Match(double p){
  double cut=5.5/p;
  if (cut>9.0) cut=9.0;
  if (cut<5.) cut=5.0;
  return cut;
}


#endif // _DTrackCandidate_factory_FDCCathodes_


// $Id$
//
//    File: DTrackCandidate.h
// Created: Sun Apr  3 12:38:16 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DTrackCandidate_
#define _DTrackCandidate_

#include <JANA/JObject.h>

#include <TRACKING/DTrackingData.h>
#include <TRACKING/DTrackFitter.h>

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="CORE.png" width="100">
///	</A>
/// \endhtmlonly

/// DTrackCandidate objects are the result of track finding and the
/// input to track fitting. Several algorithms exist for making
/// these and then merging them. For the default, see
/// DTrackCandidate_factory .

class DTrackCandidate : public JObject {
public:
  JOBJECT_PUBLIC(DTrackCandidate);
  
  DTrackCandidate():chisq(0),Ndof(0){}
  
  float chisq;			///< Chi-squared for the track (not chisq/dof!)
  int Ndof;				///< Number of degrees of freedom in the fit
		
  vector<int>used_cdc_indexes;
  vector<int>used_fdc_indexes;
  
  // Circle fit data
  double xc,yc,rc;

  // charge
  double dCharge;

  // Kinematic data
  DVector3 dPosition;
  DVector3 dMomentum;
  
  // Rough timing information
  double dMinimumDriftTime;
  DetectorSystem_t dDetector;

  // Range of packages used in candidate in FDC
  unsigned FirstPackage,LastPackage;

  // Hits used in track
  vector<const DFDCPseudo*>fdchits;
  vector<const DCDCTrackHit*>cdchits;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(SystemName(dDetector), "Detector", "%s");
    summary.add(dCharge, "Charge", "%f");
    summary.add(dMomentum.x(), "px [GeV/c]", "%f");
    summary.add(dMomentum.y(), "py [GeV/c]", "%f");
    summary.add(dMomentum.z(), "pz [GeV/c]", "%f");
    summary.add(dPosition.x(), "x [cm]", "%f");
    summary.add(dPosition.y(), "y [cm]", "%f");
    summary.add(dPosition.z(), "z [cm]", "%f");
    summary.add(dMinimumDriftTime, "t [ns]", "%f");
    summary.add(chisq, "chisq", "%f");
    summary.add(Ndof, "Ndof", "%d");
  }
};

#endif // _DTrackCandidate_


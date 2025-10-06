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

class DReferenceTrajectory;

#define MAX_IHITS 256

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="CORE.png" width="100">
///	</A>
/// \endhtmlonly

/// DTrackCandidate objects are the result of track finding and the
/// input to track fitting. Several algorithms exist for making
/// these and then merging them. For the default, see
/// DTrackCandidate_factory .

class DTrackCandidate:public DTrackingData{
	public:
		JOBJECT_PUBLIC(DTrackCandidate);
		
		DTrackCandidate():chisq(0),Ndof(0),rt(0),IsSmoothed(0){}

		float chisq;			///< Chi-squared for the track (not chisq/dof!)
		int Ndof;				///< Number of degrees of freedom in the fit

		const DReferenceTrajectory *rt; ///< pointer to reference trjectory representing this track (if any)
		
		vector<DTrackFitter::pull_t> pulls; // vector of residuals and other track-related quantities 
		
		vector<int>used_cdc_indexes;
		vector<int>used_fdc_indexes;

		// Circle fit data
		double xc,yc,rc;

      bool IsSmoothed; // Boolean value to indicate whether the smoother was run succesfully over this track.

		void Summarize(JObjectSummary& summary) const override {
			DKinematicData::Summarize(summary);
			summary.add(chisq, "chisq", "%f");
			summary.add(Ndof, "Ndof", "%d");
		}
};

#endif // _DTrackCandidate_


// $Id$
//
//    File: DTrackTimeBased.h
// Created: Thu Sep  4 14:02:44 EDT 2008
// Creator: davidl (on Darwin harriet.jlab.org 8.11.1 i386)
//

#ifndef _DTrackTimeBased_
#define _DTrackTimeBased_

#include <JANA/JObject.h>
#include <DANA/DObjectID.h>
#include "DTrackingData.h"
#include "DTrackFitter.h"
#include "CDC/DCDCTrackHit.h"
#include "FDC/DFDCPseudo.h"

using namespace std;

class DTrackTimeBased:public DTrackingData{
	public:
		JOBJECT_PUBLIC(DTrackTimeBased);
		
		double dEdx(void) const{return ((dNumHitsUsedFordEdx_CDC >= dNumHitsUsedFordEdx_FDC) ? ddEdx_CDC_amp : ddEdx_FDC);}

		unsigned int measured_fdc_hits_on_track;
		unsigned int measured_cdc_hits_on_track;
		unsigned int potential_fdc_hits_on_track;
		unsigned int potential_cdc_hits_on_track;

		oid_t id = reinterpret_cast<oid_t>(this);
		oid_t trackid;			///< id of DTrackWireBased corresponding to this track
		oid_t candidateid;   ///< id of DTrackCandidate corresponding to this track
		float chisq;			///< Chi-squared for the track (not chisq/dof!)
		int Ndof;				///< Number of degrees of freedom in the fit
		vector<DTrackFitter::pull_t> pulls;	///< Holds pulls used in chisq calc. (not including off-diagonals)
		map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >extrapolations;
		int flags;
		enum DTrackTimeBased_flag_t{
		  FLAG__GOODFIT=0,
		  FLAG__USED_WIREBASED_FIT=1,
		  FLAG__USED_OTHER_HYPOTHESIS=2,
		};
		

		bool GetProjection(DetectorSystem_t detector,DVector3 &pos,
				   DVector3 *mom=nullptr,double *t=nullptr) const;


      bool IsSmoothed; // Boolean value to indicate whether the smoother was run succesfully over this track.

		typedef struct{
		  double t0,t0_sigma;
		  DetectorSystem_t system;
		}DStartTime_t;
		vector<DStartTime_t>start_times;

		double FOM;

		double ddEdx_FDC;
		double ddx_FDC;
		unsigned int dNumHitsUsedFordEdx_FDC;
		double ddEdx_CDC,ddEdx_CDC_amp;
		double ddx_CDC,ddx_CDC_amp;
		unsigned int dNumHitsUsedFordEdx_CDC;

		// Add vectors for truncated means in the DC dEdx track parameters.
		// If these vectors are empty then the truncated means are not available,
		// otherwise ddx_FDC_trunc[1] is the dx sum for the FDC track hits with
		// the single hit with the highest dEdx removed, [2] with the two hits
		// with the highest dEdx removed, and so on, for as many as you find.
		std::vector<double> ddx_FDC_trunc;  
		std::vector<double> ddEdx_FDC_trunc;  
		std::vector<double> ddx_FDC_amp_trunc;  
		std::vector<double> ddEdx_FDC_amp_trunc;  
		std::vector<double> ddx_CDC_trunc;  
		std::vector<double> ddEdx_CDC_trunc;  
		std::vector<double> ddx_CDC_amp_trunc;  
		std::vector<double> ddEdx_CDC_amp_trunc;  

		// Hit CDC Rings & FDC Planes
		// use the DParticleID Get_CDCRings & Get_FDCPlanes functions to extract the information from these
		unsigned int dCDCRings; //CDC rings where the track has an associated DCDCTrackHit //rings correspond to bits (1 -> 28)
		unsigned int dFDCPlanes; //FDC planes where the track has an associated DFDCPseudoHit //planes correspond to bits (1 -> 24)

		// Matching to MC: Highest % of track hits matched to a thrown
		int dMCThrownMatchMyID; //MC track match myid (-1 if somehow no match)
		int dNumHitsMatchedToThrown;

		void Summarize(JObjectSummary& summary) const override {
			DKinematicData::Summarize(summary);
			summary.add(candidateid, "candidate", "%d");
			//summary.add(trackid, "wirebased", "%d");
			summary.add(chisq, "chisq", "%f");
			summary.add(Ndof, "Ndof", "%d");
			summary.add((float)FOM, "FOM", "%f");
			summary.add(flags, "Flags", "%d");
			//summary.add(dMCThrownMatchMyID, "MCMatchID", "%d");
			//summary.add(dNumHitsMatchedToThrown, "#HitsMCMatched", "%d");
		}
};

size_t Get_NumTrackHits(const DTrackTimeBased* locTrackTimeBased);
inline size_t Get_NumTrackHits(const DTrackTimeBased* locTrackTimeBased)
{
	vector<const DCDCTrackHit*> locCDCHits = locTrackTimeBased->Get<DCDCTrackHit>();
	vector<const DFDCPseudo*> locFDCHits = locTrackTimeBased->Get<DFDCPseudo>();

	size_t locNumHits = locCDCHits.size() + locFDCHits.size();
	if(locNumHits > 0)
		return locNumHits;

	return locTrackTimeBased->Ndof + 5; //is WRONG because FDC DoF != FDC Hits
}

inline bool DTrackTimeBased::GetProjection(DetectorSystem_t detector,
					   DVector3 &pos,
					   DVector3 *mom,double *t) const{
  if (extrapolations.at(detector).size()>0){
    DTrackFitter::Extrapolation_t extrapolation=extrapolations.at(detector)[0];
    pos=extrapolation.position;
    if (mom){
      *mom=extrapolation.momentum;
    }
    if (t){
      *t=extrapolation.t;
    }
    return true;
  }

  
  return false;
}

#endif // _DTrackTimeBased_


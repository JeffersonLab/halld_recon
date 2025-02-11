// $Id: DTrackWireBased_factory.h 5569 2009-10-02 22:27:08Z staylor $
//
//    File: DTrackWireBased_factory.h
// Created: Wed Sep  3 09:33:40 EDT 2008
// Creator: davidl (on Darwin harriet.jlab.org 8.11.1 i386)
//

#ifndef _DTrackWireBased_factory_
#define _DTrackWireBased_factory_

#include <JANA/JFactoryT.h>
#include <JANA/JObject.h>

#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackHitSelector.h>
#include "PID/DParticleID.h"
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"


#include <TH2.h>
#include <TH1.h>

class DTrackCandidate;
class DParticleID;

#include "DTrackWireBased.h"

///////////////////////////////////////////////////////////////////////
/// The DTrackWireBased_factory class coordinates the fitting of wire-based
/// tracks. While the hit selection is done here, the actual heavy
/// lifting of the fit is done by the DTrackFitter class (or, more
/// specifically, a class that inherits from DTrackFitter).
///
/// This grabs a DTrackFitter object using the default Tag through JANA
/// and uses it to fit the DTrackCandidate objects which it grabs
/// also using the default Tag.
/// 
/// The DTrackWireBased objects are wire-based tracks (no drift time 
/// information is used). As such, this is hardwired to set the fit type for 
/// the DTrackFitter to kWireBased. See the DTrackTimeBased classes for the
/// time-based counterpart.
///
/// This may appear uneccessarily complex, but it provides for using the
/// exact same code for fitting both wire-based and time-based tracks
/// as well as allowing a lot of flexibility in swapping out the 
/// DTrackFitter class used by both the wire-based and time-based 
/// stages using the same DEFTAG mechanism used by the rest of JANA.
///////////////////////////////////////////////////////////////////////

class DTrackWireBased_factory:public JFactoryT<DTrackWireBased>{
	public:
		DTrackWireBased_factory(){};
		~DTrackWireBased_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		int DEBUG_LEVEL;
		DTrackFitter *fitter;
		DReferenceTrajectory *rt;

		vector<int> mass_hypotheses_positive;
		vector<int> mass_hypotheses_negative;
		size_t MAX_DReferenceTrajectoryPoolSize; 
		int mNumHypPlus,mNumHypMinus;

		void FilterDuplicates(void);
		void DoFit(unsigned int c_id,const DTrackCandidate *candidate,
			   DReferenceTrajectory *rt, const std::shared_ptr<const JEvent>& event,
			   double mass);
		void AddMissingTrackHypothesis(vector<DTrackWireBased*>&tracks_to_add,
					       const DTrackWireBased *src_track,
					       double my_mass,double q);
		bool InsertMissingHypotheses(void);
		void CorrectForELoss(DVector3 &position,DVector3 &momentum,
				     double q, double mass); 
		void AddMissingTrackHypotheses(unsigned int mass_bits,
					       vector<DTrackWireBased*>&tracks_to_add,
					       vector<DTrackWireBased *>&hypotheses,
					       double q);

 
		const DGeometry *geom;

		bool DEBUG_HISTS;
		bool SKIP_MASS_HYPOTHESES_WIRE_BASED;
		bool USE_HITS_FROM_CANDIDATE;
		double MIN_FIT_P;
		double PROTON_MOM_THRESH;
		bool dIsNoFieldFlag;

		const DParticleID* dPIDAlgorithm;
};

#endif // _DTrackWireBased_factory_


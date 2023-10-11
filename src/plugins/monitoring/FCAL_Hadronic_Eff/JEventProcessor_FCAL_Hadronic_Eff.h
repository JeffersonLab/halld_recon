// $Id$
//
// File: JEventProcessor_FCAL_Hadronic_Eff.h
//

#ifndef _JEventProcessor_FCAL_Hadronic_Eff_
#define _JEventProcessor_FCAL_Hadronic_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TH1I.h"
#include "TH2I.h"

#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"

#include "FCAL/DFCALShower.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
#include "PID/DParticleID.h"
#include "PID/DDetectorMatches.h"
#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DTreeInterface.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <thread>

using namespace std;

class JEventProcessor_FCAL_Hadronic_Eff : public JEventProcessor
{
	public:
		JEventProcessor_FCAL_Hadronic_Eff(){
			SetTypeName("JEventProcessor_FCAL_Hadronic_Eff");
		};
		~JEventProcessor_FCAL_Hadronic_Eff(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		double Calc_FCALTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DParticleID* locParticleID, const DEventRFBunch* locEventRFBunch, double& locDeltaT);
		bool Cut_TOFTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis);

		//TRACK REQUIREMENTS
		double dMaxTOFDeltaT;
		double dMinTrackingFOM;
		unsigned int dMinNumTrackHits;
		int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;
		double dMaxFCALThetaCut;
		double dMaxVertexR;
		DCutAction_TrackHitPattern* dCutAction_TrackHitPattern;

		//HISTOGRAMS
		TH2I* dHist_TrackFCALYVsX_HasHit;
		TH2I* dHist_TrackFCALYVsX_TotalHit;
		TH2I* dHist_TrackFCALRowVsColumn_HasHit;
		TH2I* dHist_TrackFCALRowVsColumn_TotalHit;

		//TREE
		DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
			//important: manages it's own data internally: don't want to call new/delete every event!
		static thread_local DTreeFillData dTreeFillData;
};

#endif // _JEventProcessor_FCAL_Hadronic_Eff_


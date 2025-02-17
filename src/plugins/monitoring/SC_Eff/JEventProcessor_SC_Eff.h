// $Id$
//
// File: JEventProcessor_SC_Eff.h
//

#ifndef _JEventProcessor_SC_Eff_
#define _JEventProcessor_SC_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TH1I.h"
#include "TH2I.h"

#include "START_COUNTER/DSCHit.h"
#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"

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

class JEventProcessor_SC_Eff : public JEventProcessor
{
	public:
		JEventProcessor_SC_Eff(){
			SetTypeName("JEventProcessor_SC_Eff");
		};
		~JEventProcessor_SC_Eff(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		bool Cut_PIDDeltaT(const DChargedTrackHypothesis* locChargedTrackHypothesis);

		//TRACK REQUIREMENTS
		double dMinTrackingFOM;
		unsigned int dMinNumTrackHits;
		double dMaxVertexR;
		int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;
		DCutAction_TrackHitPattern* dCutAction_TrackHitPattern;
		map<DetectorSystem_t, double> dMaxPIDDeltaTMap;

		//HISTOGRAMS
		TH2I* dHist_HitFound;
		TH2I* dHist_HitTotal;

		//TREE
		DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
			//important: manages it's own data internally: don't want to call new/delete every event!
		static thread_local DTreeFillData dTreeFillData;
};

#endif // _JEventProcessor_SC_Eff_


// $Id$
//
// File: JEventProcessor_TOF_Eff.h
//

#ifndef _JEventProcessor_TOF_Eff_
#define _JEventProcessor_TOF_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TH1I.h"
#include "TH2I.h"

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

class JEventProcessor_TOF_Eff : public JEventProcessor
{
	public:
		JEventProcessor_TOF_Eff(){
			SetTypeName("JEventProcessor_TOF_Eff");
		};
		~JEventProcessor_TOF_Eff(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		int Calc_NearestHit(const DTOFPaddleHit* locPaddleHit) const;
		bool Cut_FCALTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DParticleID* locParticleID, const DEventRFBunch* locEventRFBunch);
		double Calc_TOFTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DParticleID* locParticleID, const DEventRFBunch* locEventRFBunch, double& locDeltaT);

		//TRACK REQUIREMENTS
		double dMaxFCALDeltaT;
		double dMinTrackingFOM;
		unsigned int dMinNumTrackHits;
		double dMaxTOFThetaCut;
		double dMaxVertexR;
		int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;
		DCutAction_TrackHitPattern* dCutAction_TrackHitPattern;

		//HISTOGRAMS
		//DTOFPaddle
		double dMinTOFPaddleMatchDistance;
		TH2I* dHist_TOFPaddleTrackYVsVerticalPaddle_HasHit_Top;
		TH2I* dHist_TOFPaddleTrackYVsVerticalPaddle_TotalHit_Top;
		TH2I* dHist_TOFPaddleHorizontalPaddleVsTrackX_HasHit_North;
		TH2I* dHist_TOFPaddleHorizontalPaddleVsTrackX_TotalHit_North;

		TH2I* dHist_TOFPaddleTrackYVsVerticalPaddle_HasHit_Bottom;
		TH2I* dHist_TOFPaddleTrackYVsVerticalPaddle_TotalHit_Bottom;
		TH2I* dHist_TOFPaddleHorizontalPaddleVsTrackX_HasHit_South;
		TH2I* dHist_TOFPaddleHorizontalPaddleVsTrackX_TotalHit_South;

		//TOFPoint
		TH2I* dHist_TrackTOFYVsX_HasHit;
		TH2I* dHist_TrackTOFYVsX_TotalHit;
		TH2I* dHist_TrackTOF2DPaddles_HasHit;
		TH2I* dHist_TrackTOF2DPaddles_TotalHit;



		//TREE
		DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
			//important: manages it's own data internally: don't want to call new/delete every event!
		static thread_local DTreeFillData dTreeFillData;
};

#endif // _JEventProcessor_TOF_Eff_


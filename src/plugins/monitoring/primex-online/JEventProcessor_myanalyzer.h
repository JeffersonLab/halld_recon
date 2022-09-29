/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2022       GlueX and PrimEX-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#ifndef _JEventProcessor_myanalyzer_
#define _JEventProcessor_myanalyzer_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include "TTree.h"
#include "TH1.h"
#include "TH2.h"

#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>

#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>
#include "PID/DVertex.h"
#include <PID/DNeutralParticle.h>
#include <CCAL/DCCALGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <START_COUNTER/DSCHit.h>
#include "TRACKING/DMCThrown.h"
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DChargedTrack.h>
#include "TRACKING/DTrackTimeBased.h"
#include "HistogramTools.h"
#include "PID/DMCReaction.h"
#include <PID/DDetectorMatches.h>

#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "TRandom3.h"

#include "Combination.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace jana;
using namespace std;



class JEventProcessor_myanalyzer:public jana::JEventProcessor{
	public:
		JEventProcessor_myanalyzer()  {};
		~JEventProcessor_myanalyzer() {};
		const char* className(void){return "JEventProcessor_myanalyzer";}

	private:
		jerror_t init(void);
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);
		jerror_t erun(void);
		jerror_t fini(void);
		
		int fcalLayer(int row, int col);
		
		jerror_t FillParticleVectors(vector<const DChargedTrack *>&tracks,
					     vector<const DTrackTimeBased *>&pims,
					     vector<const DTrackTimeBased *>&pips);
		
		void Combined6g(vector<const DNeutralParticleHypothesis *>&EMList,
				Double_t &bestChi2Eta,
				Double_t &bestChi2EtaPrim,
				vector<int>&Index6gList,
				vector<int>&Indexprim6gList);
		
		Combination  *combi6;       

		double TargetMass;
		double m_beamX, m_beamY, m_beamZ;
		double m_fcalX, m_fcalY, m_fcalZ;
		double m_ccalX, m_ccalY, m_ccalZ;
		
		const double m_pi0 =  0.1349770;
		const double m_eta =  0.547862;
		const double massBe9 = 8.39479;
		const double massHe4 = 3.727379238;
		
		const double me = 0.510998928e-3;
		const double mp = 0.93827208816;
		const double m_etapr = 0.95778;  
				
		const double FCAL_RF_CUT       =  3.0;
		const double CCAL_RF_CUT       =  3.0;
		const double BCAL_RF_CUT       =  6.012;
		const double TOF_RF_CUT       =  6.5;
		const double BEAM_RF_CUT       =  2.004;
		
		const double MIN_FCAL_ENERGY   =  0.5;
		const double MIN_BEAM_ENERGY   =  8.0;
				
};

#endif // _JEventProcessor_myanalyzer_


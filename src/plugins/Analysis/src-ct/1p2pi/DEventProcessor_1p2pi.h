#ifndef _DEventProcessor_1p1pi1pi0_
#define _DEventProcessor_1p1pi1pi0_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include <JANA/JEventProcessor.h>
#include <TRIGGER/DTrigger.h>
#include <PID/DBeamPhoton.h>

#include "ANALYSIS/DAnalysisUtilities.h"
#include "ANALYSIS/DTreeInterface.h"
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>

#include <PID/DChargedTrack.h>
#include <PID/DParticleID.h>
#include <TH1F.h>

#include "TProfile.h"
#include <TTree.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>



class DEventProcessor_1p2pi:public JEventProcessor{
	public:
		DEventProcessor_1p2pi(){};
		~DEventProcessor_1p2pi(){};
		const char* className(void){return "DEventProcessor_1p2pi";}

	private:
		jerror_t init(void);	
		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	
		jerror_t erun(void);	
		jerror_t fini(void);

		DKinFitUtils_GlueX *dKinFitUtils;
		DKinFitter *dKinFitter;
		const DAnalysisUtilities* dAnalysisUtilities;


  		void GetHypotheses(vector<const DChargedTrack *> &tracks,
		     map<Particle_t, int> &particles,
		     map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
		     vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
	        ) const;



  //TREE
  DTreeInterface* dTreeInterface;
  static thread_local DTreeFillData dTreeFillData;

};

#endif // _DEventProcessor_1p1pi2gamma_



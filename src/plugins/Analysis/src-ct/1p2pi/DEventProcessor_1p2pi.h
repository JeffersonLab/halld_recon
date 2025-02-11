#ifndef _DEventProcessor_1p1pi1pi0_
#define _DEventProcessor_1p1pi1pi0_

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
	DEventProcessor_1p2pi(){ SetTypeName("DEventProcessor_1p2pi"); };
	~DEventProcessor_1p2pi() = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::mutex m_mutex;

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

#endif // _DEventProcessor_1p1pi1pi0_



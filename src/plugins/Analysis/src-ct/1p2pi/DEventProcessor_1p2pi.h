// $Id$
//
//    File: DEventProcessor_1p2pi.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_1p2pi_
#define _DEventProcessor_1p2pi_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include "TApplication.h"
#include "TCanvas.h"


#include <JANA/JEventProcessor.h>
#include <TRIGGER/DTrigger.h>
#include <PID/DBeamPhoton.h>
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
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		
		void GetHypotheses(vector<const DChargedTrack *> &tracks,
                   	   //vector<Particle_t> &particles,
                           map<Particle_t, int> &particles,
                  	   map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
                  	    vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
                ) const;


		TTree* T;
		TTree* RunInformation;
		Int_t fcal_ncl; 
	
		Float_t fcal_en_cl, fcal_x_cl, fcal_y_cl;
		Float_t mass_piplus, mass_piminus, massrho;

};

#endif // _DEventProcessor_1p2pi_


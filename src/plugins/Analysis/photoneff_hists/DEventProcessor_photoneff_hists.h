// $Id$
//
//    File: DEventProcessor_photoneff_hists.h
// Created: Thu Feb 12 09:43:13 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DEventProcessor_photoneff_hists_
#define _DEventProcessor_photoneff_hists_

#include <pthread.h>
#include <map>
using std::map;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <PID/DKinematicData.h>
#include <PID/DPhoton.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrajectoryPoint.h>

#include "photon.h"


class DEventProcessor_photoneff_hists:public JEventProcessor{

	public:
		DEventProcessor_photoneff_hists();
		~DEventProcessor_photoneff_hists();

		TTree *phtneff;
		photon phtn;
		photon *phtn_ptr;


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		bool isReconstructable(const DMCThrown *mcthrown, const std::shared_ptr<const JEvent>& loop);

		pthread_mutex_t mutex;
		
		int DEBUG;
		
};

#endif // _DEventProcessor_photoneff_hists_


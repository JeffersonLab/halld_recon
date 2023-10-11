// $Id$
//
//    File: DEventProcessor_phys_tree.h
// Created: Wed Sep  2 20:25:05 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DEventProcessor_phys_tree_
#define _DEventProcessor_phys_tree_

#include <vector>
using namespace std;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <PID/DKinematicData.h>
#include <PID/DChargedTrack.h>
#include <PID/DNeutralParticle.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DMCTrackHit.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DMCThrown.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCHit.h>
#include <PID/DVertex.h>

#include "Event.h"
#include "Particle.h"

class DMCTrajectoryPoint;
class DCoordinateSystem;

class DEventProcessor_phys_tree:public JEventProcessor{
	public:
		DEventProcessor_phys_tree();
		~DEventProcessor_phys_tree();

		class particle_set{
			public:
				vector<Particle> photons;
				vector<Particle> neutrons;
				vector<Particle> piplus;
				vector<Particle> piminus;
				vector<Particle> protons;
				vector<Particle> Kplus;
				vector<Particle> Kminus;
		};

		Event *evt_recon;
		Event *evt_thrown;
		TTree *tree_recon;
		TTree *tree_thrwn;

		pthread_mutex_t mutex;
		
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		Particle MakeParticle(const DKinematicData *kd, double mass);
		Particle MakeParticle(const DChargedTrackHypothesis *locChargedTrackHypothesis, double mass);
		Particle MakeParticle(const DNeutralParticleHypothesis *locNeutralParticleHypothesis, double mass);
		bool IsFiducial(const DKinematicData *kd);
		void FillEvent(Event *evt, particle_set &pset, particle_set &pset_match);
		Particle FindBestMatch(const Particle &primary, vector<Particle> &secondaries);
		double GetFOM(const Particle &a, const Particle &b) const;
};

#endif // _DEventProcessor_trackres_tree_


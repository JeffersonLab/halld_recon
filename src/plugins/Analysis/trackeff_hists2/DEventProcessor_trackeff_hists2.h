// $Id$
//
//    File: DEventProcessor_trackeff_hists2.h
// Created: Wed Oct 10 13:30:37 EDT 2007
// Creator: davidl (on Darwin fwing-dhcp95.jlab.org 8.10.1 i386)
//

#ifndef _DEventProcessor_trackeff_hists2_
#define _DEventProcessor_trackeff_hists2_

#include <pthread.h>
#include <map>
#include <iostream>
#include <cmath>

using std::map;

#include <TThread.h>
#include <TROOT.h>
#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <PID/DKinematicData.h>
#include <PID/DParticleID.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackCandidate.h>
#include <TRACKING/DMCThrown.h>
#include <TRACKING/DMCTrajectoryPoint.h>

#include <FDC/DFDCGeometry.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <FDC/DFDCHit.h>

#include <DVector2.h>
#include <particleType.h>

#include "track2.h"
#include "DTrackingResolution.h"
#include "DTrackingResolutionGEANT.h"

class DReferenceTrajectory;
class DCoordinateSystem;

class DEventProcessor_trackeff_hists2:public JEventProcessor{

	public:
		DEventProcessor_trackeff_hists2();
		~DEventProcessor_trackeff_hists2();

		TTree *trkeff;
		track2 trk;
		track2 *trk_ptr;

		bool Search_ChargedTrackHypotheses(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber, const DMCThrown *mcthrown);
		bool Search_WireBasedTracks(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber, const DMCThrown *mcthrown);
		bool Search_TrackCandidates(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber, const DMCThrown *mcthrown);


	private:
		bool use_rt_thrown;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		bool isReconstructable(const DMCThrown *mcthrown, vector<const DMCTrajectoryPoint*> &mctrajpoints);

		DTrackingResolution *trkres;
		pthread_mutex_t mutex;
		DReferenceTrajectory *rt_thrown;
		
		double CDCZmin, CDCZmax;
		
		int DEBUG;
		
		void FindLR(vector<const DCoordinateSystem*> &wires, const DReferenceTrajectory *crt, vector<int> &LRhits);
		void FindLR(vector<const DCoordinateSystem*> &wires, vector<const DMCTrajectoryPoint*> &trajpoints, vector<int> &LRhits);
};

#endif // _DEventProcessor_trackeff_hists2_


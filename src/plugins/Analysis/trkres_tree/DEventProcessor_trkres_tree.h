// $Id$
//
//    File: DEventProcessor_trackres_tree.h
// Created: Tue Apr  7 14:54:33 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DEventProcessor_trkres_tree_
#define _DEventProcessor_trkres_tree_

#include <vector>
using namespace std;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <PID/DKinematicData.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DMCTrackHit.h>
#include <TRACKING/DMCThrown.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCHit.h>

#include "trackres.h"

class DMCTrajectoryPoint;
class DCoordinateSystem;

class DEventProcessor_trkres_tree:public JEventProcessor{
	public:
		DEventProcessor_trkres_tree();
		~DEventProcessor_trkres_tree();

		trackres *trkres_ptr, trkres;
		TTree *ttrkres;
		const DMagneticFieldMap *bfield;

		pthread_mutex_t mutex;
		
		double SIGMA_CDC;
		double SIGMA_FDC_ANODE;
		double SIGMA_FDC_CATHODE;
		
		class meas_t{
			public:
				double s;
				double err;
				double errc;
				double radlen;
				double B;
				const DMCTrajectoryPoint* traj;
		};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		const DMCTrajectoryPoint* FindTrajectoryPoint(const DCoordinateSystem *wire, double &radlen, double &s, vector<const DMCTrajectoryPoint*> trajpoints);
		void GetPtRes(vector<meas_t> &meas, double &deltak, double &pt_res);
		void GetThetaRes(vector<meas_t> &meas, double &theta_res);

};

#endif // _DEventProcessor_trackres_tree_


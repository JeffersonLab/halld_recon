// $Id$
//
//    File: DCustomAction_p2pi_unusedHists.h
// Created: Thu Jan 22 08:06:18 EST 2015
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _DCustomAction_p2pi_unusedHists_
#define _DCustomAction_p2pi_unusedHists_

#include <map>
#include <string>
#include <iostream>

#include <JANA/JEvent.h>
#include "JANA/JApplication.h"

#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"

#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry.h>
#include <BCAL/DBCALCluster.h>
#include <BCAL/DBCALPoint.h>

using namespace std;

class DCustomAction_p2pi_unusedHists : public DAnalysisAction
{
	public:

                DCustomAction_p2pi_unusedHists(const DReaction* locReaction, bool locUseKinFitResultsFlag, string locActionUniqueString = "") : 
	        DAnalysisAction(locReaction, "Custom_p2pi_unusedHists", locUseKinFitResultsFlag, locActionUniqueString){}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);

	private:

		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);
		void FillTrack(const std::shared_ptr<const JEvent>& locEvent, const DChargedTrack* locChargedTrack, bool locMatch, const DMCThrown* locMCThrown);
		void FillShower(const DNeutralShower* locNeutralShower, bool locMatch, double locBeamPhotonTime, double locFlightTime);

		// Optional: Useful utility functions.
		// const DAnalysisUtilities* dAnalysisUtilities;

		// need PID algos for SC matching
		const DParticleID* dParticleID;
		const DFCALGeometry* dFCALGeometry;

		// maps of histograms by track charge and match flag
		map<bool, map<int, TH2I*> > dHistMap_TrackNhits_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackNhitsCDC_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackNhitsFDC_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackNhitsFDCVsCDC_Theta13_16;
		map<bool, map<int, TH2I*> > dHistMap_TrackNhitsFDCVsCDC_Theta16_20;
		map<bool, map<int, TH2I*> > dHistMap_TrackChiSq_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackFOM_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackP_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackPOCAXY;
		map<bool, map<int, TH1I*> > dHistMap_TrackPOCAZ;
		map<bool, map<int, TH2I*> > dHistMap_TrackCDCHitRadius_Nhits;
		map<bool, map<int, TH2I*> > dHistMap_LowNDFTrackCDCHitRadius_PT;
		map<bool, map<int, TH2I*> > dHistMap_HighNDFTrackCDCHitRadius_PT;
		map<bool, map<int, TH2I*> > dHistMap_LowNDFTrackP_VertZ;
		map<bool, map<int, TH2I*> > dHistMap_LowNDFTrackPT_Phi;
		map<bool, map<int, TH2I*> > dHistMap_TrackMCHitFraction_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackMCMomRes_Theta;
		map<bool, map<int, TH2I*> > dHistMap_TrackMCThetaRes_Theta;

		// maps of histograms by detector system and match flag
		TH2I *dNShowerBCAL_FCAL;
		TH1I *h1_deltaX, *h1_deltaY, *h1_Efcal, *h1_tfcal;
		TH1I *h1_N9, *h1_E9, *h1_t9, *h1_t9sigma;
		TH2I *h2_YvsX9, *h2_dEdx9_vsp, *h2_E9vsp, *h2_dEdxvsE9, *h2_E9_vsTheta, *h2_E9_vsPhi;
		TH1I *h1_N1, *h1_E1, *h1_t1, *h1_t1sigma;
		TH2I *h2_YvsX1, *h2_dEdx1_vsp, *h2_E1vsp, *h2_dEdxvsE1, *h2_E1_vsTheta, *h2_E1_vsPhi, *h2_E1vsRlocal, *h2_YvsXcheck;
		TH2I *dHistMap_FCALShowerDeltaR_P, *dHistMap_FCALShowerDeltaR_Theta, *dHistMap_FCALShowerDeltaD_P, *dHistMap_FCALShowerDeltaD_Theta;
		TH2I *dHistMap_FCALShowerDeltaD_DeltaT;
		TH2I *dHistMap_BCALShowerDeltaPhi_DeltaZ, *dHistMap_BCALShowerDeltaPhi_P, *dHistMap_BCALShowerDeltaPhi_Theta;
		TH2I *dHistMap_BCALShowerDeltaPhi_DeltaT, *dHistMap_BCALShowerDeltaZ_DeltaT;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerEnergy_Theta;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerPhi_Theta;
		map<bool, map<DetectorSystem_t, TH1I*> > dHistMap_ShowerNclusters;
		map<bool, map<DetectorSystem_t, TH1I*> > dHistMap_ShowerNhits;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerMaxEnergy_Nhits;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerDeltaT_Nhits;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerDeltaT_E;
		map<bool, map<DetectorSystem_t, TH2I*> > dHistMap_ShowerE_Theta;
		map<bool, TH2I*> dHistMap_Layer1Energy_Theta;
		map<bool, TH2I*> dHistMap_Layer2Energy_Theta;
		map<bool, TH2I*> dHistMap_Layer3Energy_Theta;
		map<bool, TH2I*> dHistMap_Layer4Energy_Theta;
		map<bool, TH1I*> dHistMap_BCALShowerNcell;

};

#endif // _DCustomAction_p2pi_unusedHists_


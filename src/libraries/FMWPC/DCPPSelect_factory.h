// $Id$
//
//    File: DCPPSelect_factory.h
// Created: Sun May 17 01:31:58 PM EDT 2026
// Creator: alfab (on Linux ifarm2402.jlab.org 5.14.0-611.49.2.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2


#ifndef _DCPPSelect_factory_
#define _DCPPSelect_factory_

#include <JANA/JFactoryT.h>
#include "DCPPSelect.h"

#include <JANA/JObject.h>
#include <JANA/JEvent.h>

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>

#include <vector>

#include <ANALYSIS/DKinFitUtils_GlueX.h>
#include <ANALYSIS/DAnalysisUtilities.h>

#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackingData.h>
#include <TRACKING/DReferenceTrajectory.h>

#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DDetectorMatches.h>

#include <FMWPC/DFMWPCMatchedTrack.h>
#include <FMWPC/DFMWPCHit.h>

#include <TRIGGER/DL1Trigger.h>

#include <TOF/DTOFPoint.h>

#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALShower.h>



enum class MWPCKey {
    fmwpc1_proj_plus  = 0,
    fmwpc1_proj_minus = 1,
    fmwpc2_proj_plus  = 2,
    fmwpc2_proj_minus = 3,
    fmwpc3_proj_plus  = 4,
    fmwpc3_proj_minus = 5,
    fmwpc4_proj_plus  = 6,
    fmwpc4_proj_minus = 7,
    fmwpc5_proj_plus  = 8,
    fmwpc5_proj_minus = 9,
    fmwpc6_proj_plus  = 10,
    fmwpc6_proj_minus = 11
};



class DCPPSelect_factory:public JFactoryT<DCPPSelect>{
	public:
		DCPPSelect_factory(){
			SetTag("");
		}
		~DCPPSelect_factory(){}


	private:
		void Init() override;                       ///< Called once at program start.
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
		void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
		void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
		void Finish() override;                     ///< Called after last event of last event source has been processed.

        double calculateTrackEnergy(DVector3 ptrack_mom, double pmass);
        bool MatchToTOF_CPP_GEOM(const vector<const DTOFPoint*>& tof_points, DVector3 tof_proj_pos);
        bool MatchToFCALShower_CPP(const vector<const DFCALShower*>& fcal_showers, vector<const DFCALShower*>& fcal_matched_showers, DVector3 fcal_proj_pos, DVector3 fcal_proj_mom);
        bool MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, vector<const DFCALHit*>& fcal_matched_hits, double& e9e25, double& doca, double& e1e9,DVector3 fcal_proj_pos, DVector3 fcal_proj_mom,double& sumUSh, double& sumVSh);
		double mwpc_sigma(int ic, double p);
		bool CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos);
        void RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map);
		bool ComputeMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits,std::map<MWPCKey,DVector3> mwpc_projections,double track1_energy,double track2_energy, map<MWPCKey,int>& mwpc_multis, bool& piplus_track_chamber6, bool& piminus_track_chamber6);

		const DMagneticFieldMap *bfield;
        double fcalfrontfaceZ;
        double m_FCALdX, m_FCALdY, m_FCALfront, m_TOFdX, m_TOFdY, m_TOFfront;

};

#endif // _DCPPSelect_factory_


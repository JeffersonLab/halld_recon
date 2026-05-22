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

static const int cwire_min[4] = {20, 39, 84, 103};
static const int cwire_max[4] = {43, 64, 106, 126};

static const double cwire_min_X[4] = {-52.324,-33.02,12.7,32.004};
static const double cwire_max_X[4] = {-28.956,-7.62,35.052,55.372};

static const double mwpcz[6] = {811.62, 827.79, 849.04,890.65,921.84,941.12};

const double FMWPC_WIRE_SPACING=1.016;

static const double mu_mass = 0.10565;


static const double fmwpc_z[6]  = {811.62, 827.79, 849.04, 890.65, 931.84, 941.12};
static const double fmwpc_sz[6] = {  2.26,   2.60,   2.94,   3.64,   4.93,   5.15};
static const double fmwpc_al[6] = {  0.25,  -0.22,   0.23,  -0.27,   0.17,  -0.06};

static const double mpi0    = 0.1349766;
static const double mpic    = 0.13957;
static const double meta    = 0.547862;
static const double metap   = 0.95778;
static const double clight  = 29.9792458;


static const double my_TOF_dx  = -0.28;
static const double my_TOF_dy  =  0.21;

static const double fmwpc_spar[6][4] = {	\
      3.8515, 1.6761, 0.98888, 0.041894, \
      3.9221, 1.7107, 1.18600, 0.057278, \
      4.0039, 1.6271, 1.53250, 0.088580, \
      13.070, 6.6978, 2.04570, 0.140260, \
      2.2558, .14294, -1., 0., \
      2.2548, .42019, 1.3483, 0.0022198 };

enum class MWPCProjKey {
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

		double mwpc_sigma(int ic, double p);
		bool CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos);
        void RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map);
		void ComputeMWPCWireResiduals(const DTrackTimeBased* locTrackTimeBased,const vector<const DFMWPCHit*> locFMWPCHits,std::map<MWPCProjKey,DVector3> mwpc_projections);

		const DMagneticFieldMap *bfield;
        double fcalfrontfaceZ;
        double m_FCALdX, m_FCALdY, m_FCALfront, m_TOFdX, m_TOFdY, m_TOFfront;

};

#endif // _DCPPSelect_factory_


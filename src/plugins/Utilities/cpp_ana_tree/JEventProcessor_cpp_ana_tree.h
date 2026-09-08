// $Id$
//
//    File: JEventProcessor_cpp_ana_tree.h
// Created: Tue Jul 28 11:38:57 AM EDT 2026
// Creator: alfab (on Linux ifarm2401.jlab.org 5.14.0-611.55.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_cpp_ana_tree_
#define _JEventProcessor_cpp_ana_tree_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>

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
#include <PID/DNeutralParticle.h>
#include <PID/DNeutralShower.h>
#include <PID/DDetectorMatches.h>

#include <FMWPC/DFMWPCMatchedTrack.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DFMWPCDigiHit.h>
#include <FMWPC/DCTOFHit.h>
#include <FMWPC/DCTOFPoint.h>
#include <FMWPC/DEPIClassifierMLPPlus.h>
#include <FMWPC/DEPIClassifierMLPMinus.h>

#include <FMWPC/DCPPSelect.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRIGGER/DTrigger.h>

#include <HDDM/DEventHitStatistics.h>

#include <TOF/DTOFPoint.h>

#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALShower.h>

#include <TRACKING/DMCThrown.h>

#include <TRIGGER/DL1Trigger.h>

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


class JEventProcessor_cpp_ana_tree:public JEventProcessor{
    public:
        JEventProcessor_cpp_ana_tree();
        ~JEventProcessor_cpp_ana_tree();
        const char* className(void){return "JEventProcessor_cpp_ana_tree";}

        float nCT, nFS, nFh, nMh, nFMT,FEc8, FE38, FE58, Fcw18,Fdcw18,FNc18,Fcw28,Fdcw28,FNc28,Fcw38,Fdcw38,FNc38,Fcw48,Fdcw48,FNc48,Fcw58,
            Fdcw58,FNc58,Fcw68,Fdcw68,FNc68,FEc9,FE39,FE59,Fcw19,Fdcw19,FNc19,Fcw29,Fdcw29,FNc29,Fcw39,Fdcw39,FNc39,Fcw49,Fdcw49,FNc49,Fcw59,
            Fdcw59, FNc59,Fcw69,Fdcw69,FNc69,TPm,TMm,fcale8,fcalep8,fcaldoca8,fcale9e258,fcalblocksn8,fcale9,fcalep9,fcaldoca9,fcale9e259,
            fcalblocksn9,sumu8,sumu9,sumv8,sumv9,fmwpc1n,fmwpc2n,fmwpc3n,fmwpc4n,fmwpc5n,fmwpc6n,cn8,cn9,w_2pi,p1x,p1y,p1z,p1e,p2x,p2y,p2z,p2e,p1x_pos,p1y_pos,p1z_pos,p2x_pos,p2y_pos,p2z_pos,epimlp_plus,epimlp_minus,
            bE,bT,t1t,t2t,t1e,t2e,elas,wght,tagid,mm_chi,mp_8_kf,mp_9_kf,fcalepkf8,fcalepkf9,mlp_kf_8,mlp_kf_9,w_2mu_kf,p1x_kf,p1y_kf,p1z_kf,p1e_kf,p2x_kf,
            p2y_kf,p2z_kf,p2e_kf,kf_ndf,t1_ndf,t2_ndf,eventno,pfinalChamber,nfinalChamber,pfinalChamber1to5,nfinalChamber1to5,
            pdiff1,pdiff2,pdiff3,pdiff4,pdiff5,pdiff6,ndiff1,ndiff2,ndiff3,ndiff4,ndiff5,ndiff6,fcale1e98,fcale1e99,
            fmwpc1m8,fmwpc2m8,fmwpc3m8,fmwpc4m8,fmwpc5m8,fmwpc6m8,fmwpc1m9,fmwpc2m9,fmwpc3m9,fmwpc4m9,fmwpc5m9,fmwpc6m9,
            tfmwpc1m,tfmwpc2m,tfmwpc3m,tfmwpc4m,tfmwpc5m,tfmwpc6m,nFMh8,nFMh9,nFSr,nFHr,tfinalChamber,tfinalChamber1to5,
            thrownpx1,thrownpy1,thrownpz1,thrownpe1,thrownpx2,thrownpy2,thrownpz2,thrownpe2,nFS8r,nFS9r,nFH8r,nFH9r,
            fcaltime8,fcaltime9,vtxt8,vtxt9,vtxpos8,vtxpos9,trackatfcalz8,trackatfcalz9,trackatfcalt8,trackatfcalt9,
            pfinalChamber_exclusive,nfinalChamber_exclusive,pfinalChamber1to5_exclusive,nfinalChamber1to5_exclusive,
            tfinalChamber_exclusive,tfinalChamber1to5_exclusive,ctofbar1,ctofbar2,ctofbar3,ctofbar4,c1extrap_plus,c2extrap_plus,c3extrap_plus,c4extrap_plus,c5extrap_plus,c6extrap_plus,c1extrap_minus,c2extrap_minus,c3extrap_minus,c4extrap_minus,c5extrap_minus,c6extrap_minus,c6fid_plus,c6fid_minus, bshowerexist,bhitexist,plusenergy_bcal,minusenergy_bcal;

        TFile *ofile;
        TTree *ptree;
        TH2D *h2_vertex_x_y,*h_xyvtx;
        TH1D *h1_vertex_z,*h1_t0_rf, *h_u,*h_zvtx;
        TH1D *h1_muplus_chisq, *h1_muminus_chisq;
        TH1D *h1_muplus_energy, *h1_muminus_energy, *h1_2c_energy;
        TH1D *h1_muplus_energy_nocut,*h1_muminus_energy_nocut,*h1_2c_energy_nocut;
        TH1D *h1_mlp_score_plus,*h1_mlp_score_minus;
        TH1I *h1_bar1_track_multiplicity,*h1_bar2_track_multiplicity,*h1_bar3_track_multiplicity,*h1_bar4_track_multiplicity;
        TH1I *h1_bar1_tm_plus,*h1_bar2_tm_plus,*h1_bar3_tm_plus,*h1_bar4_tm_plus;
        TH1I *h1_bar1_tm_minus,*h1_bar2_tm_minus,*h1_bar3_tm_minus,*h1_bar4_tm_minus;
        TH1I *h1_energy_combos,*h1_fcal_event_type;
        TH1D *h1_tof_dx, *h1_tof_dy;

        TBranch *nChargedTracks, *nFCALShowers, *nFCALhits, *nMWPChits, *nFMWPCMatchedTracks, *FCAL_E_center_8, *FCAL_E_3x3_8,
            *FCAL_E_5x5_8, *FMWPC_closest_wire1_8, *FMWPC_dist_closest_wire1_8, *FMWPC_Nhits_cluster1_8, *FMWPC_closest_wire2_8,
            *FMWPC_dist_closest_wire2_8, *FMWPC_Nhits_cluster2_8, *FMWPC_closest_wire3_8, *FMWPC_dist_closest_wire3_8,
            *FMWPC_Nhits_cluster3_8, *FMWPC_closest_wire4_8, *FMWPC_dist_closest_wire4_8, *FMWPC_Nhits_cluster4_8, *FMWPC_closest_wire5_8,
            *FMWPC_dist_closest_wire5_8, *FMWPC_Nhits_cluster5_8, *FMWPC_closest_wire6_8, *FMWPC_dist_closest_wire6_8,
            *FMWPC_Nhits_cluster6_8, *FCAL_E_center_9, *FCAL_E_3x3_9, *FCAL_E_5x5_9, *FMWPC_closest_wire1_9, *FMWPC_dist_closest_wire1_9,
            *FMWPC_Nhits_cluster1_9, *FMWPC_closest_wire2_9, *FMWPC_dist_closest_wire2_9, *FMWPC_Nhits_cluster2_9, *FMWPC_closest_wire3_9,
            *FMWPC_dist_closest_wire3_9, *FMWPC_Nhits_cluster3_9, *FMWPC_closest_wire4_9, *FMWPC_dist_closest_wire4_9,
            *FMWPC_Nhits_cluster4_9, *FMWPC_closest_wire5_9, *FMWPC_dist_closest_wire5_9,*FMWPC_Nhits_cluster5_9, *FMWPC_closest_wire6_9,
            *FMWPC_dist_closest_wire6_9, *FMWPC_Nhits_cluster6_9,*TRACK_PLUS_MOM,*TRACK_MINUS_MOM,*FCAL_e_8,*FCAL_eoverp_8,*FCAL_doca_8,
            *FCAL_e9e25_8,*FCAL_blocks_n_8, *FCAL_e_9,*FCAL_eoverp_9,*FCAL_doca_9,*FCAL_e9e25_9,*FCAL_blocks_n_9,*FCALSumU8,*FCALSumU9,*FCALSumV8,*FCALSumV9,
            *FMWPC_chamber1,*FMWPC_chamber2,*FMWPC_chamber3,*FMWPC_chamber4,*FMWPC_chamber5,*FMWPC_chamber6,*chisq_8,*chisq_9,*InvariantMass,*Pion1_px,*Pion1_py,*Pion1_pz,
            *Pion1_pe,*Pion2_px,*Pion2_py,*Pion2_pz,*Pion2_pe,*Pion1_x,*Pion1_y,*Pion1_z,*Pion2_x,*Pion2_y,*Pion2_z,*EPI_MLP_Response_plus,*EPI_MLP_Response_minus,*Beam_Energy,*Beam_Time,*Track1_Time,
            *Track2_Time,*Track1_Energy,*Track2_Energy,*Elasticity,*Weight,*Tagger_ID,*KinFit_Chisq,*Track_Mom_8_KinFit,*Track_Mom_9_KinFit,*FCAL_eoverp_kf_8,*FCAL_eoverp_kf_9,
            *EPI_MLP_Response_KF8,*EPI_MLP_Response_KF9,*InvariantMass_KF,*Pion1_px_KF,*Pion1_py_KF,*Pion1_pz_KF,*Pion1_pe_KF,*Pion2_px_KF,*Pion2_py_KF,
            *Pion2_pz_KF,*Pion2_pe_KF,*KinFit_NDF,*Track1_NDF,*Track2_NDF,*Event_No,*FinalChamber8,*FinalChamber9,*FinalChamber81to5,*FinalChamber91to5,*Chamber1_diff_8,*Chamber2_diff_8,*Chamber3_diff_8,
            *Chamber4_diff_8,*Chamber5_diff_8,*Chamber6_diff_8,*Chamber1_diff_9,*Chamber2_diff_9,*Chamber3_diff_9,*Chamber4_diff_9,*Chamber5_diff_9,*Chamber6_diff_9,
            *FCAL_E1E9_8,*FCAL_E1E9_9,*Chamber1MatchedHits8,*Chamber2MatchedHits8,*Chamber3MatchedHits8,*Chamber4MatchedHits8,*Chamber5MatchedHits8,*Chamber6MatchedHits8,
            *Chamber1MatchedHits9,*Chamber2MatchedHits9,*Chamber3MatchedHits9,*Chamber4MatchedHits9,*Chamber5MatchedHits9,*Chamber6MatchedHits9,
            *Chamber1TotalMatchedHits,*Chamber2TotalMatchedHits,*Chamber3TotalMatchedHits,*Chamber4TotalMatchedHits,*Chamber5TotalMatchedHits,*Chamber6TotalMatchedHits,
            *NoChamber_matches_8,*NoChamber_matches_9,*NoFCALShowersRefined,*NoFCALHitsRefined,*TriggerFinalChamber,*TriggerFinalChamber1to5,
            *Thrown_P4X1,*Thrown_P4Y1,*Thrown_P4Z1,*Thrown_P4E1,*Thrown_P4X2,*Thrown_P4Y2,*Thrown_P4Z2,*Thrown_P4E2,*FCAL_SHOWER_COUNT8,*FCAL_SHOWER_COUNT9,
            *FCAL_HIT_COUNT8,*FCAL_HIT_COUNT9,*FCALTime8,*FCALTime9,*VertexZpos8,*VertexZpos9,*TrackZFCAL8,*TrackZFCAL9,*VertexT8,*VertexT9,*TrackTFCAL8,*TrackTFCAL9,
            *TriggerFinalChamber1to5_exclusive,*TriggerFinalChamber_exclusive,*FinalChamber8_exclusive,*FinalChamber9_exclusive,*FinalChamber81to5_exclusive,
            *FinalChamber91to5_exclusive,*CTOF_Bar1,*CTOF_Bar2,*CTOF_Bar3,*CTOF_Bar4,*Chamber1_Extrap_Exists_Plus,*Chamber2_Extrap_Exists_Plus,*Chamber3_Extrap_Exists_Plus,
            *Chamber4_Extrap_Exists_Plus,*Chamber5_Extrap_Exists_Plus,*Chamber6_Extrap_Exists_Plus,*Chamber1_Extrap_Exists_Minus,*Chamber2_Extrap_Exists_Minus,*Chamber3_Extrap_Exists_Minus,*Chamber4_Extrap_Exists_Minus,*Chamber5_Extrap_Exists_Minus,*Chamber6_Extrap_Exists_Minus,*Chamber6_Fiducial_Plus,*Chamber6_Fiducial_Minus,*BShowersExist,*BHitsExist,*BCALPlusEnergy,*BCALMinusEnergy;

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

        double calculateTrackEnergy(DVector3 ptrack_mom, double pmass);
        double getEPIClassifierMinus(double EoverP_minus, double FCAL_DOCA_minus, double FCAL_E9E25_minus);
        double getEPIClassifierPlus(double EoverP_plus, double FCAL_DOCA_plus, double FCAL_E9E25_plus);
        

        double SPLIT_CUT,FCAL_THRESHOLD,BCAL_THRESHOLD,GAMMA_DT_CUT;
        const DMagneticFieldMap *bfield;
        double fcalfrontfaceZ;
        double m_FCALdX, m_FCALdY, m_FCALfront, m_TOFdX, m_TOFdY, m_TOFfront;
        int tof_count, fcal_count;

        ReadMLPMinus* dEPIClassifierMinus;
        ReadMLPPlus* dEPIClassifierPlus;
        const char* inputVarsMinus[3] = { "EoverP_minus", "FCAL_DOCA_em", "FCAL_E9E25_em" };
        const char* inputVarsPlus[3] = { "EoverP_plus", "FCAL_DOCA_ep", "FCAL_E9E25_ep" };

        int counter;
};

#endif // _JEventProcessor_cpp_ana_tree_


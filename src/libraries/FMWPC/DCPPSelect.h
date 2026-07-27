// $Id$
//
//    File: DCPPSelect.h
// Created: Sun May 17 01:31:58 PM EDT 2026
// Creator: alfab (on Linux ifarm2402.jlab.org 5.14.0-611.49.2.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2


#ifndef _DCPPSelect_h_
#define _DCPPSelect_h_

#include <JANA/JObject.h>
#include <DVector3.h>

#include <particleType.h>
#include <TRACKING/DTrackTimeBased.h>

#include <TOF/DTOFPoint.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALHit.h>
#include <BCAL/DBCALShower.h>
#include <BCAL/DBCALHit.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DCTOFPoint.h>

#include <DVector3.h>

struct ProjectionResults {
    bool projection_success = false;
    DVector3 tof_projection;
    DVector3 fcal_projection;
    std::array<bool,6> mwpc_flags;
    std::array<DVector3,6> mwpc_projections;
    DVector3 ctof_projection;
};

struct FCALSingleHitVals {
    bool fcalSinglesFound = false; 
    double fcalSingleE1E9 = 0.0;
    double fcalSingleE9E25 = 999.0;
    double fcalSingleDOCA = 0.0;
    double fcalSingleSumU = 0.0;
    double fcalSingleSumV = 0.0;
};

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


class DCPPSelect : public JObject {
    public:
        JOBJECT_PUBLIC(DCPPSelect)
    
        //HELPER Functions
        static ProjectionResults SwimTracksToAllDetectors(const DTrackTimeBased* locTimeTrack, Particle_t particle_id,const DMagneticFieldMap *bfield,double fcalfrontfaceZ,double m_TOFfront);

        static bool MatchToTOF_CPP_GEOM(const vector<const DTOFPoint*>& tof_points, DVector3 tof_proj_pos);

        static bool MatchToFCALShower_CPP(const vector<const DFCALShower*>& fcal_showers, vector<const DFCALShower*>& fcal_matched_showers, DVector3 fcal_proj_pos);
        
        static bool MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, DVector3 fcal_proj_pos, vector<const DFCALHit*>& fcal_matched_hits, FCALSingleHitVals& fcal_single_hit_vals);

        static bool MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, vector<const DFCALHit*>& fcal_matched_hits, double& e9e25, double& doca, double& e1e9,DVector3 fcal_proj_pos, double& sumUSh, double& sumVSh);


        static bool ComputeMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits,std::map<MWPCKey,DVector3> mwpc_projections,double track1_energy,double track2_energy, map<MWPCKey,int>& mwpc_multis, bool& piplus_track_chamber6, bool& piminus_track_chamber6);

        static bool ComputeSingleTrackMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits, std::map<MWPCKey,DVector3> mwpc_projections,double track_energy,bool& track_in_chamber6, double track_charge,std::map<int,double>& fmwpc1_wire_diff, std::map<int,double>& fmwpc2_wire_diff, std::map<int,double>& fmwpc3_wire_diff, std::map<int,double>& fmwpc4_wire_diff,std::map<int,double>& fmwpc5_wire_diff, std::map<int,double>& fmwpc6_wire_diff);

        static double mwpc_sigma(int ic, double p);
        static bool CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos);

        static void RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map);

        static bool CheckTrackToCTOF_CPP(const vector<const DCTOFPoint*>& ctof_hit, DVector3 ctof_proj_pos, vector<int>& bCTGH);

        static bool MatchToPaddle(double x_track_pos, const vector<int>& allowedPaddles,int hitPaddleID);

        static bool MatchToCTOFHit_CPP(const vector<const DCTOFPoint*>& ctof_Hits, vector<const DCTOFPoint*>& ctof_matched_hits, DVector3 ctof_proj_pos,const vector<int>& allowedPaddles);
        //Beam Info
        double beam_weight;
        double beam_energy;
        
        //Track Info
        DVector3 piplus3mom;
        DVector3 piminus3mom;

        //KinFit 
        DVector3 piplus3mom_kf;
        DVector3 piminus3mom_kf;
        double kinfit_chisq;
        double kinfit_ndf;

        bool IS_TrackDOCAGood;

        //BCAL 
        bool PlusTrack_EnergyInBCAL;
        bool MinusTrack_EnergyInBCAL;

        bool BCALShowersExist;
        bool BCALHitsExist;

        //TOF 
        bool IS_PlusTrackInTOF;
        bool IS_MinusTrackInTOF;

        //FCAL Features
        double fcal_energy_piplus;      //FCAL Energy pi+ track
        double fcal_energy_piminus;     //FCAL Energy pi- track
        double fcal_eoverp_piplus;      //FCAL Energy/pi+ track momentum
        double fcal_eoverp_piminus;     //FCAL Energy/pi- track momentum
        double fcal_doca_piplus;        //FCAL DOCA pi+ track
        double fcal_doca_piminus;       //FCAL DOCA pi- track
        double fcal_e1e9_piplus;        //FCAL E1/E9 Shower Ratio pi+ track
        double fcal_e1e9_piminus;       //FCAL E1/E9 Shower Ratio pi- track
        double fcal_e9e25_piplus;       //FCAL E9/E25 Shower Ratio pi+ track
        double fcal_e9e25_piminus;      //FCAL E9/E25 Shower Ratio pi- track
        double fcal_nblocks_piplus;     //FCAL Number Blocks pi+ track
        double fcal_nblocks_piminus;    //FCAL Number Blocks pi- track
        int fcal_showers_count_piplus;  //Number of FCAL Showers pi+ track
        int fcal_showers_count_piminus; //Number of FCAL Showers pi- track
        int fcal_hit_count_piplus;      //Number of FCAL Showers pi+ track
        int fcal_hit_count_piminus;     //Number of FCAL Showers pi- track

        int fcal_showers_only;
        int fcal_hits_only;
        int fcal_shower_plus_minus_hit;
        int fcal_shower_minus_plus_hit;

        //FMWPC Features
        bool IS_Chamber1ExtrapReal_plus;
        bool IS_Chamber2ExtrapReal_plus;
        bool IS_Chamber3ExtrapReal_plus;
        bool IS_Chamber4ExtrapReal_plus;
        bool IS_Chamber5ExtrapReal_plus;
        bool IS_Chamber6ExtrapReal_plus;

        bool IS_Chamber1ExtrapReal_minus;
        bool IS_Chamber2ExtrapReal_minus;
        bool IS_Chamber3ExtrapReal_minus;
        bool IS_Chamber4ExtrapReal_minus;
        bool IS_Chamber5ExtrapReal_minus;
        bool IS_Chamber6ExtrapReal_minus;

        bool IS_PlusTrackINChamber6; //Is plus track in fiducial region of chamber 6
        bool IS_MinusTrackINChamber6; //Is minus track in fiducial region of chamber 6
        //FMWPC Multiplicities
        int fmwpc1n_piplus;     //MWPC 1 Multiplicity 
        int fmwpc1n_piminus;    //MWPC 1 Multiplicity 
        int fmwpc2n_piplus;     //MWPC 2 Multiplicity  
        int fmwpc2n_piminus;    //MWPC 2 Multiplicity 
        int fmwpc3n_piplus;     //MWPC 3 Multiplicity  
        int fmwpc3n_piminus;    //MWPC 3 Multiplicity 
        int fmwpc4n_piplus;     //MWPC 4 Multiplicity  
        int fmwpc4n_piminus;    //MWPC 4 Multiplicity  
        int fmwpc5n_piplus;     //MWPC 5 Multiplicity 
        int fmwpc5n_piminus;    //MWPC 5 Multiplicity 
        int fmwpc6n_piplus;     //MWPC 6 Multiplicity 
        int fmwpc6n_piminus;    //MWPC 6 Multiplicity 

        //CTOF
        int ctof_bar1;
        int ctof_bar2;
        int ctof_bar3;
        int ctof_bar4;


    void Summarize(JObjectSummary& summary) const override {
        summary.add(beam_weight, "Acc. Weight","%f");
        summary.add(beam_energy, "Beam E","%f");

        summary.add(IS_MinusTrackInTOF,"TOF+","%d");
        summary.add(IS_MinusTrackInTOF,"TOF-","%d");

        summary.add(fcal_eoverp_piplus,"FCAL E/P pi+","%f");
        summary.add(fcal_eoverp_piminus,"FCAL E/P pi-","%f");
        summary.add(fcal_doca_piplus,"FCAL DOCA pi+","%f");
        summary.add(fcal_doca_piminus,"FCAL DOCA pi-","%f");
        summary.add(fcal_e1e9_piplus,"FCAL E1/E9 pi+","%f");
        summary.add(fcal_e1e9_piminus,"FCAL E1/E9 pi-","%f");
        summary.add(fcal_nblocks_piplus,"FCAL # of Blocks pi+","%f");
        summary.add(fcal_nblocks_piminus,"FCAL # of Blocks pi-","%f");

        summary.add(fmwpc1n_piplus,"FMWPC 1 Multiplicity pi+","%d");
        summary.add(fmwpc1n_piminus,"FMWPC 1 Multiplicity pi-","%d");
        summary.add(fmwpc2n_piplus,"FMWPC 2 Multiplicity pi+","%d");
        summary.add(fmwpc2n_piminus,"FMWPC 2 Multiplicity pi-","%d");
        summary.add(fmwpc3n_piplus,"FMWPC 3 Multiplicity pi+","%d");
        summary.add(fmwpc3n_piminus,"FMWPC 3 Multiplicity pi-","%d");
        summary.add(fmwpc4n_piplus,"FMWPC 4 Multiplicity pi+","%d");
        summary.add(fmwpc4n_piminus,"FMWPC 4 Multiplicity pi-","%d");
        summary.add(fmwpc5n_piplus,"FMWPC 5 Multiplicity pi+","%d");
        summary.add(fmwpc5n_piminus,"FMWPC 5 Multiplicity pi-","%d");
        summary.add(fmwpc6n_piplus,"FMWPC 6 Multiplicity pi+","%d");
        summary.add(fmwpc6n_piminus,"FMWPC 6 Multiplicity pi-","%d");
    }
};


#endif // _DCPPSelect_h_


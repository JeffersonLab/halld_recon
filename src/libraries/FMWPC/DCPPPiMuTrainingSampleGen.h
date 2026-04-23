// $Id$
//
//    File: DCPPPiMuTrainingSampleGen.h
// Created: Wed Apr  1 03:58:36 PM EDT 2026
// Creator: alfab (on Linux ifarm2402.jlab.org 5.14.0-611.38.1.el9_7.x86_64 x86_64)
//

#ifndef _DCPPPiMuTrainingSampleGen_h_
#define _DCPPPiMuTrainingSampleGen_h_

#include <JANA/JObject.h>
#include <TLorentzVector.h>

class DCPPPiMuTrainingSampleGen : public JObject {
    public:
        JOBJECT_PUBLIC(DCPPPiMuTrainingSampleGen);

        //Beam Info
        double beam_energy;   //Beam Energy
        double beam_time;     //Beam Time
        double weight;        //Accidental Weighting  
        //Relative Track Info
        double chisq_piplus;        //Chi^2 pi+ Track Fit
        double chisq_piminus;       //Chi^2 pi- Track Fit
        double nDof_piplus;         //Number of Degrees of Freedom pi+ Track Fit
        double nDof_piminus;        //Number of Degrees of Freedom pi- Track Fit
        TLorentzVector piplus_p4;   //pi+ Momentum 4-vector
        TLorentzVector piminus_p4;   //pi- Momentum 4-vector
        int TOF_match_to_plus_track; //Check if Plus Track hit TOF
        int TOF_match_to_minus_track; //Check if Minus Track hit TOF
        //KinFit Info
        double chisq_kinfit;              //Kinematic Fit Chi^2
        double ndof_kinfit;               //Kinematic Fit DoF
        TLorentzVector piplus_kinfit_p4;  //Kinematic Fit pi+ Momentum 4-vector
        TLorentzVector piminus_kinfit_p4; //Kinematic Fit pi- Momentum 4-vector

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

        //MWPC Features
        int fmwpc1n_piplus;     //MWPC 1 Multiplicity pi+ 
        int fmwpc1n_piminus;    //MWPC 1 Multiplicity pi-
        int fmwpc2n_piplus;     //MWPC 2 Multiplicity pi+ 
        int fmwpc2n_piminus;    //MWPC 2 Multiplicity pi-
        int fmwpc3n_piplus;     //MWPC 3 Multiplicity pi+ 
        int fmwpc3n_piminus;    //MWPC 3 Multiplicity pi-
        int fmwpc4n_piplus;     //MWPC 4 Multiplicity pi+ 
        int fmwpc4n_piminus;    //MWPC 4 Multiplicity pi- 
        int fmwpc5n_piplus;     //MWPC 5 Multiplicity pi+
        int fmwpc5n_piminus;    //MWPC 5 Multiplicity pi-
        int fmwpc6n_piplus;     //MWPC 6 Multiplicity pi+
        int fmwpc6n_piminus;    //MWPC 6 Multiplicity pi-
        int final_chamber_hit;  //Final Chamber Hit
        int final_chamber_hit_1to5; //Final Chamber Hit 1 to 5
        int hit_in_chamber6_plus; 
        int hit_in_chamber6_minus; 

        void Summarize(JObjectSummary& summary) const override {
            summary.add(beam_energy, "Beam Energy", "%f");
            summary.add(beam_time,"Beam Time", "%f");
            summary.add(weight,"Accidental Weighting", "%f");
            summary.add(chisq_piplus, "pi+ Chi^2", "%f");
            summary.add(chisq_piminus, "pi- Chi^2","%f");
            summary.add(piplus_p4.P(), "pi+ Momentum","%f");
            summary.add(piminus_p4.P(), "pi- Momentum","%f");
            summary.add(chisq_kinfit,"Chi^2 KinFit","%f");
            summary.add(piplus_kinfit_p4.P(),"pi+ Momentum KinFit","%f");
            summary.add(piminus_kinfit_p4.P(),"pi- Momentum KinFit","%f");
            summary.add(fcal_eoverp_piplus,"FCAL E/P pi+","%f");
            summary.add(fcal_eoverp_piminus,"FCAL E/P pi-","%f");
            summary.add(fcal_doca_piplus,"FCAL DOCA pi+","%f");
            summary.add(fcal_doca_piminus,"FCAL DOCA pi-","%f");
            summary.add(fcal_e1e9_piplus,"FCAL E1/E9 pi+","%f");
            summary.add(fcal_e1e9_piminus,"FCAL E1/E9 pi-","%f");
            summary.add(fcal_nblocks_piplus,"FCAL # of Blocks pi+","%f");
            summary.add(fcal_nblocks_piminus,"FCAL # of Blocks pi-","%f");
            summary.add(fmwpc1n_piplus,"FMWPC 1 Multiplicity pi+","%f");
            summary.add(fmwpc1n_piminus,"FMWPC 1 Multiplicity pi-","%f");
        }
};


#endif // _DCPPPiMuTrainingSampleGen_h_


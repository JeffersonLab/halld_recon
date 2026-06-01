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


class DCPPSelect : public JObject {
    public:
        JOBJECT_PUBLIC(DCPPSelect)

        //Track Info
        DVector3 piplus3mom;
        DVector3 piminus3mom;

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

        //FMWPC Features
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





    void Summarize(JObjectSummary& summary) const override {
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


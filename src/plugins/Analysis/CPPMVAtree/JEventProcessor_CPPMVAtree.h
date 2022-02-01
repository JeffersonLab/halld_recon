// $Id$
//
//    File: JEventProcessor_CPPMVAtree.h
// Created: Thu Oct 28 07:44:19 EDT 2021
// Creator: davidl (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_CPPMVAtree_
#define _JEventProcessor_CPPMVAtree_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>

/*********TFile, TTree, and TBranches****/
#define nMaxTracks 10
#define nMaxFCALhits 20
#define nMaxFCALShowers 10
#define nMaxMWPChits 100
#define nMaxMWPCMatchedTracks 8

class JEventProcessor_CPPMVAtree:public jana::JEventProcessor{
	public:
		JEventProcessor_CPPMVAtree();
		~JEventProcessor_CPPMVAtree();
		const char* className(void){return "JEventProcessor_CPPMVAtree";}



        //----- The following are used to hold the values for the TTree -----
        TTree *t1;

        //General
        Int_t eventNo, nChargedTracks, nFCALShowers, nFCALhits, nMWPChits;

        //TRACKS
        Double_t p_timebased[nMaxTracks], p_timebased_x[nMaxTracks], p_timebased_y[nMaxTracks], p_timebased_z[nMaxTracks];
        Double_t mass_timebased[nMaxTracks];
        Int_t charge_timebased[nMaxTracks];
        Double_t FOM_timebased[nMaxTracks];

        Double_t trkpos_fcal_x[nMaxTracks], trkpos_fcal_y[nMaxTracks], trkpos_fcal_z[nMaxTracks], 
        proj_mom_fcal_x[nMaxTracks], proj_mom_fcal_y[nMaxTracks], proj_mom_fcal_z[nMaxTracks], proj_fcal_t[nMaxTracks];

        Double_t trkpos_fmwpc_x[nMaxTracks], trkpos_fmwpc_y[nMaxTracks], trkpos_fmwpc_z[nMaxTracks], 
        proj_mom_fmwpc_x[nMaxTracks], proj_mom_fmwpc_y[nMaxTracks], proj_mom_fmwpc_z[nMaxTracks], proj_t_fmwpc[nMaxTracks];

        //FCAL HITS
        Double_t hit_Efcal[nMaxFCALhits], hit_tfcal[nMaxFCALhits], hit_intOverPeak_fcal[nMaxFCALhits], hit_x_fcal[nMaxFCALhits], hit_y_fcal[nMaxFCALhits];
        Int_t hit_row_fcal[nMaxFCALhits], hit_col_fcal[nMaxFCALhits];

        //FCAL SHOWERS
        Double_t shower_pos_fcal_x[nMaxFCALShowers], shower_pos_fcal_y[nMaxFCALShowers], shower_time_fcal[nMaxFCALShowers], shower_nblocks_fcal[nMaxFCALShowers]; 

        //MWPC HITS
        Int_t MWPC_hit_layer[nMaxMWPChits], MWPC_hit_wire[nMaxMWPChits];
        Double_t MWPC_hit_dEwire[nMaxMWPChits], MWPC_hit_twire[nMaxMWPChits], MWPC_hit_x[nMaxMWPChits], MWPC_hit_y[nMaxMWPChits];

        //------ Tree to hold DFMWPCMatchedTrack values
        TTree *tmatched;
        
         //General
        Int_t nFMWPCMatchedTracks;

        //MATCHED TRACKS
        Int_t    FMWPC_pid[nMaxMWPCMatchedTracks];
        Double_t FCAL_E_center[nMaxMWPCMatchedTracks], FCAL_E_3x3[nMaxMWPCMatchedTracks], FCAL_E_5x5[nMaxMWPCMatchedTracks];
        Int_t    FMWPC_closest_wire[6][nMaxMWPCMatchedTracks], FMWPC_Nhits_cluster[6][nMaxMWPCMatchedTracks];
        Double_t FMWPC_dist_closest_wire[6][nMaxMWPCMatchedTracks];

        //--------  Histograms
        TH1D *tdiff_trk_fcal = nullptr;
        TH2D *tdiff_trk_fmwpc = nullptr;

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_CPPMVAtree_


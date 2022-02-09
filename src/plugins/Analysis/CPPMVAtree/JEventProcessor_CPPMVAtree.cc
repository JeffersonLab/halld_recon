// $Id$
//
//    File: JEventProcessor_CPPMVAtree.cc
// Created: Thu Oct 28 07:44:19 EDT 2021
// Creator: davidl (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_CPPMVAtree.h"
using namespace jana;
using namespace std;

#include <FDC/DFDCHit.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DFMWPCMatchedTrack.h>
#include <TRACKING/DTrackTimeBased.h>
//#include "ANALYSIS/DAnalysisUtilities.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_CPPMVAtree());
}
} // "C"


//------------------
// JEventProcessor_CPPMVAtree (Constructor)
//------------------
JEventProcessor_CPPMVAtree::JEventProcessor_CPPMVAtree()
{

}

//------------------
// ~JEventProcessor_CPPMVAtree (Destructor)
//------------------
JEventProcessor_CPPMVAtree::~JEventProcessor_CPPMVAtree()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_CPPMVAtree::init(void)
{
	// This is called once at program startup. 

    t1 = new TTree("MVA_TrainingVaraibles", "ntuple");

    //General
    t1->Branch("eventNo", &eventNo, "eventNo/I");
    t1->Branch("nChargedTracks", &nChargedTracks, "nChargedTracks/I");
    t1->Branch("nFCALShowers", &nFCALShowers, "nFCALShowers/I");
    t1->Branch("nFCALhits", &nFCALhits, "nFCALhits/I");
    t1->Branch("nMWPChits", &nMWPChits, "nMWPChits/I");

    //TRACKS
    t1->Branch("p_timebased", p_timebased, "p_timebased[nChargedTracks]/D");
    t1->Branch("p_timebased_x", p_timebased_x, "p_timebased_x[nChargedTracks]/D");
    t1->Branch("p_timebased_y", p_timebased_y, "p_timebased_y[nChargedTracks]/D");
    t1->Branch("p_timebased_z", p_timebased_z, "p_timebased_z[nChargedTracks]/D");
    t1->Branch("charge_timebased", charge_timebased, "charge_timebased[nChargedTracks]/I");
    t1->Branch("mass_timebased", mass_timebased, "mass_timebased[nChargedTracks]/D");
    t1->Branch("FOM_timebased", FOM_timebased, "FOM_timebased[nChargedTracks]/D");

    //FCAL                                                                                                                                           
    t1->Branch("trkpos_fcal_x", trkpos_fcal_x, "trkpos_fcal_x[nChargedTracks]/D");
    t1->Branch("trkpos_fcal_y", trkpos_fcal_y, "trkpos_fcal_y[nChargedTracks]/D");
    t1->Branch("trkpos_fcal_z", trkpos_fcal_z, "trkpos_fcal_z[nChargedTracks]/D");

    t1->Branch("proj_mom_fcal_x", proj_mom_fcal_x, "proj_mom_fcal_x[nChargedTracks]/D");
    t1->Branch("proj_mom_fcal_y", proj_mom_fcal_y, "proj_mom_fcal_y[nChargedTracks]/D");
    t1->Branch("proj_mom_fcal_z", proj_mom_fcal_z, "proj_mom_fcal_z[nChargedTracks]/D");
    t1->Branch("proj_fcal_t", proj_fcal_t, "proj_fcal_t[nChargedTracks]/D");

    //MWPC                                                                                                                                            
    t1->Branch("trkpos_fmwpc_x", trkpos_fmwpc_x, "trkpos_fmwpc_x[nChargedTracks]/D");
    t1->Branch("trkpos_fmwpc_y", trkpos_fmwpc_y, "trkpos_fmwpc_y[nChargedTracks]/D");
    t1->Branch("trkpos_fmwpc_z", trkpos_fmwpc_z, "trkpos_fmwpc_z[nChargedTracks]/D");

    t1->Branch("proj_mom_fmwpc_x", proj_mom_fmwpc_x, "proj_mom_fmwpc_x[nChargedTracks]/D");
    t1->Branch("proj_mom_fmwpc_y", proj_mom_fmwpc_y, "proj_mom_fmwpc_y[nChargedTracks]/D");
    t1->Branch("proj_mom_fmwpc_z", proj_mom_fmwpc_z, "proj_mom_fmwpc_z[nChargedTracks]/D");
    t1->Branch("proj_t_fmwpc", proj_t_fmwpc, "proj_t_fmwpc[nChargedTracks]/D");

    //FCAL Hit Branches
    t1->Branch("hit_row_fcal", hit_row_fcal, "hit_row_fcal[nFCALhits]/I");
    t1->Branch("hit_col_fcal", hit_col_fcal, "hit_col_fcal[nFCALhits]/I");
    t1->Branch("hit_Efcal", hit_Efcal, "hit_Efcal[nFCALhits]/D");
    t1->Branch("hit_tfcal", hit_tfcal, "hit_tfcal[nFCALhits]/D");
    t1->Branch("hit_x_fcal", hit_x_fcal, "hit_x_fcal[nFCALhits]/D");
    t1->Branch("hit_y_fcal", hit_y_fcal, "hit_y_fcal[nFCALhits]/D");
    t1->Branch("hit_intOverPeak_fcal", hit_intOverPeak_fcal, "hit_intOverPeak_fcal[nFCALhits]/D");


    //FCAL Shower Branches
    t1->Branch("shower_pos_fcal_x", shower_pos_fcal_x, "shower_pos_fcal_x[nFCALShowers]/D");
    t1->Branch("shower_pos_fcal_y", shower_pos_fcal_y, "shower_pos_fcal_y[nFCALShowers]/D");
    t1->Branch("shower_time_fcal", shower_time_fcal, "shower_time_fcal[nFCALShowers]/D");
    t1->Branch("shower_nblocks_fcal", shower_nblocks_fcal, "shower_nblocks_fcal[nFCALShowers]/D");

    //MWPC HITS
    t1->Branch("MWPC_hit_layer", MWPC_hit_layer, "MWPC_hit_layer[nMWPChits]/I");
    t1->Branch("MWPC_hit_wire", MWPC_hit_wire, "MWPC_hit_wire[nMWPChits]/I");
    t1->Branch("MWPC_hit_dEwire", MWPC_hit_dEwire,"MWPC_hit_dEwire[nMWPChits]/D");
    t1->Branch("MWPC_hit_twire", MWPC_hit_twire, "MWPC_hit_twire[nMWPChits]/D");
    t1->Branch("MWPC_hit_x", MWPC_hit_x, "MWPC_hit_x[nMWPChits]/D");
    t1->Branch("MWPC_hit_y", MWPC_hit_y,"MWPC_hit_y[nMWPChits]/D");

    //------------ Tree for DFMWPCMatchedTrack objects
    tmatched = new TTree("matched", "DFMWPCMatchedTrack");

    //General
    tmatched->Branch("eventNo", &eventNo, "eventNo/I");
    tmatched->Branch("nChargedTracks", &nChargedTracks, "nChargedTracks/I");
    tmatched->Branch("nFCALShowers", &nFCALShowers, "nFCALShowers/I");
    tmatched->Branch("nFCALhits", &nFCALhits, "nFCALhits/I");
    tmatched->Branch("nMWPChits", &nMWPChits, "nMWPChits/I");
    tmatched->Branch("nFMWPCMatchedTracks", &nFMWPCMatchedTracks, "nFMWPCMatchedTracks/I");

    // TRACK info
    tmatched->Branch("FMWPC_pid", FMWPC_pid, "FMWPC_pid[nFMWPCMatchedTracks]/I");

    // FCAL info
    tmatched->Branch("FCAL_E_center", FCAL_E_center, "FCAL_E_center[nFMWPCMatchedTracks]/D");
    tmatched->Branch("FCAL_E_3x3", FCAL_E_3x3, "FCAL_E_3x3[nFMWPCMatchedTracks]/D");
    tmatched->Branch("FCAL_E_5x5", FCAL_E_5x5, "FCAL_E_5x5[nFMWPCMatchedTracks]/D");

    // FMWPC info
    for( int ilayer=1; ilayer<=6; ilayer++) {
        char branch_name[256];
        sprintf(branch_name, "FMWPC_closest_wire%d", ilayer);
        tmatched->Branch(branch_name, &FMWPC_closest_wire[ilayer-1],      (string(branch_name)+"[nFMWPCMatchedTracks]/I").c_str());
        sprintf(branch_name, "FMWPC_dist_closest_wire%d", ilayer);
        tmatched->Branch(branch_name, &FMWPC_dist_closest_wire[ilayer-1], (string(branch_name)+"[nFMWPCMatchedTracks]/D").c_str());
        sprintf(branch_name, "FMWPC_Nhits_cluster%d", ilayer);
        tmatched->Branch(branch_name, &FMWPC_Nhits_cluster[ilayer-1],     (string(branch_name)+"[nFMWPCMatchedTracks]/I").c_str());
    }

    //------------ Histograms
    tdiff_trk_fcal  = new TH1D("tdiff_trk_fcal", "#deltat between track and matched FCAL hits from DFMWPCMatchedTrack;t_{TRK}-t_{FCAL}(ns)", 101, -50.0, 50.0);
    tdiff_trk_fmwpc = new TH2D("tdiff_trk_fmwpc", "#deltat between track and matched FMWPC hits from DFMWPCMatchedTrack;t_{TRK}-t_{FMWPC}(ns);FMWPC layer", 101, -50.0, 50.0, 6,0.5,6.5);

    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_CPPMVAtree::brun(JEventLoop *loop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_CPPMVAtree::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    vector<const DFCALHit*>        fcalhits;
    vector<const DFCALShower*>     fcalshowers;
    vector<const DFMWPCHit*>       fmwpchits;
    vector<const DTrackTimeBased*> timebasedtracks;
    vector<const DFMWPCMatchedTrack*> fmwpcmatchedtracks;

    loop->Get(fcalhits);
    loop->Get(fcalshowers);
    loop->Get(fmwpchits);
    loop->Get(timebasedtracks);
    loop->Get(fmwpcmatchedtracks);

    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
    
    eventNo        = (Int_t)eventnumber;
    nChargedTracks = 0;
    nFCALhits      = 0;
    nFCALShowers   = 0;
    nMWPChits      = 0;
    nFMWPCMatchedTracks = 0;

    // Timebased tracks
    for( auto tbt : timebasedtracks ){
    
        // Only interested in pion or muon hypotheses
        // (really, only pion will ever be fit)
        if( !( (tbt->PID()==8) || (tbt->PID()==9) ) ) continue;

        // Projections to FCAL and FMWPC
        DVector3 trkpos_fcal, proj_mom_fcal, trkpos_fmwpc, proj_mom_fmwpc;
        double t_fcal=-1000.0, t_fmwpc=-1000.0;
        bool good_fcal_proj  = tbt->GetProjection(SYS_FCAL,  trkpos_fcal,  &proj_mom_fcal,  &t_fcal);
        bool good_fmwpc_proj = tbt->GetProjection(SYS_FMWPC, trkpos_fmwpc, &proj_mom_fmwpc, &t_fmwpc);

        // Do NOT include tracks that do not cleanly project to
        // both the FCAL and FMWPC at this time. We will need to
        // decide later if any such tracks are actually worth keeping
        if( ! (good_fcal_proj && good_fmwpc_proj) ) continue;

        // Time-based track properties
        p_timebased[nChargedTracks]      = tbt->momentum().Mag();
        p_timebased_x[nChargedTracks]    = tbt->momentum().X();
        p_timebased_y[nChargedTracks]    = tbt->momentum().Y();
        p_timebased_z[nChargedTracks]    = tbt->momentum().Z();
        charge_timebased[nChargedTracks] = tbt->charge();
        mass_timebased[nChargedTracks]   = tbt->mass();
        FOM_timebased[nChargedTracks]    = tbt->FOM;

        // Projection to FCAL
        trkpos_fcal_x[nChargedTracks]    = trkpos_fcal.X();
        trkpos_fcal_y[nChargedTracks]    = trkpos_fcal.Y();
        trkpos_fcal_z[nChargedTracks]    = trkpos_fcal.Z();
        proj_mom_fcal_x[nChargedTracks]  = proj_mom_fcal.X();
        proj_mom_fcal_y[nChargedTracks]  = proj_mom_fcal.Y();
        proj_mom_fcal_z[nChargedTracks]  = proj_mom_fcal.Z();
        proj_fcal_t[nChargedTracks]      = t_fcal;

        // Projection to FMWPC
        trkpos_fmwpc_x[nChargedTracks]   = trkpos_fcal.X();
        trkpos_fmwpc_y[nChargedTracks]   = trkpos_fmwpc.Y();
        trkpos_fmwpc_z[nChargedTracks]   = trkpos_fmwpc.Z();
        proj_mom_fmwpc_x[nChargedTracks] = proj_mom_fmwpc.X();
        proj_mom_fmwpc_y[nChargedTracks] = proj_mom_fmwpc.Y();
        proj_mom_fmwpc_z[nChargedTracks] = proj_mom_fmwpc.Z();
        proj_t_fmwpc[nChargedTracks]     = t_fmwpc;

        if( ++nChargedTracks >= nMaxTracks ) break;
    }

    // FCAL Hits
    for( auto fcalhit : fcalhits ){
        
        hit_row_fcal[nFCALhits]         = fcalhit->row;
        hit_col_fcal[nFCALhits]         = fcalhit->column;
        hit_x_fcal[nFCALhits]           = fcalhit->x;
        hit_y_fcal[nFCALhits]           = fcalhit->y;
        hit_Efcal[nFCALhits]            = fcalhit->E;
        hit_tfcal[nFCALhits]            = fcalhit->t;
        hit_intOverPeak_fcal[nFCALhits] = fcalhit->intOverPeak;

        if( ++nFCALhits >= nMaxFCALhits ) break;        
    }

    // FCAL Showers
    for( auto fcalshower : fcalshowers ){

        auto pos = fcalshower->getPosition();
        shower_pos_fcal_x[nFCALShowers]   = pos.X();
        shower_pos_fcal_y[nFCALShowers]   = pos.Y();
        shower_time_fcal[nFCALShowers]    = fcalshower->getTime();
        shower_nblocks_fcal[nFCALShowers] = fcalshower->getNumBlocks();

        if( ++nFCALShowers >= nMaxFCALShowers ) break;        
    }

    // FMWPC Hits
    for( auto fmwpchit : fmwpchits ){

        Int_t layer     = fmwpchit->layer;
        Int_t wire      = fmwpchit->wire;
        Double_t x      = 0;
        Double_t y      = 0;
        Double_t xdelta = 1.016;
        Double_t xmid   = 72.5*xdelta;
        if (layer==1 || layer==3 || layer==5 || layer==6) {
            // chamber is vertical -> wire gives x position
            x = wire*xdelta - xmid;
        } else if (layer==2 || layer==4) {
            // chamber is horizontal -> wire gives y position
            y = wire*xdelta - xmid;
        } else {
            jerr << " Bad FMWPC layer number " << layer << "!" << std::endl;
        }

        MWPC_hit_layer[nMWPChits]  = layer;
        MWPC_hit_wire[nMWPChits]   = wire;
        MWPC_hit_dEwire[nMWPChits] = fmwpchit->dE;
        MWPC_hit_twire[nMWPChits]  = fmwpchit->t;
        MWPC_hit_x[nMWPChits]      = x;
        MWPC_hit_y[nMWPChits]      = y;

        if( ++nMWPChits >= nMaxMWPChits ) break;        
    }

    //MATCHED TRACKS
     for( auto fmwpc_mt : fmwpcmatchedtracks ) {

         FMWPC_pid[nFMWPCMatchedTracks] = fmwpc_mt->PID();
         FCAL_E_center[nFMWPCMatchedTracks] = fmwpc_mt->FCAL_E_center;
         FCAL_E_3x3[nFMWPCMatchedTracks] = fmwpc_mt->FCAL_E_3x3;
         FCAL_E_5x5[nFMWPCMatchedTracks] = fmwpc_mt->FCAL_E_5x5;
         for (int ilayer = 1; ilayer <= 6; ilayer++) {
            FMWPC_closest_wire[ilayer-1][nFMWPCMatchedTracks]      = fmwpc_mt->FMWPC_closest_wire[ilayer-1];
            FMWPC_Nhits_cluster[ilayer-1][nFMWPCMatchedTracks]     = fmwpc_mt->FMWPC_Nhits_cluster[ilayer-1];
            FMWPC_dist_closest_wire[ilayer-1][nFMWPCMatchedTracks] = fmwpc_mt->FMWPC_dist_closest_wire[ilayer-1];
         }

         // Histogram of time diff
         // FCAL
         DVector3 pos;
         double t_fcal_proj=-1000.0;
         if( fmwpc_mt->tbt->GetProjection(SYS_FCAL, pos, NULL, &t_fcal_proj) ) {

             vector<const DFCALHit *> fcalhits;
             fmwpc_mt->Get(fcalhits);
             for (auto hit: fcalhits) {
                 auto delta_t = t_fcal_proj - hit->t;
                 tdiff_trk_fcal->Fill(delta_t);
             }
         }

         // FMPWC
         vector<const DFMWPCHit *> fmwpchits;
         fmwpc_mt->Get(fmwpchits);
         if( !fmwpchits.empty() ) {
             auto fmwpc_projections = fmwpc_mt->tbt->extrapolations.at(SYS_FMWPC);
             for (int layer = 1; layer <= (int) fmwpc_projections.size(); layer++) {
                 auto proj = fmwpc_projections[layer - 1];

                 for (auto hit: fmwpchits) {
                     auto delta_t = proj.t - hit->t;
                     tdiff_trk_fmwpc->Fill(delta_t, hit->layer);
                 }
             }
         }

         if( ++nFMWPCMatchedTracks >= nMaxMWPCMatchedTracks ) break;
     }

     // Fill trees
     t1->Fill();
     tmatched->Fill();

     japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

     return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_CPPMVAtree::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_CPPMVAtree::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}


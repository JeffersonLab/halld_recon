// $Id$
//
//    File: JEventProcessor_fcal_charged.cc
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include <JANA/JEventProcessor.h>
#include <JANA/JEventLoop.h>


#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"


// using namespace jana;
using namespace std;


// Routine used to create our JEventProcessor
#include "JEventProcessor_fcal_charged.h"
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_fcal_charged());   // register this plugin
  }
} // "C"


//------------------
// JEventProcessor_fcal_charged (Constructor)
//------------------
JEventProcessor_fcal_charged::JEventProcessor_fcal_charged()
{
  
}

//------------------
// ~JEventProcessor_fcal_charged (Destructor)
//------------------
JEventProcessor_fcal_charged::~JEventProcessor_fcal_charged()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_fcal_charged::init(void)
{
  // This is called once at program startup. 

  TDirectory *top = gDirectory;
  top->cd();

  gDirectory->mkdir("fcal_charged")->cd();

  h1_events = new TH1D("h1_events",";Events",10,0,10);
  
  top->cd();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_fcal_charged::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_fcal_charged::evnt(JEventLoop *locEventLoop, uint64_t eventnumber)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // loop->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);

  h1_events->Fill(eventnumber%10);
  
  vector<const DFCALShower*> locFCALShowers;
  vector<const DFCALHit*> locFCALHits;
  vector<const DFCALCluster*> locFCALClusters;
  vector<const DFMWPCHit*> fmwpcHits;
  vector<const DChargedTrack*> locChargedTrack;
  vector <const DMCThrown*> mcthrowns;
  const DMCThrown *mcthrown;

  // print out generated parameters

  Double_t pthrown=0;
  locEventLoop->Get(mcthrowns);
  // unsigned int kmax= mcthrowns.size() <=1? mcthrowns.size(): 1 ;    // assumes that the original particle is first in list
  unsigned int kmax= mcthrowns.size();    // assumes that the original particle is first in list

  for (unsigned int k=0; k<kmax;k++){
    //cout << endl << " cppFMWPC testing output " << endl;
    mcthrown = mcthrowns[k];
    // if (mcthrown->charge() !=0 && mcthrown->z()<2 && mcthrown->time()<5 && mcthrown->parentid==0) {
    if (mcthrown->charge() !=0 && mcthrown->z()<2) {
      pthrown = mcthrown->momentum().Mag();
      cout << endl << " Event=" << eventnumber << " k=" << k << " pthrown= " <<  pthrown << " charge=" << mcthrown->charge() << " size=" << mcthrowns.size() << " z=" << mcthrown->z() << " time=" << mcthrown->time() << " parentid=" << mcthrown->parentid << " GEANT PID=" << mcthrown->PID()  << " mass=" << mcthrown->mass() <<  endl;
    }
    else {
      cout << " *** JEventProcessor Loop: charge=" << mcthrown->charge() << " size=" << mcthrowns.size() << " mcthrown->z()=" << mcthrown->z() << " mcthrown->time()=" << mcthrown->time() << " mcthrown->parentid=" << mcthrown->parentid << endl;
    }
  }


  locEventLoop->Get(fmwpcHits);
  locEventLoop->Get(locFCALHits);
  locEventLoop->Get(locFCALClusters);
  locEventLoop->Get(locFCALShowers);
  // locEventLoop->Get(kinfitVertex);
  locEventLoop->Get(locChargedTrack);


  DVector3 trkpos_fcal;
  DVector3 proj_mom_fcal;
  double t_fcal=0;

  DVector3 trkpos_fmwpc;
  DVector3 proj_mom_fmwpc;
  double t_fmwpc=0;


  for (int k=0; k<(int)locChargedTrack.size(); k++) {
    const DChargedTrack *ctrk = locChargedTrack[k];
    const DChargedTrackHypothesis *bestctrack = ctrk->Get_BestTrackingFOM();
    // vector<const DTrackTimeBased*> locTrackTimeBased;
    // bestctrack->Get(locTrackTimeBased);     // locTrackTimeBased[0] contains the best FOM track
    const DTrackTimeBased* locTrackTimeBased = bestctrack->Get_TrackTimeBased();
    double p = locTrackTimeBased->momentum().Mag();
    double q = locTrackTimeBased->charge();
    double trmass = locTrackTimeBased->mass();
    // locTrackTimeBased->setPID (mcthrown->PID()); // Extrapolate using generated particle ID. Propagation through FCAL makes a big difference
    cout << "TrackTimeBased Event=" << eventnumber << " ncharged=" << locChargedTrack.size() << " k=" << k << " p =" << p << " q=" << q << " mass=" << trmass <<  " set to " << locTrackTimeBased->mass() << endl;
    if (locTrackTimeBased->GetProjection(SYS_FCAL,trkpos_fcal,&proj_mom_fcal,&t_fcal)) {
      cout << " Extrapolation fcal x=" << trkpos_fcal.X() << " fcal y=" <<  trkpos_fcal.Y() << " fcal z=" <<  trkpos_fcal.Z() << " px=" << proj_mom_fcal.X() << " fcal py=" <<  proj_mom_fcal.Y() << " fcal pz=" <<  proj_mom_fcal.Z() << " fcal p=" << proj_mom_fcal.Mag() <<" fcal t=" << t_fcal << endl;
    }
    if (locTrackTimeBased->GetProjection(SYS_FMWPC,trkpos_fmwpc,&proj_mom_fmwpc,&t_fmwpc)) {
      cout << " Extrapolation fmwpc x=" << trkpos_fmwpc.X() << " fmwpc y=" <<  trkpos_fmwpc.Y() << " fmwpc z=" <<  trkpos_fmwpc.Z() << " px=" << proj_mom_fmwpc.X() << " fmwpc py=" <<  proj_mom_fmwpc.Y() << " fmwpc pz=" <<  proj_mom_fmwpc.Z() << " fmwpc p=" << proj_mom_fmwpc.Mag() << " fmwpc t=" << t_fmwpc << endl;
    }
  }

  for (unsigned int j=0; j < locFCALHits.size(); ++j) {
       int row = locFCALHits[j]->row;
       int col = locFCALHits[j]->column;
       double x = locFCALHits[j]->x;
       double y = locFCALHits[j]->y;
       double Efcal = locFCALHits[j]->E;
       double tfcal= locFCALHits[j]->t;
       double intOverPeak = locFCALHits[j]->intOverPeak;
       cout << "FCAL Hits Event=" << eventnumber << " row=" << row << " col=" << col << " x=" << x << " y=" << y << " Efcal=" << Efcal << " tfcal=" << tfcal << " intOverPeak=" << intOverPeak << endl;
  }

  
  for (unsigned int j=0; j < locFCALShowers.size(); ++j) {
    DVector3 pos  = locFCALShowers[j]->getPosition();
    double time  = locFCALShowers[j]->getTime();
    double nblocks  = locFCALShowers[j]->getNumBlocks();
    cout << "FCAL Shower Event=" << eventnumber << " j=" << j << " X=" << pos.X() << " Y=" << pos.Y() << " time=" << time << " nblocks=" << nblocks << endl;
  }

  Double_t xdelta=1.016;
  Double_t xmid=72.5*xdelta;

  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    const DFMWPCHit *hit1 = fmwpcHits[k];
    Int_t layer = hit1->layer;
    Int_t wire = hit1->wire;
    Double_t q = hit1->q;
    Double_t twire = hit1->t;
    Double_t x = 0;
    Double_t y = 0;
    if (layer==1 || layer==3 || layer==5 || layer==6) {
      // chamber is vertical -> wire gives x position
      x = wire*1.016 - xmid;
    }
    if (layer==2 || layer==4) {
      // chamber is horizontal -> wire gives y position
      y = wire*1.016 - xmid;
    }
    cout << "FMWPC Hits Event=" << eventnumber << " k=" << k << " layer=" << layer <<  " wire=" << wire << " q=" << q << " x=" << x << " y=" << y << " time=" << twire << endl;
  }


  // If Historgrams are to be filled. Locks need to be in place
  //  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  // FILL HISTOGRAMS
  // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
  //  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK


  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_fcal_charged::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_fcal_charged::fini(void)
{
  // Called before program exit after event processing is finished.

  return NOERROR;
}


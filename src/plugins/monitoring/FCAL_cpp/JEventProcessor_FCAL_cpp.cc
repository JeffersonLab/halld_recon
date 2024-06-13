// $Id$
//
//    File: JEventProcessor_FCAL_cpp.cc
// Created: Mon Feb 19 16:33:33 EST 2024
// Creator: ilarin (on Linux ifarm1801.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_cpp.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>


#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALShower.h>
#include <TRIGGER/DL1Trigger.h>
#include <PID/DEventRFBunch.h>


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_FCAL_cpp());
}
} // "C"


//------------------
// JEventProcessor_FCAL_cpp (Constructor)
//------------------
JEventProcessor_FCAL_cpp::JEventProcessor_FCAL_cpp()
{

}

//------------------
// ~JEventProcessor_FCAL_cpp (Destructor)
//------------------
JEventProcessor_FCAL_cpp::~JEventProcessor_FCAL_cpp()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCAL_cpp::init(void)
{
	// This is called once at program startup. 

  memset(fcal_nhit,0,sizeof(fcal_nhit));
  memset(fcal_nclust,0,sizeof(fcal_nclust));
  memset(fcal_nhitonly,0,sizeof(fcal_nhitonly));
  nfcaltriggers = 0;  ntoftriggers = 0;

  // create root folder for this plugin and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("FCAL_cpp")->cd();


  h_fcalshower_tall     = new TProfile("h_fcalshower_tall",  "Shower Time Alignment",  N_FCAL_BLOCKS, -0.5, N_FCAL_BLOCKS-0.5, -50., 50.);
  h_fcalshower_tw       = new TProfile("h_fcalshower_tw",    "Shower Time Walk",  100, 0., 10, -20., 20.);
  h_fcal_nclust   = new TH2F("h_fcal_nclust","Cluster Occupancy", 59, -0.5, 58.5, 59, -0.5, 58.5);
  h_fcal_nhitonly = new TH2F("h_fcal_nhitonly","Missed Trigger Hit Occupancy", 59, -0.5, 58.5, 59, -0.5, 58.5);
  h_fcal_hcr      = new TH2F("h_fcal_hcr","Noise Hit Occupancy", 59, -0.5, 58.5, 59, -0.5, 58.5);

  h_fcalshower_tall_map = new TProfile2D("h_fcalshower_tall_map","FCAL Shower Time Shift Map", 59, -0.5, 58.5, 59, -0.5, 58.5);
  h_fcalehit_tall_map   = new TProfile2D("h_fcalehit_tall_map","FCAL Low-Energy Hit Map", 59, -0.5, 58.5, 59, -0.5, 58.5);

  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCAL_cpp::brun(JEventLoop *loop, int32_t runnumber)
{
	// This is called whenever the run number changes

  vector<const DFCALGeometry*> fcalGeomVect;
  loop->Get(fcalGeomVect);
  fcalGeom = fcalGeomVect[0];

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCAL_cpp::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  vector<const DL1Trigger*> l1trig;
  loop->Get(l1trig);

  int fcaltrigger = 0, toftrigger = 0;
  if(l1trig.size()) {
    if( (l1trig[0]->trig_mask & (1 << (1-1))) ) fcaltrigger = 1;
    if( (l1trig[0]->trig_mask & (1 << (2-1))) && ! fcaltrigger) toftrigger = 1;
  }

  if(!fcaltrigger&&!toftrigger) return NOERROR;
  if(fcaltrigger) ++nfcaltriggers;
  if(toftrigger) ++ntoftriggers;

  vector<const DFCALHit*> fcal_hits;
  loop->Get(fcal_hits);

  vector<const DFCALCluster*> fcal_clusters;
  loop->Get(fcal_clusters);

  vector<const DFCALShower*> fcal_showers;
  loop->Get(fcal_showers);

  double efcaltot = 0., efcalmax = 0.;
  for(unsigned int i = 0; i<fcal_hits.size(); ++i) {
    const DFCALHit *fcal_hit = fcal_hits[i];
    double fcale = fcal_hit->E;
    if(fcale>0.05)    efcaltot += fcale;
    if(fcale>efcalmax) efcalmax = fcale;
  }

  for(unsigned int i = 0; i<fcal_hits.size(); ++i) {
    const DFCALHit *fcal_hit = fcal_hits[i];
    double fcale = fcal_hit->E;
    int row = fcal_hit->row;
    int col = fcal_hit->column;

    if(fcaltrigger&&fcale>1.0&&(efcaltot-fcale)<1.0&&fcale>0.9*efcalmax) fcal_nhitonly[row][col][0]  += 1.;
    if(toftrigger&&fcale>1.0&&(efcaltot-fcale)<1.0&&fcale>0.9*efcalmax) fcal_nhitonly[row][col][1]   += 1.;

    if(fcaltrigger&&fcale>0.3) fcal_nhit[row][col] += 1.;
    if(fcale<0.3) {
      japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
      h_fcalehit_tall_map->Fill(col,row,fcale);
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
    }
  }

  for(unsigned int i = 0; i<fcal_clusters.size(); ++i) {
    const DFCALCluster *fcal_cluster = fcal_clusters[i];
    int block = fcal_cluster->getChannelEmax();
    double e  = fcal_cluster->getEnergy();
    int dim   = fcal_cluster->getHits();
    int row   = fcalGeom->row(block);
    int col   = fcalGeom->column(block);
    if(fcaltrigger&&e>0.3&&dim>2) fcal_nclust[row][col] += 1.;
  }

  for(unsigned int i = 0; i<fcal_showers.size(); ++i) {
    const DFCALShower *fcal_shower = fcal_showers[i];
    double t  = fcal_shower->getTime();
    double e  = fcal_shower->getEnergy();
	  const DFCALCluster* locAssociatedCluster = NULL;
	  fcal_shower->GetSingle(locAssociatedCluster);
	  if(!locAssociatedCluster) continue;
    int   id  = locAssociatedCluster->getChannelEmax();
    int row   = fcalGeom->row(id);
    int col   = fcalGeom->column(id);
    for(unsigned int j = i+1; j<fcal_showers.size(); ++j) {
      const DFCALShower *fcal_shower2 = fcal_showers[j];
      double t2 = fcal_shower2->getTime();
      double e2 = fcal_shower2->getEnergy();
      if(e2>1.0&&e2<3.0) {
        japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
        if(e>1.0&&e<3.0) {
          h_fcalshower_tall->Fill(id,t-t2);
          h_fcalshower_tall_map->Fill(col,row,t-t2);
        }
        if(e2>1.5&&e2<2.5)  h_fcalshower_tw->Fill(e,t-t2);
        japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
      }
    }
  }

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCAL_cpp::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCAL_cpp::fini(void)
{
	// Called before program exit after event processing is finished.

  if(nfcaltriggers>1000)
  for(int i = 0; i<59;  ++i)
    for(int j = 0; j<59;  ++j) {

      h_fcal_nclust->SetBinContent(i+1, j+1, fcal_nclust[j][i]/double(nfcaltriggers));
      h_fcal_nclust->SetBinError(i+1, j+1, sqrt(fcal_nclust[j][i])/double(nfcaltriggers));

      if(ntoftriggers) {
        int n0 = fcal_nhitonly[j][i][0], n1 = fcal_nhitonly[j][i][1];

        double p, ep;

        if(n0==0 && n1==0) {
          p   = 0.;
          ep  = 1.;
        }

        if(n0==0 && n1>0) {
          p   = 1.;
          ep  = 1./double(n1);
        }

        if(n0>0  && n1==0) {
          p   = 0.;
          ep  = 1./double(n0);
        }

        if(n0>0  && n1>0) {
          p   = double(n1) / double(n0+n1);
          ep  = sqrt( p*(1.-p)/double(n0+n1) );
        }

        h_fcal_nhitonly->SetBinContent(i+1, j+1,  p);
        h_fcal_nhitonly->SetBinError(i+1, j+1, ep);
      }

      if(fcal_nclust[i][j]) {
        double r = fcal_nhit[j][i]/fcal_nclust[j][i];
        h_fcal_hcr->SetBinContent(i+1, j+1, r);
        h_fcal_hcr->SetBinError(i+1, j+1, sqrt(fcal_nhit[j][i]*(1.+r))/fcal_nclust[j][i]);
      }
    }

	return NOERROR;
}


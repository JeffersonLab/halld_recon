// $Id$
//
//    File: JEventProcessor_FMWPC_Performance.cc
// Created: Fri Mar  4 10:17:38 EST 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_FMWPC_Performance.h"
using namespace jana;

#include "FMWPC/DFMWPCCluster.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"

#include <TH1.h>
#include <TH2.h>

static TH1D *hfmwpc_residual[6];
static TH2D *hfmwpc_residual_vs_x[6];
static TH2D *hfmwpc_residual_vs_y[6];

static TH2D *hfmwpc_expected[6];
static TH2D *hfmwpc_measured[6];

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_FMWPC_Performance());
}
} // "C"


//------------------
// JEventProcessor_FMWPC_Performance (Constructor)
//------------------
JEventProcessor_FMWPC_Performance::JEventProcessor_FMWPC_Performance()
{

}

//------------------
// ~JEventProcessor_FMWPC_Performance (Destructor)
//------------------
JEventProcessor_FMWPC_Performance::~JEventProcessor_FMWPC_Performance()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FMWPC_Performance::init(void)
{
  // This is called once at program startup. 

  // create root folder for fdc and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("FMWPC_Performance")->cd();
  gDirectory->mkdir("Alignment")->cd();

  for(int layer=1; layer<=6; layer++){
      
    char hname[256];
    sprintf(hname, "hfmwpc_residual_layer%d", layer);
    hfmwpc_residual[layer-1] = new TH1D(hname, "", 200, -10, 10);

    sprintf(hname, "hfmwpc_residual_vs_x_layer%d", layer);
    hfmwpc_residual_vs_x[layer-1] = new TH2D(hname, "", 200, -72.5, 72.5, 200, -10, 10);

    sprintf(hname, "hfmwpc_residual_vs_y_layer%d", layer);
    hfmwpc_residual_vs_y[layer-1] = new TH2D(hname, "", 200, -72.5, 72.5, 200, -10, 10);

  }	
  gDirectory->cd("/FMWPC_Performance");

  gDirectory->mkdir("Efficiency")->cd();

  for(int layer=1; layer<=6; layer++){
      
    char hname[256];
    sprintf(hname, "hfmwpc_expected_layer%d", layer);
    hfmwpc_expected[layer-1] = new TH2D(hname, "", 200, -72.5, 72.5, 200, -72.5, 72.5);

    sprintf(hname, "hfmwpc_measured_layer%d", layer);
    hfmwpc_measured[layer-1] = new TH2D(hname, "", 200, -72.5, 72.5, 200, -72.5, 72.5);
  }	

  main->cd();
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FMWPC_Performance::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FMWPC_Performance::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	
  // get fmwpc clusters
  vector<const DFMWPCCluster*> locFMWPCClusters;
  loop->Get(locFMWPCClusters);
  
  // pre-sort clusters to save time
  // only need to search within the given layer
  map<int, set<const DFMWPCCluster*> > locSortedFMWPCClusters; // int: layer
  for( unsigned int clusNum = 0; clusNum < locFMWPCClusters.size(); clusNum++){
    const DFMWPCCluster * locCluster = locFMWPCClusters[clusNum];
    locSortedFMWPCClusters[locCluster->layer-1].insert(locCluster);
  }
  
  // get charged tracks
  vector <const DChargedTrack *> chargedTrackVector;
  loop->Get(chargedTrackVector);
  
  for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){
    
    const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();
    
    // Cut very loosely on the track quality
    auto thisTimeBasedTrack = bestHypothesis->Get_TrackTimeBased();
    
    // Get all extrapolations and loop over layers
    auto fmwpc_projections = thisTimeBasedTrack->extrapolations.at(SYS_FMWPC);
    for( int layer=1; layer<=(int)fmwpc_projections.size(); layer++){
      auto proj = fmwpc_projections[layer-1];

      // FILL HISTOGRAMS
      japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
      hfmwpc_expected[layer-1]->Fill(proj.position.x(), proj.position.y());
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

      set<const DFMWPCCluster*>& locLayerClusters = locSortedFMWPCClusters[layer-1];
      // loop over the FMWPC Clusters in that layer
      for(set<const DFMWPCCluster*>::iterator locIterator = locLayerClusters.begin();  locIterator !=  locLayerClusters.end(); ++locIterator)
	{
	  const DFMWPCCluster * locCluster = * locIterator;
	  if (locCluster == NULL) continue;

	  double residual = 0;
	  if (locCluster->pos.y() == -777) // plane measures X
	    residual = proj.position.x() - locCluster->pos.x();
	  else  // plane measures Y
	    residual = proj.position.y() - locCluster->pos.y();

	  // FILL HISTOGRAMS
	  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	  hfmwpc_residual[layer-1]->Fill(residual);
	  hfmwpc_residual_vs_x[layer-1]->Fill(proj.position.x(), residual);
	  hfmwpc_residual_vs_y[layer-1]->Fill(proj.position.y(), residual);
	  if (residual < 5)
	    hfmwpc_measured[layer-1]->Fill(proj.position.x(), proj.position.y());
	  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
		  
	}
    }
  }
  
  
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FMWPC_Performance::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FMWPC_Performance::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}


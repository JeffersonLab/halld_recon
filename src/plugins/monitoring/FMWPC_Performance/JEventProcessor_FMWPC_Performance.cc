// $Id$
//
//    File: JEventProcessor_FMWPC_Performance.cc
// Created: Fri Mar  4 10:17:38 EST 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_FMWPC_Performance.h"
using namespace jana;

#include "FMWPC/DFMWPCCluster.h"
#include "FMWPC/DCPPEpEm.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"

#include <TH1.h>
#include <TH2.h>

static TH1D *hfmwpc_residual[6];
static TH2D *hfmwpc_residual_vs_x[6];
static TH2D *hfmwpc_residual_vs_y[6];

static TH2D *hfmwpc_expected[6];
static TH2D *hfmwpc_measured[6];

static TH1D *hinvmass_nocut;
static TH1D *hinvmass_pipi;
static TH1D *hinvmass_mumu;
static TH1D *hpimu_ML_classifier;

static TH1D *hpsi_nocut;
static TH1D *hpsi_pipi;
static TH1D *hpsi_mumu;


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

  gDirectory->cd("/FMWPC_Performance");

  gDirectory->mkdir("Reconstructed")->cd();
  hinvmass_nocut = new TH1D("hinvmass_nocut", "Inv. Mass #pi^{+}#pi^{-} - No cut;inv. mass (MeV/c^{2})", 200, 250.0, 650.0);
  hinvmass_pipi = new TH1D("hinvmass_pipi", "Inv. Mass #pi^{+}#pi^{-} - ML=#pi;inv. mass (MeV/c^{2})", 200, 250.0, 650.0);
  hinvmass_mumu = new TH1D("hinvmass_mumu", "Inv. Mass #pi^{+}#pi^{-} - ML=#mu;inv. mass (MeV/c^{2})", 200, 250.0, 650.0);
  hpimu_ML_classifier = new TH1D("hpimu_ML_classifier", "ML model classifier for #pi/#mu;classifier value", 200, 0.0, 1.0);
  
  hinvmass_mumu->SetLineColor(kBlue);
  hinvmass_pipi->SetLineColor(kRed);
  
  hpsi_nocut = new TH1D("hpsi_nocut", "#psi;#psi (radians)", 200, -0.01, 6.4);
  hpsi_pipi = new TH1D("hpsi_pipi", "#psi;#psi (radians)", 200, -0.01, 6.4);
  hpsi_mumu = new TH1D("hpsi_mumu", "#psi;#psi (radians)", 200, -0.01, 6.4);

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

  // get kinematic fit values + pi/mu classification
  vector<const DCPPEpEm*> locCPPEpEms;
  loop->Get(locCPPEpEms);
	
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
  
  // Invariant mass
  for( auto cppepem : locCPPEpEms ){
  	auto &pip_v4 = cppepem->pip_v4;
  	auto &pim_v4 = cppepem->pim_v4;
  	auto invmass = 1E3*(pip_v4 + pim_v4).M(); // in MeV !
	auto &pimu_ML_classifier = cppepem->pimu_ML_classifier;
	hinvmass_nocut->Fill( invmass );

	double model_cut = 0.5;
	hpimu_ML_classifier->Fill( pimu_ML_classifier );
	if( (pimu_ML_classifier>=0.0      ) && (pimu_ML_classifier<model_cut) ) hinvmass_pipi->Fill( invmass );
	if( (pimu_ML_classifier>=model_cut) && (pimu_ML_classifier<=1.0     ) ) hinvmass_mumu->Fill( invmass );
	
	//------ Rory's recipe ------
	TVector3 k_hat = (pip_v4+pim_v4).Vect();
	k_hat.SetMag(1.0);

	// Rotation angle about y to get into y-z plane
	double theta_yz = atan2(-k_hat.X(), k_hat.Z());
	k_hat.RotateY(theta_yz);

	// pi+ direction in rotated lab frame
	TVector3 pip_hat = pip_v4.Vect();
	pip_hat.RotateY(theta_yz);

	// Angle to rotate k_hat into z-direction
	double theta_x = atan2(k_hat.Y(), k_hat.Z());
	k_hat.RotateX(theta_x);
	pip_hat.RotateX(theta_x);
	double psi = pip_hat.Phi();
	if(psi<0.0)psi += TMath::TwoPi();
	//----------------------------
	
	hpsi_nocut->Fill( psi );
	if( (pimu_ML_classifier>=0.0      ) && (pimu_ML_classifier<model_cut) ) hpsi_pipi->Fill( psi );
	if( (pimu_ML_classifier>=model_cut) && (pimu_ML_classifier<=1.0     ) ) hpsi_mumu->Fill( psi );
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


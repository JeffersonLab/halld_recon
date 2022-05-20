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

static TH1D *hpsi_nocut_rory;
static TH1D *hpsi_pipi_rory;
static TH1D *hpsi_mumu_rory;

static TH1D *hpsi_nocut;
static TH1D *hpsi_pipi;
static TH1D *hpsi_mumu;

static TH1D *hPhi_nocut;
static TH1D *hPhi_pipi;
static TH1D *hPhi_mumu;

static TH1D *hphi_nocut;
static TH1D *hphi_pipi;
static TH1D *hphi_mumu;

static TH1D *hCosTheta_nocut;
static TH1D *hCosTheta_pipi;
static TH1D *hCosTheta_mumu;

static TH2D *hCosTheta_vs_psi_nocut;
static TH2D *hCosTheta_vs_psi_pipi;
static TH2D *hCosTheta_vs_psi_mumu;

static TH2D *hphi_vs_Phi_nocut;
static TH2D *hphi_vs_Phi_pipi;
static TH2D *hphi_vs_Phi_mumu;

static TH1D *hphiJT;

static TH1D *hpimem_ML_classifier;
static TH1D *hpipep_ML_classifier;


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
  
  hpsi_nocut_rory = new TH1D("hpsi_nocut_rory", "#psi;#psi (radians)", 200, -0.01, 6.4);
  hpsi_pipi_rory = new TH1D("hpsi_pipi_rory", "#psi;#psi (radians)", 200, -0.01, 6.4);
  hpsi_mumu_rory = new TH1D("hpsi_mumu_rory", "#psi;#psi (radians)", 200, -0.01, 6.4);

  hpsi_nocut = new TH1D("hpsi_nocut", "#psi;#psi (degrees)", 180,-180,180);
  hpsi_pipi = new TH1D("hpsi_pipi", ";#psi;#psi (degrees)", 180,-180,180);
  hpsi_mumu = new TH1D("hpsi_mumu", "#psi;#psi (degrees)", 180,-180,180);

  hPhi_nocut =  new TH1D("hPhi_nocut", "#Phi ;#Phi (degrees)", 180,-180,180);
  hPhi_pipi = new TH1D("hPhi_pipi", "#Phi ;#Phi (degrees)", 180,-180,180);
  hPhi_mumu = new TH1D("hPhi_mumu", "#Phi ;#Phi (degrees)", 180,-180,180);

  hphi_nocut = new TH1D("hphi_nocut", "#phi ;#phi (degrees)", 180,-180,180);
  hphi_pipi = new TH1D("hphi_pipi", "#phi ;#phi (degrees)", 180,-180,180);
  hphi_mumu = new TH1D("hphi_mumu", "#phi ;#phi (degrees)", 180,-180,180);

  hCosTheta_nocut = new TH1D("hCosTheta_nocut", "Cos#Theta ;Cos#Theta", 100, -1, 1);
  hCosTheta_pipi = new TH1D("hCostTheta_pipi", "Cos#Theta ;Cos#Theta", 100, -1, 1);
  hCosTheta_mumu = new TH1D("hCostTheta_mumu", "Cos#Theta ;Cos#Theta", 100, -1, 1);

  hCosTheta_vs_psi_nocut = new TH2D("hCosTheta_vs_psi_nocut", "#Cos#Theta vs #psi ;#psi (degrees) ;Cos#Theta", 180, -180., 180, 100, -1., 1.);
  hCosTheta_vs_psi_pipi = new TH2D("hCosTheta_vs_psi_pipi", "#Cos#Theta vs #psi ;#psi (degrees) ;Cos#Theta", 180, -180., 180, 100, -1., 1.);
  hCosTheta_vs_psi_mumu = new TH2D("hCosTheta_vs_psi_mumu", "#Cos#Theta vs #psi ;#psi (degrees) ;Cos#Theta", 180, -180., 180, 100, -1., 1.);

  hphi_vs_Phi_nocut = new TH2D("hphi_vs_Phi_nocut", "#phi vs #Phi ;#Phi ;#phi", 90, -180, 180, 90, -180, 180); 
  hphi_vs_Phi_pipi = new TH2D("hphi_vs_Phi_pipi", "#phi vs #Phi ;#Phi ;#phi", 90, -180, 180, 90,-180, 180);
  hphi_vs_Phi_mumu = new TH2D("hphi_vs_Phi_mumu", "#phi vs #Phi ;#Phi ;#phi", 90, -180, 180, 90,-180, 180);

  hphiJT = new TH1D("hphiJT", "#vec{J}_{T}.#phi() ;#phi (degrees)", 90, -180, 180);
  
  hpimem_ML_classifier = new TH1D("hpimem_ML_classifier", "ML model classifier for #pi^{-}/e^{-};classifier value", 200, 0.0, 1.0);
  hpipep_ML_classifier = new TH1D("hpipep_ML_classifier", "ML model classifier for #pi^{+}/e^{+};classifier value", 200, 0.0, 1.0);
  
  
  
   
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
	auto &Ebeam = cppepem->Ebeam;
	auto &weight = cppepem->weight;
	
	DLorentzVector Pb208Target, locMissingPb208P4, locBeamP4;
	Pb208Target.SetXYZM(0,0,0,193.750748);
	locBeamP4.SetXYZM(0,0,Ebeam,0);
	locMissingPb208P4 = locBeamP4 + Pb208Target - pim_v4 - pip_v4;

        auto invmass = 1E3*(pip_v4 + pim_v4).M(); // in MeV !
	auto &pimu_ML_classifier = cppepem->pimu_ML_classifier;
	double model_cut = 0.5;

	
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
	double psi_rory = pip_hat.Phi();
	if(psi_rory<0.0)psi_rory += TMath::TwoPi();
	//----------------------------


	//------- rho study recipe --------
	TLorentzRotation resonanceBoost2( -(pip_v4 + pim_v4).BoostVector() );   // boost into 2pi frame                                                     
	TLorentzVector beam_res = resonanceBoost2 * locBeamP4;
	TLorentzVector recoil_res = resonanceBoost2 * locMissingPb208P4;
	TLorentzVector p1_res = resonanceBoost2 * pip_v4;
	TLorentzVector p2_res = resonanceBoost2 * pim_v4;
	double phipol = 0;
	TVector3 eps(cos(phipol), sin(phipol), 0.0);
	TVector3 y = (locBeamP4.Vect().Unit().Cross(-locMissingPb208P4.Vect().Unit())).Unit();
	// choose helicity frame: z-axis opposite recoil lead in rho rest frame 
	TVector3 z = -1. * recoil_res.Vect().Unit();
	TVector3 x = y.Cross(z).Unit();

	TVector3 angles( (p1_res.Vect()).Dot(x),
			    (p1_res.Vect()).Dot(y),
			    (p1_res.Vect()).Dot(z) );

	double CosTheta = angles.CosTheta();
	double phi = angles.Phi();
	double Phi = atan2(y.Dot(eps), locBeamP4.Vect().Unit().Dot(eps.Cross(y)));
	double psi = Phi - phi;
	if(psi < -3.14159) psi += 2*3.14159;
	if(psi > 3.14159) psi -= 2*3.14159;
	//---------------------------------
	
	//---- Polarization observable from e+ e-
	auto &ep_v4 = cppepem->ep_v4;
	auto &em_v4 = cppepem->em_v4;
	double ep_v3mag = ep_v4.Vect().Mag(); //sqrt(p1.X()*p1.X() + p1.Y()*p1.Y() + p1.Z()*p1.Z());
	double em_v3mag = em_v4.Vect().Mag(); 
	double JTx = ep_v4.X() * 2*em_v4.E()/(ep_v4.E() - ep_v3mag * cos(ep_v4.Theta()))  +
	             em_v4.X() * 2*ep_v4.E()/(em_v4.E() - em_v3mag * cos(em_v4.Theta()));

	double JTy = ep_v4.Y() * 2*em_v4.E()/(ep_v4.E() - ep_v3mag * cos(ep_v4.Theta()))  +
	             em_v4.Y() * 2*ep_v4.E()/(em_v4.E() - em_v3mag * cos(em_v4.Theta()));

        double Jphi = atan2(JTy, JTx)*180/acos(-1);
	double MLPClassifierMinus=cppepem->pimem_ML_classifier;
	hpimem_ML_classifier->Fill(MLPClassifierMinus);
  	double MLPClassifierPlus=cppepem->pipep_ML_classifier;
	hpipep_ML_classifier->Fill(MLPClassifierPlus);
	if(MLPClassifierPlus > 0.8 && MLPClassifierMinus > 0.8){
	  hphiJT->Fill(Jphi, weight);
	}
        
	//------- no MVA cut histograms ---------
	hinvmass_nocut->Fill( invmass, weight );
	hpimu_ML_classifier->Fill( pimu_ML_classifier );

	hpsi_nocut_rory->Fill( psi_rory );
	hpsi_nocut->Fill(psi*TMath::RadToDeg(), weight);
	hPhi_nocut->Fill(Phi*TMath::RadToDeg(), weight);
	hphi_nocut->Fill(phi*TMath::RadToDeg(), weight);
	hCosTheta_nocut->Fill(CosTheta, weight);	
	hCosTheta_vs_psi_nocut->Fill(psi*TMath::RadToDeg(), CosTheta);
	hphi_vs_Phi_nocut->Fill(Phi*TMath::RadToDeg(), phi*TMath::RadToDeg());


	//------- pion selection -------
	if( (pimu_ML_classifier>=0.0      ) && (pimu_ML_classifier<model_cut) )
	  {
	    hinvmass_pipi->Fill( invmass, weight );
	    hpsi_pipi_rory->Fill( psi_rory );
	    hpsi_pipi->Fill(psi*TMath::RadToDeg(), weight);
	    hPhi_pipi->Fill(Phi*TMath::RadToDeg(), weight);
	    hphi_pipi->Fill(phi*TMath::RadToDeg(), weight);
	    hCosTheta_pipi->Fill(CosTheta,weight);
	    hCosTheta_vs_psi_pipi->Fill(psi*TMath::RadToDeg(), CosTheta);
	    hphi_vs_Phi_pipi->Fill(Phi*TMath::RadToDeg(), phi*TMath::RadToDeg());
	  }

	//------- muon selection -------
	if( (pimu_ML_classifier>=model_cut) && (pimu_ML_classifier<=1.0     ) )
	  {
	    hinvmass_mumu->Fill( invmass, weight );
	    hpsi_mumu_rory->Fill( psi_rory );
	    hpsi_mumu->Fill(psi*TMath::RadToDeg(), weight);
	    hPhi_mumu->Fill(Phi*TMath::RadToDeg(), weight);
	    hphi_mumu->Fill(phi*TMath::RadToDeg(), weight);
	    hCosTheta_mumu->Fill(CosTheta,weight);
	    hCosTheta_vs_psi_mumu->Fill(psi*TMath::RadToDeg(), CosTheta);
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


// $Id$
//
//    File: JEventProcessor_epem_ml_skim.cc
// Created: Mon Jun  5 10:42:13 EDT 2023
// Creator: acschick (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_epem_ml_skim.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

#include "FMWPC/DCPPEpEm.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"

#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TTree.h>
#include "TProfile.h"
#include <TDirectory.h>


static TH1D *htrack_mom;
static TH1D *htrack_mom_ok;
static TH1D *htrack_chi2;
static TH1D *htrack_chi2_ok;
static TH1D *htrack_theta;
static TH1D *htrack_theta_ok;
static TH1D *htrack_phi;
static TH1D *htrack_phi_ok;


static TH1D *hphiJT;
static TH1D *hinvmass_ee;
static TH1D *hFCALElasticity;

static TH1D *hpimem_ML_classifier;
static TH1D *hpipep_ML_classifier;


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_epem_ml_skim());
}
} // "C"


//------------------
// JEventProcessor_epem_ml_skim (Constructor)
//------------------
JEventProcessor_epem_ml_skim::JEventProcessor_epem_ml_skim()
{

}

//------------------
// ~JEventProcessor_epem_ml_skim (Destructor)
//------------------
JEventProcessor_epem_ml_skim::~JEventProcessor_epem_ml_skim()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_epem_ml_skim::init(void)
{
  // This is called once at program startup. 

  events_read           = 0;
  events_written        = 0;

  TDirectory *main = gDirectory;
  gDirectory->mkdir("epem_ml_skim")->cd();
  gDirectory->mkdir("Tracks")->cd();

  htrack_mom = new TH1D("htrack_mom", ";p (GeV/c)", 200, 0, 10);
  htrack_mom_ok = new TH1D("htrack_mom_ok", ";p (GeV/c)", 200, 0, 10);
  htrack_chi2 = new TH1D("htrack_chi2","; #chi^{2}/NDF", 500, 0.0, 1.0);
  htrack_chi2_ok = new TH1D("htrack_chi2_ok","; #chi^{2}/NDF", 500, 0.0, 1.0);
  htrack_theta = new TH1D("htrack_theta","; Track #theta", 500, 0.0, 50);
  htrack_theta_ok = new TH1D("htrack_theta_ok","; Track #theta", 500, 0.0, 50);
  htrack_phi = new TH1D("htrack_phi"," ; Track #phi", 360, -180, 180);
  htrack_phi_ok = new TH1D("htrack_phi_ok"," ; Track #phi", 360, -180, 180);

  gDirectory->cd("/epem_ml_skim");
  gDirectory->mkdir("Reconstructed")->cd();

  hphiJT = new TH1D("hphiJT", "#vec{J}_{T}.#phi() ;#phi (degrees)", 90, -180, 180);
  hinvmass_ee = new TH1D("hinvmass_ee", "Inv. Mass e^{+}e^{-} ;inv. mass (MeV/c^{2})", 234, 0.0, 1.0);
  hFCALElasticity = new TH1D("hFCALElasticity", "; (E^{FCAL}_{1} + E^{FCAL}_{2})/E_{0}", 200, 0, 1.5);

  hpimem_ML_classifier = new TH1D("hpimem_ML_classifier", "ML model classifier for #pi^{-}/e^{-};classifier value", 200, 0.0, 1.0);
  hpipep_ML_classifier = new TH1D("hpipep_ML_classifier", "ML model classifier for #pi^{+}/e^{+};classifier value", 200, 0.0, 1.0);

  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_epem_ml_skim::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_epem_ml_skim::evnt(JEventLoop *loop, uint64_t eventnumber)
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
	//  ... fill histograms or trees ...
	// japp->RootFillUnLock(this);



  ++events_read;
  //  if( eventnumber%100 == 0){
    vector<const DCPPEpEm*> locCPPEpEms;
    loop->Get(locCPPEpEms);

    // get charged tracks                                                                                                                                    
    vector <const DChargedTrack *> chargedTrackVector;
    loop->Get(chargedTrackVector);

    // get detector matches                                                                                                                                  
    const DDetectorMatches *detMatches = nullptr;
    loop->GetSingle(detMatches);


    for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){
      const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();
      // Cut very loosely on the track quality                                                                                                               
      auto thisTimeBasedTrack = bestHypothesis->Get_TrackTimeBased();

      japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK                                                                                                     
      htrack_mom->Fill(thisTimeBasedTrack->pmag());
      htrack_chi2->Fill(thisTimeBasedTrack->FOM);
      htrack_theta->Fill(thisTimeBasedTrack->momentum().Theta()*TMath::RadToDeg());
      htrack_phi->Fill(thisTimeBasedTrack->momentum().Phi()*TMath::RadToDeg());
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK                                                                                                   

      // At least one match either to the Time-Of-Flight OR the FCAL                                                                                         
      if ( !detMatches->Get_IsMatchedToDetector(thisTimeBasedTrack, SYS_TOF) && !detMatches->Get_IsMatchedToDetector(thisTimeBasedTrack, SYS_FCAL)){
	//	cout << "no tracks matched to TOF or FCAL" << "\n";
        continue;
      }
      /*
      // from CDC analysis                                                                                                                                   
	if (thisTimeBasedTrack->FOM < 1E-20){
	  continue;
	}
      */
      // Cut on the reconstructed momentum below 1GeV, no curlers                                                                                            
      /*
	if(thisTimeBasedTrack->pmag() < 1.0){
	  continue;
	}
      */
      japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK                                                                                                     
      htrack_mom_ok->Fill(thisTimeBasedTrack->pmag());
      htrack_chi2_ok->Fill(thisTimeBasedTrack->FOM);
      htrack_theta_ok->Fill(thisTimeBasedTrack->momentum().Theta()*TMath::RadToDeg());
      htrack_phi_ok->Fill(thisTimeBasedTrack->momentum().Phi()*TMath::RadToDeg());
      japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK             
    }

    for( auto cppepem : locCPPEpEms ){
      auto &Ebeam = cppepem->Ebeam;
      auto &weight = cppepem->weight;
      auto &ep_v4 = cppepem->ep_v4;
      auto &em_v4 = cppepem->em_v4;
      auto &ep_FCALShower = cppepem->PositronShower;
      auto &em_FCALShower = cppepem->ElectronShower;
      double ep_FCALEnergy = (ep_FCALShower!=NULL)?ep_FCALShower->getEnergy():0.;
      double em_FCALEnergy = (em_FCALShower!=NULL)?em_FCALShower->getEnergy():0.;
      double FCALElasticity = (ep_FCALEnergy + em_FCALEnergy)/Ebeam;
      double W_2e = (ep_v4 + em_v4).Mag();
      if(W_2e > 0.621){
	continue;
      }
      double ep_v3mag = ep_v4.Vect().Mag(); 
      double em_v3mag = em_v4.Vect().Mag();
      double JTx = ep_v4.X() * 2*em_v4.E()/(ep_v4.E() - ep_v3mag * cos(ep_v4.Theta()))  +
	em_v4.X() * 2*ep_v4.E()/(em_v4.E() - em_v3mag * cos(em_v4.Theta()));

      double JTy = ep_v4.Y() * 2*em_v4.E()/(ep_v4.E() - ep_v3mag * cos(ep_v4.Theta()))  +
	em_v4.Y() * 2*ep_v4.E()/(em_v4.E() - em_v3mag * cos(em_v4.Theta()));

      double Jphi = atan2(JTy, JTx)*180/acos(-1);

      double MLPClassifierMinus=cppepem->pimem_ML_classifier;
      double MLPClassifierPlus=cppepem->pipep_ML_classifier;
      

      if(MLPClassifierPlus < 0.8 || MLPClassifierMinus < 0.8){
	continue;
      }

      japp->RootFillLock(this);
      hpimem_ML_classifier->Fill(MLPClassifierMinus);
      hpipep_ML_classifier->Fill(MLPClassifierPlus);
      japp->RootFillUnLock(this);
     
      japp->RootFillLock(this);
      hphiJT->Fill(Jphi, weight);
      hFCALElasticity->Fill(FCALElasticity, weight);
      hinvmass_ee->Fill(W_2e, weight);
      japp->RootFillUnLock(this);

      
      const DEventWriterEVIO* locEventWriterEVIO = NULL;
      loop->GetSingle(locEventWriterEVIO);
      locEventWriterEVIO->Write_EVIOEvent(loop,"epem_selection");
      ++events_written;
      return NOERROR;
    }
    
    // } //uncomment HERE for skimming modulo N, and the line containing if( eventnumber%100 == 0){ 

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_epem_ml_skim::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_epem_ml_skim::fini(void)
{
	// Called before program exit after event processing is finished.
  jout << "read " << events_read << " events;  written " << events_written << endl;
	return NOERROR;
}


// $Id$
//
//    File: DEventProcessor_BCAL_Shower.cc
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_BCAL_inv_mass.h"

#include <TLorentzVector.h>
#include "TMath.h"

#include "BCAL/DBCALShower.h"
#include "BCAL/DBCALTruthShower.h"
#include "BCAL/DBCALCluster.h"
#include "BCAL/DBCALPoint.h"
#include "BCAL/DBCALHit.h"
#include "FCAL/DFCALCluster.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"

//#include "TRACKING/DTrackFinder.h"
#include "TRIGGER/DTrigger.h"

#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

// Routine used to create our DEventProcessor


static TH1I* bcal_diphoton_mass_300 = NULL;
static TH1I* bcal_diphoton_mass_500 = NULL;
static TH1I* bcal_diphoton_mass_700 = NULL;
static TH1I* bcal_diphoton_mass_900 = NULL;

static TH2I* bcal_diphoton_mass_v_E = NULL;
static TH2I* bcal_diphoton_mass_v_z_lowE = NULL;
static TH2I* bcal_diphoton_mass_v_z_highE = NULL;

static TH1I* bcal_fcal_diphoton_mass_300 = NULL;
static TH1I* bcal_fcal_diphoton_mass_500 = NULL;
static TH1I* bcal_fcal_diphoton_mass_700 = NULL;
static TH1I* bcal_fcal_diphoton_mass_900 = NULL;

extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new JEventProcessor_BCAL_inv_mass()); //register this plugin
	}
} // "C"

//------------------
// Init
//------------------
void JEventProcessor_BCAL_inv_mass::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:

	if(bcal_diphoton_mass_500 != NULL){
	  return;
	}

	TDirectory *main = gDirectory;
	gDirectory->mkdir("bcal_inv_mass")->cd();

        bcal_diphoton_mass_300 = new TH1I("bcal_diphoton_mass_300","bcal diphoton mass (Cluster E > 300 MeV)",100,0.0,1.0);
        bcal_diphoton_mass_300->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_diphoton_mass_300->GetYaxis()->SetTitle("counts / 10 MeV");

	bcal_diphoton_mass_500 = new TH1I("bcal_diphoton_mass_500","bcal diphoton mass (Cluster E > 500 MeV)",100,0.0,1.0);
	bcal_diphoton_mass_500->GetXaxis()->SetTitle("invariant mass [GeV]");
	bcal_diphoton_mass_500->GetYaxis()->SetTitle("counts / 10 MeV");	

        bcal_diphoton_mass_700 = new TH1I("bcal_diphoton_mass_700","bcal diphoton mass (Cluster E > 700 MeV)",100,0.0,1.0);
        bcal_diphoton_mass_700->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_diphoton_mass_700->GetYaxis()->SetTitle("counts / 10 MeV");

        bcal_diphoton_mass_900 = new TH1I("bcal_diphoton_mass_900","bcal diphoton mass (Cluster E > 900 MeV)",100,0.0,1.0);
        bcal_diphoton_mass_900->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_diphoton_mass_900->GetYaxis()->SetTitle("counts / 10 MeV");
	
	bcal_diphoton_mass_v_E = new TH2I("bcal_diphoton_mass_v_E","bcal diphoton mass v E(Both Showers within 100 MeV)",600,0,1.2,100,0,.3);
	bcal_diphoton_mass_v_E->GetXaxis()->SetTitle("shower energy [GeV]");
	bcal_diphoton_mass_v_E->GetYaxis()->SetTitle("2#gamma mass [GeV]");

	bcal_diphoton_mass_v_z_lowE = new TH2I("bcal_diphoton_mass_v_z_lowE","bcal diphoton mass v z(Both showers 300<E<500MeV)",213,0,426,100,0,.3);
	bcal_diphoton_mass_v_z_lowE->GetXaxis()->SetTitle("shower z [cm]");
	bcal_diphoton_mass_v_z_lowE->GetYaxis()->SetTitle("2#gamma mass [GeV]");

	bcal_diphoton_mass_v_z_highE = new TH2I("bcal_diphoton_mass_v_z_highE","bcal diphoton mass v z(Both showers 500<E<700MeV)",213,0,426,100,0,.3);
	bcal_diphoton_mass_v_z_highE->GetXaxis()->SetTitle("shower z [cm]");
	bcal_diphoton_mass_v_z_highE->GetYaxis()->SetTitle("2#gamma mass [GeV]");	

        bcal_fcal_diphoton_mass_300 = new TH1I("bcal_fcal_diphoton_mass_300","bcal and fcal diphoton mass (Cluster E > 300 MeV)",100,0.0,1.0);
        bcal_fcal_diphoton_mass_300->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_fcal_diphoton_mass_300->GetYaxis()->SetTitle("counts / 10 MeV");
	
        bcal_fcal_diphoton_mass_500 = new TH1I("bcal_fcal_diphoton_mass_500","bcal and fcal diphoton mass (Cluster E > 500 MeV)",100,0.0,1.0);
        bcal_fcal_diphoton_mass_500->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_fcal_diphoton_mass_500->GetYaxis()->SetTitle("counts / 10 MeV");

        bcal_fcal_diphoton_mass_700 = new TH1I("bcal_fcal_diphoton_mass_700","bcal and fcal diphoton mass (Cluster E > 700 MeV)",100,0.0,1.0);
        bcal_fcal_diphoton_mass_700->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_fcal_diphoton_mass_700->GetYaxis()->SetTitle("counts / 10 MeV");

        bcal_fcal_diphoton_mass_900 = new TH1I("bcal_fcal_diphoton_mass_900","bcal and fcal diphoton mass (Cluster E > 900 MeV)",100,0.0,1.0);
        bcal_fcal_diphoton_mass_900->GetXaxis()->SetTitle("invariant mass [GeV]");
        bcal_fcal_diphoton_mass_900->GetYaxis()->SetTitle("counts / 10 MeV");

	

	//  ... create historgrams or trees ...

	 //	TDirectory *dir = new TDirectoryFile("BCAL","BCAL");
	 //	dir->cd();

	main->cd();

	return;
}


//------------------
// BeginRun
//------------------
void JEventProcessor_BCAL_inv_mass::BeginRun(const std::shared_ptr<const JEvent>& t)
{
}

//------------------
// Process
//------------------



void JEventProcessor_BCAL_inv_mass::Process(const std::shared_ptr<const JEvent> &locEvent)
{

	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// locEvent->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	//
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// locEvent->Get(mydataclasses);
	//
	// lockService->RootWriteLock();
	//  ... fill historgrams or trees ...
	// lockService->RootUnLock();

	// DOCUMENTATION:
	// ANALYSIS library: https://halldweb1.jlab.org/wiki/index.php/GlueX_Analysis_Software

        const DTrigger* locTrigger = NULL; 
	locEvent->GetSingle(locTrigger); 
	if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
	  return;

    vector<const DTrackFitter *> fitters;
    locEvent->Get(fitters);
    
    if(fitters.size()<1){
      _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
      throw JException("Unable to get a DTrackFinder object!");
    }
	const DTrackFitter *fitter = fitters[0];

	vector<const DBCALShower*> locBCALShowers;
	vector<const DFCALCluster*> locFCALClusters;
	vector<const DVertex*> kinfitVertex;
	//const DDetectorMatches* locDetectorMatches = NULL;
	//locEvent->GetSingle(locDetectorMatches);
	locEvent->Get(locBCALShowers);
	locEvent->Get(locFCALClusters);
	locEvent->Get(kinfitVertex);

	if(locBCALShowers.size() > 15) return;

	vector<const DTrackTimeBased*> locTrackTimeBased;
	locEvent->Get(locTrackTimeBased);

	vector <const DBCALShower*> matchedShowers;
	vector <const DFCALCluster*> matchedFCALClusters;
	vector <const DTrackTimeBased*> matchedTracks;
	DVector3 mypos(0.0,0.0,0.0);

	for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i){
	  vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_BCAL);
	  if (extrapolations.size()>0){
	    for (unsigned int j=0; j< locBCALShowers.size(); ++j){
	
	      double x = locBCALShowers[j]->x;
	      double y = locBCALShowers[j]->y;
	      double z = locBCALShowers[j]->z;
	      DVector3 pos_bcal(x,y,z);
	      double R = pos_bcal.Perp();
	      if (fitter->ExtrapolateToRadius(R,extrapolations,mypos)){
		//double dPhi = TMath::Abs(mypos.Phi()-pos_bcal.Phi());
		double dPhi = mypos.Phi()-pos_bcal.Phi();
		if (dPhi<-M_PI) dPhi+=2.*M_PI;
		if (dPhi>M_PI) dPhi-=2.*M_PI;
		double dZ = TMath::Abs(mypos.Z() - z);
		
		if(dZ < 30.0 && fabs(dPhi) < 0.18 ) {
		  matchedShowers.push_back(locBCALShowers[j]);
		  matchedTracks.push_back(locTrackTimeBased[i]);
		    
		}
	      }
	    }
	  }
	}

   for(unsigned int i = 0 ; i < locTrackTimeBased.size(); ++i)
        {
	  vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_FCAL);
	  if (extrapolations.size()>0){  
	    for(unsigned int j = 0 ; j < locFCALClusters.size(); ++j)
                {
		  const DFCALCluster *c1 = locFCALClusters[j];
		  double x = c1->getCentroid().X();
		  double y = c1->getCentroid().Y();
		  double z = c1->getCentroid().Z();
		  DVector3 fcalpos(x,y,z);
		  //cout << " x = " << x << " y = " << y << endl;
		  DVector3 pos=extrapolations[0].position;
		  
		  double diffX = TMath::Abs(x - pos.X());
		  double diffY = TMath::Abs(y - pos.Y());
		  if(diffX < 3.0 && diffY < 3.0)
		    {
		      matchedFCALClusters.push_back(locFCALClusters[j]);
		    }
		}
	  }
   
	}                        

 	vector <const DChargedTrackHypothesis*> locParticles;
	double kinfitVertexX = 0.0, kinfitVertexY = 0.0, kinfitVertexZ = 0.0;
	for (unsigned int i = 0 ; i < kinfitVertex.size(); i++)
	{
		kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
		kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
		kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
		//p
		//kinfitVertexT = kinfitVertex[i]->dSpacetimeVertex.T();
	}
	
	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

	for(unsigned int i=0; i<locBCALShowers.size(); i++)
	{
	     //   if(locDetectorMatches->Get_IsMatchedToTrack(locBCALShowers[i]))
	  // continue;
		if (find(matchedShowers.begin(), matchedShowers.end(),locBCALShowers[i]) != matchedShowers.end()) continue;
		const DBCALShower *s1 = locBCALShowers[i];
		double dx1 = s1->x - kinfitVertexX;
		double dy1 = s1->y - kinfitVertexY;
		double dz1 = s1->z - kinfitVertexZ;
		double z1 = s1->z;
		double R1 = sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1);
		double  E1 = s1->E;
		double  E1_raw = s1->E_raw;
		TLorentzVector sh1_p(E1*dx1/R1, E1*dy1/R1, E1*dz1/R1, E1);
		TLorentzVector sh1_p_raw(E1_raw*dx1/R1, E1_raw*dy1/R1, E1_raw*dz1/R1, E1_raw);
			for(unsigned int j=i+1; j<locBCALShowers.size(); j++){
				const DBCALShower *s2 = locBCALShowers[j];
		     		if (find(matchedShowers.begin(), matchedShowers.end(),s2) != matchedShowers.end()) continue;
				double dx2 = s2->x - kinfitVertexX;
				double dy2 = s2->y - kinfitVertexY;
				double dz2 = s2->z - kinfitVertexZ; // shift to coordinate relative to center of target
				double z2 = s2->z;
				double z_avg = (z1+z2)/2.;
				double R2 = sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
				double E2 = s2->E;
				double E2_raw = s2->E_raw;
				double E_avg = (E1_raw + E2_raw)/2.;
				TLorentzVector sh2_p(E2*dx2/R2, E2*dy2/R2, E2*dz2/R2, E2);	
				TLorentzVector sh2_p_raw(E2_raw*dx2/R2, E2_raw*dy2/R2, E2_raw*dz2/R2, E2_raw);		
				TLorentzVector ptot = sh1_p+sh2_p;
				TLorentzVector ptot_raw = sh1_p_raw + sh2_p_raw ;
				double inv_mass_raw = ptot_raw.M();
				if(E1_raw>.3&&E2_raw>.3) bcal_diphoton_mass_300->Fill(inv_mass_raw);
				if(E1_raw>.5&&E2_raw>.5) bcal_diphoton_mass_500->Fill(inv_mass_raw);
               	                if(E1_raw>.9&&E2_raw>.9) bcal_diphoton_mass_900->Fill(inv_mass_raw);
               		        if(E1_raw>.7&&E2_raw>.7) bcal_diphoton_mass_700->Fill(inv_mass_raw);
				if(fabs(E1_raw-E2_raw)<.1) bcal_diphoton_mass_v_E->Fill(E_avg,inv_mass_raw);
				if(fabs(z1-z2)<100. && E1_raw>.3 && E2_raw>.3 && E1_raw<.5 && E2_raw<.5) bcal_diphoton_mass_v_z_lowE->Fill(z_avg,inv_mass_raw);
				if(fabs(z1-z2)<100. && E1_raw>.5 && E2_raw>.5 && E1_raw<.7 && E2_raw<.7) bcal_diphoton_mass_v_z_highE->Fill(z_avg,inv_mass_raw); 
			}		
			for(unsigned int j=0; j<locFCALClusters.size(); j++){
				if (find(matchedFCALClusters.begin(), matchedFCALClusters.end(),locFCALClusters[j]) != matchedFCALClusters.end()) continue;
				const DFCALCluster *cl2 = locFCALClusters[j];
				double dx2 = cl2->getCentroid().X()-kinfitVertexX;
	                        double dy2 = cl2->getCentroid().Y()-kinfitVertexY;
                                double dz2 = cl2->getCentroid().Z()-kinfitVertexZ;
				double fcal_E = cl2->getEnergy();
				double R2 = sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
				TLorentzVector cl2_p(fcal_E*dx2/R2, fcal_E*dy2/R2, fcal_E*dz2/R2, fcal_E);
				TLorentzVector ptot_fcal_bcal = sh1_p_raw + cl2_p;
				double inv_mass = ptot_fcal_bcal.M();
                                if(E1_raw>.3&&fcal_E>.3) bcal_fcal_diphoton_mass_300->Fill(inv_mass);
				if(E1_raw>.5&&fcal_E>.5) bcal_fcal_diphoton_mass_500->Fill(inv_mass);
			        if(E1_raw>.7&&fcal_E>.7) bcal_fcal_diphoton_mass_700->Fill(inv_mass);
                                if(E1_raw>.9&&fcal_E>.9) bcal_fcal_diphoton_mass_900->Fill(inv_mass);
			}
	}   


	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK


	/*
	//Optional: Save event to output REST file. Use this to create skims.
	dEventWriterREST->Write_RESTEvent(locEvent, "BCAL_Shower"); //string is part of output file name
	*/
}

//------------------
// EndRun
//------------------
void JEventProcessor_BCAL_inv_mass::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_BCAL_inv_mass::Finish()
{
	// Called before program exit after event processing is finished.
}


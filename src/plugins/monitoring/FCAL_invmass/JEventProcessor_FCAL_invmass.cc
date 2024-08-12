// $Id$
//
//    File: JEventProcessor_FCAL_invmass.cc
// Created: Tue May 31 09:44:35 EDT 2016
// Creator: adesh (on Linux ifarm1101 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_invmass.h"
#include <TLorentzVector.h>
#include "TMath.h"
#include "JANA/JApplication.h"
#include "DANA/DEvent.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"
#include "GlueX.h"
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

static TH1I* InvMass1 = NULL;
static TH1I* InvMass2 = NULL;
static TH1I* InvMass3 = NULL;
static TH1I* InvMass4 = NULL;
static TH1I* InvMass5 = NULL;
static TH1I* InvMass6 = NULL;
static TH1I* InvMass7 = NULL;
static TH1I* InvMass8 = NULL;
static TH1I* InvMass9 = NULL;
static TH2I* hits2D_pi0 = NULL;
static TH1I* qualCut_05 = NULL;
static TH1I* qualCut_03 = NULL;
static TH1I* qualCut_00 = NULL;


extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new JEventProcessor_FCAL_invmass()); //register this plugin
	}
} // "C"

//------------------
// Init
//------------------
void JEventProcessor_FCAL_invmass::Init()
{

	if(InvMass1 && InvMass2 != NULL){
		lockService->RootUnLock();
		return;
	}

	TDirectory *main = gDirectory;
	gDirectory->mkdir("FCAL_invmass")->cd();

	InvMass1 = new TH1I("InvMass1","Shower E. > 500 MeV;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass2 = new TH1I("InvMass2","Shower E. > 1000 MeV;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass3 = new TH1I("InvMass3"," 500 MeV < Shower E. < 900 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass4 = new TH1I("InvMass4"," 900 MeV < Shower E. < 1400 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass5 = new TH1I("InvMass5"," 1400 MeV < Shower E. < 1900 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass6 = new TH1I("InvMass6"," 1900 MeV < Shower E. < 2400 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass7 = new TH1I("InvMass7"," 2400 MeV < Shower E. < 2900 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass8 = new TH1I("InvMass8"," 2900 MeV < Shower E. < 3400 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	InvMass9 = new TH1I("InvMass9"," 3400 MeV < Shower E. < 3900 MeV ;Invariant Mass [GeV]; Counts / 1 MeV ",500,0.0,0.5);

	hits2D_pi0 = new TH2I( "hits2D_pi0", "FCAL Pi0 Shower Hits; X [4 cm] ; Y [4 cm]", 61, -30, 30, 61, -30, 30 );

	qualCut_00 = new TH1I("qualCut_00", "Quality > 0 ; Invariant MAss [GeV]; Counts / 1 MeV", 500, 0.0, 0.5);

	qualCut_03 = new TH1I("qualCut_03", "Quality > 0.3 ; Invariant MAss [GeV]; Counts / 1 MeV", 500, 0.0, 0.5);

	qualCut_05 = new TH1I("qualCut_05", "Quality > 0.5 ; Invariant MAss [GeV]; Counts / 1 MeV", 500, 0.0, 0.5);

	main->cd();

}



//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_invmass::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	double z_target; double z_fcal;
	const DGeometry* dgeom = GetDGeometry(event);

	dgeom->GetTargetZ(z_target);
	dgeom->GetFCALZ(z_fcal);

	z_diff = z_fcal - z_target;
}

void JEventProcessor_FCAL_invmass::Process(const std::shared_ptr<const JEvent> &event)
{
	map< const DFCALShower*, double > showerQualityMap;
	vector< const DNeutralShower* > neutralShowers;	
	vector< const DFCALHit*> hits;

	vector< const DFCALShower* > locFCALShowers;
	vector< const DVertex* > kinfitVertex;
	vector< const DTrackTimeBased* > locTrackTimeBased;

	event->Get( hits );

	if( hits.size() <= 500 ){ // only form clusters and showers if there aren't too many hits

		event->Get(locFCALShowers);
		event->Get(kinfitVertex);
		event->Get(locTrackTimeBased);
		event->Get( neutralShowers );
	}

	for( size_t i = 0; i < neutralShowers.size(); ++i ){

		if( neutralShowers[i]->dDetectorSystem == SYS_FCAL ){

			const DFCALShower* fcalShower = dynamic_cast< const DFCALShower* >( neutralShowers[i]->dBCALFCALShower );
			showerQualityMap[fcalShower] = neutralShowers[i]->dQuality;
		}
	}


	vector < const DFCALShower * > matchedShowers;
	vector < const DTrackTimeBased* > matchedTracks;
	vector <const DChargedTrackHypothesis*> locParticles;


	Double_t kinfitVertexX = 0.0, kinfitVertexY = 0.0, kinfitVertexZ = 0.0;

	for (unsigned int i = 0 ; i < kinfitVertex.size(); i++)
	{
		kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
		kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
		kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
	}

	DVector3 norm(0.0,0.0,-1);
	DVector3 pos,mom;

	for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i){
		if(locTrackTimeBased[i]->extrapolations.size() < SYS_FCAL ) continue;
		vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_FCAL);
		if (extrapolations.size()>0){
			for (unsigned int j=0; j< locFCALShowers.size(); ++j){

				Double_t x = locFCALShowers[j]->getPosition().X();
				Double_t y = locFCALShowers[j]->getPosition().Y();
				Double_t z = locFCALShowers[j]->getPosition().Z();

				DVector3 pos_FCAL(x,y,z);
				pos=extrapolations[0].position;
				Double_t trkmass = locTrackTimeBased[i]->mass();
				Double_t FOM = TMath::Prob(locTrackTimeBased[i]->chisq, locTrackTimeBased[i]->Ndof);
				Double_t dRho = sqrt(((pos.X() - x)*(pos.X() - x)) + ((pos.Y() - y)* (pos.Y() - y)));

				if(trkmass < 0.15 && dRho < 5 && FOM > 0.01 ) {  
					matchedShowers.push_back(locFCALShowers[j]);
					matchedTracks.push_back(locTrackTimeBased[i]);

				}
			}

		}
	}

	// lockService->RootWriteLock();
	lockService->RootWriteLock(); 
	if (locFCALShowers.size() >=2) {

		for(unsigned int i=0; i<locFCALShowers.size(); i++)
		{
			if (find(matchedShowers.begin(), matchedShowers.end(),locFCALShowers[i]) != matchedShowers.end()) continue;

			const DFCALShower *s1 = locFCALShowers[i];

			vector<const DFCALCluster*> associated_clusters1;

			s1->Get(associated_clusters1);
			Double_t dx1 = s1->getPosition().X() - kinfitVertexX;
			Double_t dy1 = s1->getPosition().Y() - kinfitVertexY;
			Double_t dz1 = s1->getPosition().Z() - kinfitVertexZ;

			Double_t R1 = sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1);
			Double_t  E1 = s1->getEnergy();
			Double_t  t1 = s1->getTime();
			TLorentzVector sh1_p(E1*dx1/R1, E1*dy1/R1, E1*dz1/R1, E1);

			for(unsigned int j=i+1; j<locFCALShowers.size(); j++){
				
				const DFCALShower *s2 = locFCALShowers[j];
				if (find(matchedShowers.begin(), matchedShowers.end(),s2) != matchedShowers.end()) continue;

				vector<const DFCALCluster*> associated_clusters2;
				s2->Get(associated_clusters2);
				Double_t dx2 = s2->getPosition().X() - kinfitVertexX;
				Double_t dy2 = s2->getPosition().Y() - kinfitVertexY;
				Double_t dz2 = s2->getPosition().Z() - kinfitVertexZ;

				
				qualH = showerQualityMap[s2];
				qualL = showerQualityMap[s1];				


				Double_t R2 = sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
				Double_t E2 = s2->getEnergy();
				Double_t  t2 = s2->getTime();

				TLorentzVector sh2_p(E2*dx2/R2, E2*dy2/R2, E2*dz2/R2, E2);
				TLorentzVector ptot = sh1_p+sh2_p;
				Double_t inv_mass = ptot.M();

				
				if( qualH > 0 && qualL > 0 )
					qualCut_00->Fill(inv_mass);

				if( qualH > 0.3 && qualL > 0.3 )
					qualCut_03->Fill(inv_mass);

				if( qualH > 0.5 && qualL > 0.5 )
					qualCut_05->Fill(inv_mass);


				if(E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) ) InvMass1->Fill(inv_mass);
				if(E1 > 1.0 && E2 > 1.0 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass2->Fill(inv_mass);

				if((E1 > 0.5 && E1 < 0.9) && (E2 > 0.5 && E2 < 0.9)   && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass3->Fill(inv_mass);

				if((E1 > 0.9 && E1 < 1.4) && (E2 > 0.9 && E2 < 1.4) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass4->Fill(inv_mass);

				if((E1 > 1.4 && E1 < 1.9) && (E2 > 1.4 && E2 < 1.9) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass5->Fill(inv_mass);

				if((E1 > 1.9 && E1 < 2.4) && (E2 > 1.9 && E2 < 2.4) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass6->Fill(inv_mass);

				if((E1 > 2.4 && E1 < 2.9) && (E2 > 2.4 && E2 < 2.9) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass7->Fill(inv_mass);

				if((E1 > 2.9 && E1 < 3.4) && (E2 > 2.9 && E2 < 3.4) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass8->Fill(inv_mass);

				if((E1 > 3.4 && E1 < 3.9) && (E2 > 3.4 && E2 < 3.9) && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)) InvMass9->Fill(inv_mass);


				for(unsigned int loc_j = 0; loc_j < associated_clusters1.size(); loc_j++)
				{
					for(unsigned int loc_jj = 0; loc_jj < associated_clusters2.size(); loc_jj++)
					{

						vector< DFCALCluster::DFCALClusterHit_t > hits1 = associated_clusters1[loc_j]->GetHits();
						vector< DFCALCluster::DFCALClusterHit_t > hits2 = associated_clusters2[loc_jj]->GetHits();

						Int_t numhits_per_cluster1 = associated_clusters1[loc_j]->GetNHits();
						Int_t numhits_per_cluster2 = associated_clusters2[loc_jj]->GetNHits();


						// Get hits per block for pi0 candidate shower

						for(  int i = 0; i < numhits_per_cluster1; ++i ){
							int my_x = hits1[i].x ; 
							int my_y = hits1[i].y ;

							if (inv_mass > 0.11 && inv_mass < 0.16 && E1 > 1 && E2 > 1 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)){
								hits2D_pi0->Fill(my_x/4,my_y/4,1);
							}
						}

						for(  int i = 0; i < numhits_per_cluster2; ++i ){
							int my_x = hits2[i].x ; 
							int my_y = hits2[i].y ;

							if (inv_mass > 0.11 && inv_mass < 0.16 && E1 > 1 && E2 > 1 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm  && (fabs (t1-t2) < 10)){
								hits2D_pi0->Fill(my_x/4,my_y/4,1);
							}
						}
					}
				}
			}
		}
	}
	//lockService->RootUnLock();

	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_invmass::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}



//------------------
// Finish
//------------------
void JEventProcessor_FCAL_invmass::Finish()
{
	// Called before program exit after event processing is finished.
}



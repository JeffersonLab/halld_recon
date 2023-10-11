// $Id$
//
//    File: JEventProcessor_twogamma_fcal_skim.cc
// Created: Sat Nov 30 16:32:06 EST 2019
// Creator: cakondi (on Linux ifarm1801 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_twogamma_fcal_skim.h"


// Routine used to create our JEventProcessor
#include <TLorentzVector.h>
#include "TMath.h"
#include "DANA/DEvent.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"
#include "PID/DEventRFBunch.h"

#include "GlueX.h"
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_twogamma_fcal_skim());
}
} // "C"


//------------------
// JEventProcessor_twogamma_fcal_skim (Constructor)
//------------------
JEventProcessor_twogamma_fcal_skim::JEventProcessor_twogamma_fcal_skim()
{
	SetTypeName("JEventProcessor_twogamma_fcal_skim");

}

//------------------
// ~JEventProcessor_twogamma_fcal_skim (Destructor)
//------------------
JEventProcessor_twogamma_fcal_skim::~JEventProcessor_twogamma_fcal_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_twogamma_fcal_skim::Init()
{
	// This is called once at program startup. 

  num_epics_events = 0;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_twogamma_fcal_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_twogamma_fcal_skim::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// event->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// event->Get(mydataclasses);
	//
	// lockService->RootWriteLock();
	//  ... fill historgrams or trees ...
	// lockService->RootUnLock();



vector< const DFCALShower* > locFCALShowers;
  vector< const DVertex* > kinfitVertex;
  event->Get(locFCALShowers);
  event->Get(kinfitVertex);

  vector< const DTrackTimeBased* > locTrackTimeBased;
  event->Get(locTrackTimeBased);

  vector < const DFCALShower * > matchedShowers;

	const DEventWriterEVIO* locEventWriterEVIO = NULL;
	event->GetSingle(locEventWriterEVIO);

  // always write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "twogamma_fcal_skim" );
      return;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "twogamma_fcal_skim" );
      num_epics_events++;
      return;
  }

  vector< const JObject* > locObjectsToSave;  

  bool Candidate = false;
  
  Double_t kinfitVertexX = 0.0, kinfitVertexY = 0.0, kinfitVertexZ = 0.0;
  
  for (unsigned int i = 0 ; i < kinfitVertex.size(); i++)
    {
      if(i==0)
          locObjectsToSave.push_back(static_cast<const JObject *>(kinfitVertex[0]));

      kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
      kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
      kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
    }

    vector<const DEventRFBunch*> locEventRFBunches;
    event->Get(locEventRFBunches);
    if(locEventRFBunches.size() > 0) {
        locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches[0]));
    }

  DVector3 norm(0.0,0.0,-1);
  DVector3 pos,mom;
 // Double_t radius = 0;
  //lockService->RootWriteLock();
  //Double_t p;
  for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i){
    vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_FCAL);
    if (extrapolations.size()==0) continue;

    for (unsigned int j=0; j< locFCALShowers.size(); ++j){

      Double_t x = locFCALShowers[j]->getPosition().X();
      Double_t y = locFCALShowers[j]->getPosition().Y();
   //    Double_t z = locFCALShowers[j]->getPosition().Z() ;
 //cout << "Z: " << z << endl;
      //DVector3 pos_FCAL(x,y,625.406);
      //for LH2 target
      //DVector3 pos_FCAL(0,0,625.406);
      
      DVector3 pos_FCAL(0,0,638);
      //at the end of the start counter; use this fall for fall '15 data
      // DVector3 pos_FCAL(0,0,692);
         //DVector3 pos_FCAL(0.0,0.0,650);
      //std::cout<<"i: "<< i<< " j: "<< j << " z: "<<z<< endl;
      // if (locTrackTimeBased[i]->rt->GetIntersectionWithPlane(pos_FCAL,norm,pos,mom)==NOERROR)
      pos=extrapolations[0].position;

      // Double_t dX= TMath::Abs(pos.X() - x);
      // Double_t dY= TMath::Abs(pos.Y() - y);
      // Double_t dZ= TMath::Abs(pos.Z() - z);
      Double_t trkmass = locTrackTimeBased[i]->mass();
      Double_t FOM = TMath::Prob(locTrackTimeBased[i]->chisq, locTrackTimeBased[i]->Ndof);
      // radius = sqrt(pos.X()*pos.X() + pos.Y()*pos.Y());
      //  Double_t Eshwr = locFCALShowers[j]->getEnergy();
      //  p = locTrackTimeBased[i]->momentum().Mag();
      // cout<<"p: "<<p<<endl;
      // Double_t dZ = TMath::Abs(pos.Z() - z);
      Double_t dRho = sqrt(((pos.X() - x)*(pos.X() - x)) + ((pos.Y() - y)* (pos.Y() - y)));
      // std::cout<<"i: "<< i<< " j: "<< j << " dRho " <<dRho << endl;
      //if(dX < 20 && dY < 20 && trkmass < 0.15 && dRho < 50 && FOM > 0.01) {  
      if(trkmass < 0.15 && dRho < 5 && FOM > 0.01 ) {  
	matchedShowers.push_back(locFCALShowers[j]);
	// matchedTracks.push_back(locTrackTimeBased[i]);
	//  printf ("Matched event=%d, i=%d, j=%d, p=%f, Ztrk=%f Zshr=%f, Xtrk=%f, Xshr=%f, Ytrk=%f, Yshr=%f\n",locEventNumber,i,j,p,
	//  pos.Z(),z,pos.X(),x,pos.Y(),y);
	//  break;
	
      }

    }
  }
   
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
			
      for(unsigned int j=i+1; j<locFCALShowers.size(); j++)
      {
	const DFCALShower *s2 = locFCALShowers[j];
	if (find(matchedShowers.begin(), matchedShowers.end(),s2) != matchedShowers.end()) continue;
	
	vector<const DFCALCluster*> associated_clusters2;
	s2->Get(associated_clusters2);
	Double_t dx2 = s2->getPosition().X() - kinfitVertexX;
	Double_t dy2 = s2->getPosition().Y() - kinfitVertexY;
	Double_t dz2 = s2->getPosition().Z() - kinfitVertexZ; 
	Double_t R2 = sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
	Double_t E2 = s2->getEnergy();
	Double_t  t2 = s2->getTime();
	
	TLorentzVector sh2_p(E2*dx2/R2, E2*dy2/R2, E2*dz2/R2, E2);
	TLorentzVector ptot = sh1_p+sh2_p;
	Double_t inv_mass = ptot.M();

    //Candidate |= (E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) && (inv_mass<0.30) ) ;
        Candidate |= (E1 > 0.5 && E2 > 0.5 && (fabs (t1-t2) < 10) && (inv_mass<1.0) ) ;

        //if(E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) && (inv_mass<0.30) ) {
        if(E1 > 0.5 && E2 > 0.5 && (fabs (t1-t2) < 10) && (inv_mass<1.0) ) {
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[i]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[i]));
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[j]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[j]));
        }
 			}
 	}		
 
 if( Candidate ){

    if( WRITE_EVIO ){

        locEventWriterEVIO->Write_EVIOEvent( event, "twogamma_fcal_skim", locObjectsToSave );
    }
 }
 




}

//------------------
// EndRun
//------------------
void JEventProcessor_twogamma_fcal_skim::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_twogamma_fcal_skim::Finish()
{
	// Called before program exit after event processing is finished.
}


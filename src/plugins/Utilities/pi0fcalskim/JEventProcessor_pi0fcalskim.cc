// $Id$
//
//    File: JEventProcessor_pi0fcalskim.cc
// Created: Mon Dec  1 14:57:11 EST 2014
// Creator: shepherd (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include <math.h>

#include "JEventProcessor_pi0fcalskim.h"

// Routine used to create our JEventProcessor
#include <TLorentzVector.h>
#include "TMath.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"
#include "PID/DEventRFBunch.h"
#include "HDDM/DEventWriterHDDM.h"
#include "TOF/DTOFPoint.h"

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
    app->Add(new JEventProcessor_pi0fcalskim());
  }
} // "C"


//------------------
// JEventProcessor_pi0fcalskim (Constructor)
//------------------
JEventProcessor_pi0fcalskim::JEventProcessor_pi0fcalskim()
{

  SetTypeName("JEventProcessor_pi0fcalskim");
}

//------------------
// ~JEventProcessor_pi0fcalskim (Destructor)
//------------------
JEventProcessor_pi0fcalskim::~JEventProcessor_pi0fcalskim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_pi0fcalskim::Init()
{
	auto app = GetApplication();

	WRITE_EVIO = 1;
	WRITE_HDDM = 0;

	app->SetDefaultParameter( "PI0FCALSKIM:WRITE_EVIO", WRITE_EVIO, "Save skim output as reduced EVIO files (default=1)" );
	app->SetDefaultParameter( "PI0FCALSKIM:WRITE_HDDM", WRITE_HDDM, "Save skim output as HDDM (default=0)" );



	MIN_MASS   = 0.0; // GeV
	//MAX_MASS   = 0.30; // GeV - old default
	MAX_MASS   = 1.0; // GeV
	MIN_E      = 0.5; // GeV (photon energy cut)
	//MIN_R      =   20; // cm  (cluster distance to beam line) - not currently used
	MAX_DT     =   10; // ns  (cluster time diff. cut)
	//MAX_ETOT   =   12; // GeV (max total FCAL energy) - not currently used
	//MIN_BLOCKS =    2; // minumum blocks per cluster - not currently used

	SAVE_TOF_HITS = 1;

	WRITE_ROOT = 0;
	WRITE_EVIO = 1;

	app->SetDefaultParameter( "PI0FCALSKIM:MIN_MASS", MIN_MASS, "Minimum two-gamma invariant mass to save (default=0.0 GeV)" );
	app->SetDefaultParameter( "PI0FCALSKIM:MAX_MASS", MAX_MASS, "Maximum two-gamma invariant mass to save (default=1.0 GeV)" );
	app->SetDefaultParameter( "PI0FCALSKIM:MIN_E", MIN_E, "Minimum photon energy to consider (default = 0.5 GeV)" );
	//app->SetDefaultParameter( "PI0FCALSKIM:MIN_R", MIN_R );
	app->SetDefaultParameter( "PI0FCALSKIM:MAX_DT", MAX_DT, "Minimum Delta T cut between two photons (default = 10 ns)" );
	//app->SetDefaultParameter( "PI0FCALSKIM:MAX_ETOT", MAX_ETOT );
	//app->SetDefaultParameter( "PI0FCALSKIM:MIN_BLOCKS", MIN_BLOCKS );
	//app->SetDefaultParameter( "PI0FCALSKIM:WRITE_ROOT", WRITE_ROOT );
	app->SetDefaultParameter( "PI0FCALSKIM:SAVE_TOF_HITS", MAX_DT, "Save hits from TOF points (default=1)" );

	num_epics_events = 0;
/*
  if( ! ( WRITE_ROOT || WRITE_EVIO ) ){

    cerr << "No output mechanism has been specified." << endl;
    return UNRECOVERABLE_ERROR;
  }

  if( WRITE_ROOT ){

    lockService->RootWriteLock();

    m_tree = new TTree( "cluster", "Cluster Tree for Pi0 Calibration" );
    m_tree->Branch( "nClus", &m_nClus, "nClus/I" );
    m_tree->Branch( "hit0", m_hit0, "hit0[nClus]/I" );
    m_tree->Branch( "px", m_px, "px[nClus]/F" );
    m_tree->Branch( "py", m_py, "py[nClus]/F" );
    m_tree->Branch( "pz", m_pz, "pz[nClus]/F" );

    m_tree->Branch( "nHit", &m_nHit, "nHit/I" );
    m_tree->Branch( "chan", m_chan, "chan[nHit]/I" );
    m_tree->Branch( "e", m_e, "e[nHit]/F" );

    lockService->RootUnLock();
  }
*/
}

//------------------
// BeginRun
//------------------
void JEventProcessor_pi0fcalskim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_pi0fcalskim::Process(const std::shared_ptr<const JEvent>& event)
{
 
  vector< const DFCALShower* > locFCALShowers;
  vector< const DTOFPoint* > locTOFPoints;
  vector< const DVertex* > kinfitVertex;
  event->Get(locFCALShowers);
  event->Get(locTOFPoints);
  event->Get(kinfitVertex);

  vector< const DTrackTimeBased* > locTrackTimeBased;
  event->Get(locTrackTimeBased);

  vector < const DFCALShower * > matchedShowers;

	const DEventWriterEVIO* locEventWriterEVIO = NULL;
	event->GetSingle(locEventWriterEVIO);

  // always write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "pi0fcalskim" );
      return;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "pi0fcalskim" );
      num_epics_events++;
      return;
  }

  vector< const JObject* > locObjectsToSave;  

  // save TOF points to help with background rejection, especially near the beam hole
  if(SAVE_TOF_HITS) {
	  for (unsigned int i = 0 ; i < locTOFPoints.size(); i++) {
		locObjectsToSave.push_back(static_cast<const JObject *>(locTOFPoints[i]));
	  }
  }

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
      
      DVector3 pos_FCAL(0,0,638);   //TOFIX: shouldn't this come from the geometry??
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
        Candidate |= ( (E1 > MIN_E) && (E2 > MIN_E) && (fabs(t1-t2) < MAX_DT) && (inv_mass>MIN_MASS) && (inv_mass<MAX_MASS) );

        //if(E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) && (inv_mass<0.30) ) {
        if( (E1 > MIN_E) && (E2 > MIN_E) && (fabs(t1-t2) < MAX_DT) && (inv_mass>MIN_MASS) && (inv_mass<MAX_MASS) ) {
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[i]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[i]));
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[j]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[j]));
        }
 	  }
 	}		
 
 if( Candidate ){

    if( WRITE_EVIO ){
        locEventWriterEVIO->Write_EVIOEvent( event, "pi0fcalskim", locObjectsToSave );
    }
    if( WRITE_HDDM ) {
      vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
      event->Get(locEventWriterHDDMVector);
      locEventWriterHDDMVector[0]->Write_HDDMEvent(event, "");
    }

 }
 
 
 /*
  vector< const DFCALCluster* > clusterVec;
  event->Get( clusterVec );

  if( clusterVec.size() < 2 ) return;

  bool hasCandidate = false;
  double eTot = 0;

  for( vector< const DFCALCluster*>::const_iterator clus1Itr = clusterVec.begin();
       clus1Itr != clusterVec.end(); ++clus1Itr ){

    eTot += (**clus1Itr).getEnergy();

    for( vector< const DFCALCluster*>::const_iterator clus2Itr = clus1Itr + 1;
	 clus2Itr != clusterVec.end(); ++clus2Itr ){

      const DFCALCluster& clusL = 
	( (**clus1Itr).getEnergy() > (**clus2Itr).getEnergy() ? 
	  (**clus2Itr) : (**clus1Itr) );

      const DFCALCluster& clusH = 
	( (**clus1Itr).getEnergy() > (**clus2Itr).getEnergy() ? 
	  (**clus1Itr) : (**clus2Itr) );

      double clusLX = clusL.getCentroid().X();
      double clusLY = clusL.getCentroid().Y();
      double rL = sqrt( clusLX * clusLX + clusLY * clusLY );
      double eL = clusL.getEnergy();
      double tL = clusL.getTime();
      int nHitL = clusL.GetHits().size();

      double clusHX = clusH.getCentroid().X();
      double clusHY = clusH.getCentroid().Y();
      double rH = sqrt( clusHX * clusHX + clusHY * clusHY );
      double eH = clusH.getEnergy();
      double tH = clusH.getTime();
      int nHitH = clusH.GetHits().size();

      DVector3 clusLMom = clusL.getCentroid(); 
      clusLMom.SetMag( eL );

      DVector3 clusHMom = clusH.getCentroid(); 
      clusHMom.SetMag( eH );
   √ 
      double dt = fabs( tL - tH );

      DLorentzVector gamL( clusLMom, clusLMom.Mag() );
      DLorentzVector gamH( clusHMom, clusHMom.Mag() );

      double mass = ( gamL + gamH ).M();

  √    hasCandidate |= 
	( ( eL > MIN_E ) &&
	  ( dt < MAX_DT ) &&
	  ( rL > MIN_R ) && ( rH > MIN_R ) &&
√√	  ( nHitL >= MIN_BLOCKS ) && ( nHitH >= MIN_BLOCKS ) &&
	  ( mass > MIN_MASS ) && ( mass < MAX_MASS  ) );
    }
  }

  if( hasCandidate && ( eTot < MAX_ETOT ) ){

    if( WRITE_EVIO ){

      dEventWriterEVIO->Write_EVIOEvent( loop, "pi0fcalskim" );
    }

    if( WRITE_ROOT ){

      lockService->RootWriteLock();
      writeClustersToRoot( clusterVec );
      lockService->RootUnLock();
    }
  }
*/
  return;
}

//------------------
// EndRun
//------------------
void JEventProcessor_pi0fcalskim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return;
}

//------------------
// Fin
//------------------
void JEventProcessor_pi0fcalskim::Finish()
{
  // Called before program exit after event processing is finished.
  return;
}


/*
void 
JEventProcessor_pi0fcalskim::writeClustersToRoot( const vector< const DFCALCluster* > clusVec ){

  // this code must run serially -- obtain a lock before
  // entering this function
 
  m_nHit = 0;
  m_nClus = 0;

  // hit and cluster indices
  int& iH = m_nHit;
  int& iC = m_nClus;
  
  for( vector< const DFCALCluster* >::const_iterator clusItr = clusVec.begin();
       clusItr != clusVec.end(); ++clusItr ){

    // if we exceed max clusters abort writing for this event
    if( iC == kMaxClus ) return;

    const DFCALCluster& clus = (**clusItr);

    if( ( clus.getCentroid().Perp() < 20*k_cm ) ||
	( clus.getEnergy() < 1*k_GeV ) ||
	( clus.GetHits().size() < 2 ) ) continue;

    DVector3 gamMom = clus.getCentroid(); 
    gamMom.SetMag( clus.getEnergy() );

    m_hit0[iC] = iH;
    m_px[iC] = gamMom.X();
    m_py[iC] = gamMom.Y();
    m_pz[iC] = gamMom.Z();
    
    const vector<DFCALCluster::DFCALClusterHit_t>& hits = clus.GetHits();

    for( unsigned int i = 0; i < hits.size(); ++i ){

      // if we exceed max hits abort this event and return
      if( iH == kMaxHits ) return;

      m_chan[iH] = hits[i].ch;
      m_e[iH] = hits[i].E;
      ++iH;
    }
    ++iC;
  }

  m_tree->Fill();
}
*/

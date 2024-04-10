/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2020       GlueX and PrimEX-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include <math.h>

#include "JEventProcessor_eta2g_primexd_skim.h"

#include <TLorentzVector.h>
#include "TMath.h"
#include "FCAL/DFCALShower.h"
#include "BCAL/DBCALShower.h"
#include "CCAL/DCCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "START_COUNTER/DSCHit.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "PID/DVertex.h"
#include "PID/DEventRFBunch.h"
#include "HDDM/DEventWriterHDDM.h"
#include <HDGEOMETRY/DGeometry.h>
#include <TOF/DTOFPoint.h>

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
    app->Add(new JEventProcessor_eta2g_primexd_skim());
  }
} // "C"


//------------------
// JEventProcessor_eta2g_primexd_skim (Constructor)
//------------------
JEventProcessor_eta2g_primexd_skim::JEventProcessor_eta2g_primexd_skim()
{
  SetTypeName("JEventProcessor_eta2g_primexd_skim");
}

//------------------
// ~JEventProcessor_eta2g_primexd_skim (Destructor)
//------------------
JEventProcessor_eta2g_primexd_skim::~JEventProcessor_eta2g_primexd_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_eta2g_primexd_skim::Init()
{
  auto app = GetApplication();
  // lockService = app->GetService<JLockService>();

  WRITE_EVIO = 1;
  WRITE_HDDM = 0;

  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:WRITE_EVIO", WRITE_EVIO );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:WRITE_HDDM", WRITE_HDDM );

/*
  MIN_MASS   = 0.03; // GeV
  MAX_MASS   = 0.30; // GeV
  MIN_E      =  1.0; // GeV (photon energy cut)
  MIN_R      =   20; // cm  (cluster distance to beam line)
  MAX_DT     =   10; // ns  (cluster time diff. cut)
  MAX_ETOT   =   12; // GeV (max total FCAL energy)
  MIN_BLOCKS =    2; // minumum blocks per cluster

  WRITE_ROOT = 0;
  WRITE_EVIO = 1;

  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MIN_MASS", MIN_MASS );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MAX_MASS", MAX_MASS );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MIN_E", MIN_E );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MIN_R", MIN_R );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MAX_DT", MAX_DT );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MAX_ETOT", MAX_ETOT );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:MIN_BLOCKS", MIN_BLOCKS );
  app->SetDefaultParameter( "ETA2G_PRIMEXD_SKIM:WRITE_ROOT", WRITE_ROOT );
  */


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
void JEventProcessor_eta2g_primexd_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  DGeometry* dgeom = GetDGeometry(event);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    return; // RESOURCE_UNAVAILABLE;
  }	
  JCalibration *jcalib = GetJCalibration(event);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");

}

//------------------
// Process
//------------------
void JEventProcessor_eta2g_primexd_skim::Process(const std::shared_ptr<const JEvent>& event)
{
  /*
  const DL1Trigger *trig = NULL;
  try {
    eventLoop->GetSingle(trig);
  } catch (...) {
  }
  if (trig == NULL) { return NOERROR; }
  
  uint32_t trigmask = trig->trig_mask;
  uint32_t fp_trigmask = trig->fp_trig_mask;
  */
 
  vector< const DFCALShower* > locFCALShowers;
  vector< const DBCALShower* > locBCALShowers;
  vector< const DCCALShower* > locCCALShowers;
  vector< const DVertex* > kinfitVertex;
  vector<const DTOFPoint*> tof_points;
  vector< const DBeamPhoton* > locBeamPhotons;
  vector< const DSCHit* > locSCHit;

  event->Get(locFCALShowers);
  event->Get(locBCALShowers);
  event->Get(locCCALShowers);
  event->Get(kinfitVertex);
  event->Get(tof_points);
  event->Get(locBeamPhotons);
  event->Get(locSCHit);
  
  vector<const DTrackCandidate*> locTrackCandidate;
  event->Get(locTrackCandidate);

  vector< const DTrackTimeBased* > locTrackTimeBased;
  event->Get(locTrackTimeBased);
  
  vector < const DFCALShower * > matchedShowers;
  
  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);

  vector<const DFCALHit *> locFCALHits;
  
  event->Get(locFCALHits);

  // always write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "eta2g-skim" );
      return;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "eta2g-skim" );
      num_epics_events++;
      return;
  }

  
  ////vector< const JObject* > locObjectsToSave;  

  bool Candidate = false;
  
  DVector3 vertex;
  vertex.SetXYZ(m_beamSpotX, m_beamSpotY, m_targetZ);
  /*
  for (unsigned int i = 0 ; i < tof_points.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(tof_points[i]));
  }
  for (unsigned int i = 0 ; i < locBCALShowers.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locBCALShowers[i]));
  }
  for (unsigned int i = 0 ; i < locCCALShowers.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locCCALShowers[i]));
  }
  for (unsigned int i = 0 ; i < locTrackCandidate.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locTrackCandidate[i]));
  }
  for (unsigned int i = 0 ; i < locTrackTimeBased.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locTrackTimeBased[i]));
  }
  for (unsigned int i = 0 ; i < locBeamPhotons.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locBeamPhotons[i]));
  }
  for (unsigned int i = 0 ; i < locSCHit.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locSCHit[i]));
  }
  //Use kinfit when available
  double kinfitVertexX = m_beamSpotX;
  double kinfitVertexY = m_beamSpotY;
  double kinfitVertexZ = m_targetZ;
  //DVector3 vertex(m_beamSpotX, m_beamSpotY, m_targetZ);
  for (unsigned int i = 0 ; i < kinfitVertex.size(); i++) {
    if(i==0)
      locObjectsToSave.push_back(static_cast<const JObject *>(kinfitVertex[0]));
  }
  
  vector<const DEventRFBunch*> locEventRFBunches;
  event->Get(locEventRFBunches);
  if(locEventRFBunches.size() > 0) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches[0]));
  }
  */

  double FCAL_trg_Esum = 0;
  
  for (vector<const DFCALHit*>::const_iterator hit  = locFCALHits.begin(); hit != locFCALHits.end(); hit++ ) {
    if ((**hit).E > 0.150)
      FCAL_trg_Esum += (**hit).E;
  }
  
  DVector3 norm(0.0,0.0,-1);
  DVector3 pos,mom;
  Double_t BCAL_energy_sum = 0;
  Double_t CCAL_energy_sum = 0;
  //Double_t FCAL_energy_sum = 0;
  
  for (unsigned int i=0; i<locBCALShowers.size(); i++) {
    Double_t E = locBCALShowers[i]->E;
    BCAL_energy_sum += E;
  }

  for (unsigned int i=0; i<locCCALShowers.size(); i++) {
    Double_t E = locCCALShowers[i]->E;
    CCAL_energy_sum += E;
  }
  
  for (unsigned int i = 0;  i < locFCALShowers.size(); i ++) {
    double e1 = locFCALShowers[i]->getEnergy();
    DVector3 position1 = locFCALShowers[i]->getPosition_log() - vertex;
    double r1 = position1.Mag();
    double t1 = locFCALShowers[i]->getTime() - (r1 / TMath::C() * 1e7);
    double p1x = e1 * sin(position1.Theta()) * cos(position1.Phi());
    double p1y = e1 * sin(position1.Theta()) * sin(position1.Phi());
    double p1z = e1 * cos(position1.Theta());
    TLorentzVector PhotonVec1(p1x, p1y, p1z, e1);
    /*
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
    */
    
    for (unsigned int j = i + 1; j < locFCALShowers.size(); j ++) {
      double e2 = locFCALShowers[j]->getEnergy();
      DVector3 position2 = locFCALShowers[j]->getPosition_log() - vertex;
      double r2 = position2.Mag();
      double t2 = locFCALShowers[j]->getTime() - (r2 / TMath::C() * 1e7);
      double p2x = e2 * sin(position2.Theta()) * cos(position2.Phi());
      double p2y = e2 * sin(position2.Theta()) * sin(position2.Phi());
      double p2z = e2 * cos(position2.Theta());
      TLorentzVector PhotonVec2(p2x, p2y, p2z, e2);
      /*
      const DFCALShower *s2 = locFCALShowers[j];
            
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
      */
      Double_t inv_mass = (PhotonVec1 + PhotonVec2).M();
      
      //Candidate |= (E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) && (inv_mass<0.30) ) ;
      //Candidate |= (E1 > 0.1 && E2 > 0.1 && (fabs (t1-t2) < 15) && (inv_mass>0.380) /*&& BCAL_energy_sum < 0.1 && CCAL_energy_sum < 0.1*/) ;
      //Candidate |= (e1 > 0.1 && e2 > 0.1 && (fabs (t1-t2) < 15) && (inv_mass>0.380) /*&& BCAL_energy_sum < 0.1 && CCAL_energy_sum < 0.1*/) ;
      
      //if(E1 > 0.5 && E2 > 0.5 && s1->getPosition().Pt() > 20*k_cm && s2->getPosition().Pt() > 20*k_cm && (fabs (t1-t2) < 10) && (inv_mass<0.30) ) {
      if(e1 > 0.1 && e2 > 0.1 && (fabs (t1-t2) < 25) && (inv_mass>0.330) ) {
	/*
	  if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[i]) == locObjectsToSave.end())
	  locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[i]));
	  if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locFCALShowers[j]) == locObjectsToSave.end())
	  locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[j]));
	*/
	Candidate = true;
      }
    }
  }		
  
  if( Candidate /*&& FCAL_trg_Esum > 5.0*/){
    
    if( WRITE_EVIO ){
      //locEventWriterEVIO->Write_EVIOEvent( loop, "eta2g_primexd_skim", locObjectsToSave );
      locEventWriterEVIO->Write_EVIOEvent( event, "eta2g-skim");
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

      dEventWriterEVIO->Write_EVIOEvent( loop, "eta2g_primexd_skim" );
    }

    if( WRITE_ROOT ){

      lockService->RootWriteLock();
      writeClustersToRoot( clusterVec );
      lockService->RootUnLock();
    }
  }
*/
}

//------------------
// EndRun
//------------------
void JEventProcessor_eta2g_primexd_skim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Fin
//------------------
void JEventProcessor_eta2g_primexd_skim::Finish()
{
  // Called before program exit after event processing is finished.
}


/*
void 
JEventProcessor_eta2g_primexd_skim::writeClustersToRoot( const vector< const DFCALCluster* > clusVec ){

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

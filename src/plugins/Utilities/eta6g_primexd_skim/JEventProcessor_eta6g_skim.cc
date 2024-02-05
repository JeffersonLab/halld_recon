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

#include "JEventProcessor_eta6g_skim.h"
using namespace jana;

// Routine used to create our JEventProcessor
#include "JANA/JApplication.h"
#include "TMath.h"
#include "JANA/JApplication.h"
#include "DANA/DApplication.h"
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
    app->AddProcessor(new JEventProcessor_eta6g_skim());
  }
} // "C"


//------------------
// JEventProcessor_eta6g_skim (Constructor)
//------------------
JEventProcessor_eta6g_skim::JEventProcessor_eta6g_skim()
{

  WRITE_EVIO = 1;
  WRITE_HDDM = 0;

  gPARMS->SetDefaultParameter( "ETA6G_SKIM:WRITE_EVIO", WRITE_EVIO );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:WRITE_HDDM", WRITE_HDDM );

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

  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MIN_MASS", MIN_MASS );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MAX_MASS", MAX_MASS );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MIN_E", MIN_E );                
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MIN_R", MIN_R );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MAX_DT", MAX_DT );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MAX_ETOT", MAX_ETOT );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:MIN_BLOCKS", MIN_BLOCKS );
  gPARMS->SetDefaultParameter( "ETA6G_SKIM:WRITE_ROOT", WRITE_ROOT );
  */
}

//------------------
// ~JEventProcessor_eta6g_skim (Destructor)
//------------------
JEventProcessor_eta6g_skim::~JEventProcessor_eta6g_skim()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_eta6g_skim::init(void)
{
  num_epics_events = 0;
/*
  if( ! ( WRITE_ROOT || WRITE_EVIO ) ){

    cerr << "No output mechanism has been specified." << endl;
    return UNRECOVERABLE_ERROR;
  }

  if( WRITE_ROOT ){

    japp->RootWriteLock();

    m_tree = new TTree( "cluster", "Cluster Tree for Pi0 Calibration" );
    m_tree->Branch( "nClus", &m_nClus, "nClus/I" );
    m_tree->Branch( "hit0", m_hit0, "hit0[nClus]/I" );
    m_tree->Branch( "px", m_px, "px[nClus]/F" );
    m_tree->Branch( "py", m_py, "py[nClus]/F" );
    m_tree->Branch( "pz", m_pz, "pz[nClus]/F" );

    m_tree->Branch( "nHit", &m_nHit, "nHit/I" );
    m_tree->Branch( "chan", m_chan, "chan[nHit]/I" );
    m_tree->Branch( "e", m_e, "e[nHit]/F" );

    japp->RootUnLock();
  }
*/
  combi6  = new Combination (6);
  combi7  = new Combination (7);
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_eta6g_skim::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  DGeometry* dgeom = NULL;
  DApplication* dapp = dynamic_cast< DApplication* >(eventLoop->GetJApplication());
  if (dapp) dgeom = dapp->GetDGeometry(runnumber);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    return RESOURCE_UNAVAILABLE;
  }	
  jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_eta6g_skim::evnt(JEventLoop *loop, uint64_t eventnumber)
{
 
  vector< const DFCALShower* > locFCALShowers;
  vector< const DBCALShower* > locBCALShowers;
  vector< const DCCALShower* > locCCALShowers;
  vector< const DVertex* > kinfitVertex;
  vector<const DTOFPoint*> tof_points;
  vector< const DBeamPhoton* > locBeamPhotons;
  vector< const DSCHit* > locSCHit;

  loop->Get(locFCALShowers);
  loop->Get(locBCALShowers);
  loop->Get(locCCALShowers);
  loop->Get(kinfitVertex);
  loop->Get(tof_points);
  loop->Get(locBeamPhotons);
  loop->Get(locSCHit);
  
  vector<const DTrackCandidate*> locTrackCandidate;
  loop->Get(locTrackCandidate);

  vector< const DTrackTimeBased* > locTrackTimeBased;
  loop->Get(locTrackTimeBased);

  

  vector < const DFCALShower * > matchedShowers;
  
  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  loop->GetSingle(locEventWriterEVIO);

  // always write out BOR events
  if(loop->GetJEvent().GetStatusBit(kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( loop, "eta6g_skim" );
      return NOERROR;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(loop->GetJEvent().GetStatusBit(kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( loop, "eta6g_skim" );
      num_epics_events++;
      return NOERROR;
  }

  
  vector< const JObject* > locObjectsToSave;  

  bool Candidate = false;
  vector<const DFCALHit *> locFCALHits;
  
  loop->Get(locFCALHits);  
  DVector3 vertex;
  vertex.SetXYZ(m_beamSpotX, m_beamSpotY, m_targetZ);
  for (unsigned int i = 0 ; i < tof_points.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(tof_points[i]));
  }
  for (unsigned int i = 0 ; i < locBCALShowers.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locBCALShowers[i]));
  }
  for (unsigned int i = 0 ; i < locFCALShowers.size(); i++) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locFCALShowers[i]));
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
  for (unsigned int i = 0 ; i < kinfitVertex.size(); i++) {
    if(i==0)
      locObjectsToSave.push_back(static_cast<const JObject *>(kinfitVertex[0]));
  }
  
  vector<const DEventRFBunch*> locEventRFBunches;
  loop->Get(locEventRFBunches);
  if(locEventRFBunches.size() > 0) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches[0]));
  }

  vector <DLorentzVector> PhotonList; PhotonList.clear(); 
  int photon_nb = locBCALShowers.size() + locFCALShowers.size() + locCCALShowers.size();
  for (unsigned int i = 0; i < locBCALShowers.size(); i ++) {
    double e =  locBCALShowers[i]->E;
    double x =  locBCALShowers[i]->x - kinfitVertexX;
    double y =  locBCALShowers[i]->y - kinfitVertexY;
    double z =  locBCALShowers[i]->z - kinfitVertexZ;
    DVector3 vertex(x, y, z);
    //double r = vertex.Mag();
    //double t = locCCALShowers[j]->time - (r / TMath::C());
    double p = e;
    double px = p * sin(vertex.Theta()) * cos(vertex.Phi());
    double py = p * sin(vertex.Theta()) * sin(vertex.Phi());
    double pz = p * cos(vertex.Theta());
    DLorentzVector PhotonVec(px, py, pz, e);
    if (e > 0.2) PhotonList.push_back(PhotonVec);
  }
  for (unsigned int i = 0; i < locFCALShowers.size(); i ++) {
    double e = locFCALShowers[i]->getEnergy();
    double x = locFCALShowers[i]->getPosition().X() - kinfitVertexX;
    double y = locFCALShowers[i]->getPosition().Y() - kinfitVertexY;
    double z = locFCALShowers[i]->getPosition().Z() - kinfitVertexZ;
    DVector3 vertex(x, y, z);
    //double r = vertex1.Mag();
    //double t = locFCALShowers[i]->getTime() - (r / TMath::C());
    double p = e;
    double px = p * sin(vertex.Theta()) * cos(vertex.Phi());
    double py = p * sin(vertex.Theta()) * sin(vertex.Phi());
    double pz = p * cos(vertex.Theta());
    DLorentzVector PhotonVec(px, py, pz, e);
    if (e > 0.2) PhotonList.push_back(PhotonVec);
  }
  for (unsigned int i = 0; i < locCCALShowers.size(); i ++) {
    double e =  locCCALShowers[i]->E;
    double x =  locCCALShowers[i]->x - kinfitVertexX;
    double y =  locCCALShowers[i]->y - kinfitVertexY;
    double z =  locCCALShowers[i]->z - kinfitVertexZ;
    DVector3 vertex(x, y, z);
    //double r = vertex.Mag();
    //double t = locCCALShowers[j]->time - (r / TMath::C());
    double p = e;
    double px = p * sin(vertex.Theta()) * cos(vertex.Phi());
    double py = p * sin(vertex.Theta()) * sin(vertex.Phi());
    double pz = p * cos(vertex.Theta());
    DLorentzVector PhotonVec(px, py, pz, e);
    //PhotonList.push_back(PhotonVec);
  }

  double FCAL_trg_Esum = 0;
  
  for (vector<const DFCALHit*>::const_iterator hit  = locFCALHits.begin(); hit != locFCALHits.end(); hit++ ) {
    if ((**hit).E > 0.150)
      FCAL_trg_Esum += (**hit).E;
  }

  int pi0_nb = 0;
  for (int i = 0; i < PhotonList.size(); i ++) {
    for (int j = i + 1; j < PhotonList.size(); j ++) {
      Double_t inv_mass = (PhotonList[i] + PhotonList[j]).M();
      if (0.09 <= inv_mass && inv_mass <= 0.2) pi0_nb ++;
    }
  }

  /*
  Double_t bestChi2Eta = 1.0e30;
  Double_t bestChi2EtaPrim = 1.0e30;
  vector <DLorentzVector> PhotonEta6gList;PhotonEta6gList.clear();
  vector <DLorentzVector> PhotonEtaprim6gList;PhotonEtaprim6gList.clear();
  Combined6g(PhotonList,
	     bestChi2Eta,
	     bestChi2EtaPrim,
	     PhotonEta6gList,
	     PhotonEtaprim6gList);
  Combined7g(PhotonList,
	     bestChi2Eta,
	     bestChi2EtaPrim,
	     PhotonEta6gList,
	     PhotonEtaprim6gList);

  Candidate |= ( (6 <= photon_nb && photon_nb <= 7) && (PhotonEta6gList.size() > 0 || PhotonEtaprim6gList.size() > 0) );
  */
  
  
  
  if (FCAL_trg_Esum > 3.5 && PhotonList.size() >= 6) {
    //cout <<"eta6g_skim"<<endl;
    if( WRITE_EVIO ){
      //locEventWriterEVIO->Write_EVIOEvent( loop, "eta6g_skim", locObjectsToSave );
      locEventWriterEVIO->Write_EVIOEvent( loop, "eta6g-skim");
    }
    if( WRITE_HDDM ) {
      vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
      loop->Get(locEventWriterHDDMVector);
      locEventWriterHDDMVector[0]->Write_HDDMEvent(loop, ""); 
    }
    
 }
 

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_eta6g_skim::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// Fin
//------------------
jerror_t JEventProcessor_eta6g_skim::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}
void JEventProcessor_eta6g_skim::Combined6g(vector<DLorentzVector>&EMList,
					    Double_t &bestChi0Eta,
					    Double_t &bestChi0EtaPrim,
					    vector<DLorentzVector>&PhotonEta6gList,
					    vector<DLorentzVector>&PhotonEtaprim6gList)
{	  
  bestChi0EtaPrim   = 1.0e30;
  bestChi0Eta       = 1.0e30;
  if(EMList.size() == 6) {
    for (int i_comb = 0; i_comb < (*combi6).getMaxCombi(); i_comb ++) {
      combi6->getCombi(i_comb);
      
      double Esum = 0.0;		  
      for (int i = 0; i < 6; i ++) {
	Esum += EMList[combi6->combi[i]].E();
      }
      
      double Chi2_pi0Mass[3];
      double Chi2_etaMass[3];
      vector<DLorentzVector>GG;GG.clear();
      vector<DLorentzVector>Pi0Cor;Pi0Cor.clear();
      vector<DLorentzVector>EtaCor;EtaCor.clear();
      
      for (int i = 0; i < 3; i ++) {
	GG.push_back(EMList[combi6->combi[2*i]] + EMList[combi6->combi[2*i+1]] );
	Pi0Cor.push_back( pi0Mass / GG[i].M() * GG[i] );
	EtaCor.push_back( etaMass / GG[i].M() * GG[i] );
	Chi2_pi0Mass[i] = TMath::Power((GG[i].M() - pi0Mass) / 12.8e-3,2.0);
	Chi2_etaMass[i] = TMath::Power((GG[i].M() - etaMass) / 31.1e-3,2.0);
      }
      
      double Chi2_3pi0      = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2]; 
      double Chi2_2pi0eta_0 = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_etaMass[2];
      double Chi2_2pi0eta_1 = Chi2_pi0Mass[0] + Chi2_etaMass[1] + Chi2_pi0Mass[2];
      double Chi2_2pi0eta_2 = Chi2_etaMass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2];
      
      if (Esum > 500.0e-3) {
	DLorentzVector EtaVec = Pi0Cor[0] + Pi0Cor[1] + Pi0Cor[2];
	bool AnEta     = false; 
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEta = true;
	
	bool AnEtaPrim  = false; 
	bool AnEtaPrim1 = false; 
	bool AnEtaPrim2 = false; 
	bool AnEtaPrim3 = false; 
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 500.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 600.0e-3)
	  AnEtaPrim1 = true;
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 500.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 600.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEtaPrim2 = true;
	if (GG[0].M() > 500.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 600.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEtaPrim3 = true;
	if (AnEtaPrim1 || AnEtaPrim2 || AnEtaPrim3)
	  AnEtaPrim = true;
	
	if (AnEta && !AnEtaPrim && Esum > 500.0e-3) {
	  if (Chi2_3pi0<bestChi0Eta) {
	    bestChi0Eta = Chi2_3pi0;
	    PhotonEta6gList.clear();
	    PhotonEta6gList.push_back(EMList[combi6->combi[0]]);
	    PhotonEta6gList.push_back(EMList[combi6->combi[1]]);
	    PhotonEta6gList.push_back(EMList[combi6->combi[2]]);
	    PhotonEta6gList.push_back(EMList[combi6->combi[3]]);
	    PhotonEta6gList.push_back(EMList[combi6->combi[4]]);
	    PhotonEta6gList.push_back(EMList[combi6->combi[5]]);
	  }
	}
	
	if (!AnEta && AnEtaPrim && Esum > 900.0e-3) {
	  double Esum2gg1 = EMList[combi6->combi[4]].E() + EMList[combi6->combi[5]].E();
	  if (AnEtaPrim1 && Esum2gg1 > 500.0e-3) {
	    if (Chi2_2pi0eta_0 < bestChi0EtaPrim) {
	      bestChi0EtaPrim = Chi2_2pi0eta_0;
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[1]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[3]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[5]]);
	    }
	  }
	  if (AnEtaPrim2) {
	    double Esum2gg2 = EMList[combi6->combi[2]].E() + EMList[combi6->combi[3]].E();
	    if (Chi2_2pi0eta_1 < bestChi0EtaPrim && Esum2gg2 > 500.0e-3) {
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[1]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[5]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[3]]);
	    }
	  }
	  if (AnEtaPrim3) {
	    double Esum2gg3 = EMList[combi6->combi[0]].E() + EMList[combi6->combi[1]].E();
	    if (Chi2_2pi0eta_2 < bestChi0EtaPrim && Esum2gg3 > 500.0e-3) {
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[3]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[5]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi6->combi[1]]);
	    }
	  }
	}
      }
    }
  }
  if(PhotonEta6gList.size()>0)
    PhotonEtaprim6gList.clear();
}
void JEventProcessor_eta6g_skim::Combined7g(vector<DLorentzVector>&EMList,
					    Double_t &bestChi0Eta,
					    Double_t &bestChi0EtaPrim,
					    vector<DLorentzVector>&PhotonEta6gList,
					    vector<DLorentzVector>&PhotonEtaprim6gList)
{	  
  bestChi0EtaPrim   = 1.0e30;
  bestChi0Eta       = 1.0e30;
  if (EMList.size() == 7) {
    for (int i_comb = 0; i_comb < (*combi7).getMaxCombi(); i_comb ++) {
      combi7->getCombi(i_comb);
      
      double Esum = 0.0;		  
      for (int i = 0; i < 6; i ++) {
	Esum += EMList[combi7->combi[i]].E();
      }
      
      double Chi2_pi0Mass[3];
      double Chi2_etaMass[3];
      vector<DLorentzVector>GG;GG.clear();
      vector<DLorentzVector>Pi0Cor;Pi0Cor.clear();
      vector<DLorentzVector>EtaCor;EtaCor.clear();
      
      for (int i = 0; i < 3; i ++) {
	GG.push_back( EMList[combi7->combi[2*i]] + EMList[combi7->combi[2*i+1]] );
	Pi0Cor.push_back( pi0Mass / GG[i].M() * GG[i] );
	EtaCor.push_back( etaMass / GG[i].M() * GG[i] );
	Chi2_pi0Mass[i] = TMath::Power((GG[i].M() - pi0Mass) / 12.8e-3,2.0);
	Chi2_etaMass[i] = TMath::Power((GG[i].M() - etaMass) / 31.1e-3,2.0);
      }
      
      if (Esum > 500.0e-3) {
	DLorentzVector EtaVec             = Pi0Cor[0] + Pi0Cor[1] + Pi0Cor[2];
	DLorentzVector EtaprimVec1        = Pi0Cor[0] + Pi0Cor[1] + EtaCor[2];
	DLorentzVector EtaprimVec2        = Pi0Cor[0] + EtaCor[1] + Pi0Cor[2];
	DLorentzVector EtaprimVec3        = EtaCor[0] + Pi0Cor[1] + Pi0Cor[2];
	DLorentzVector Candidate          = (DLorentzVector) EMList[combi7->combi[6]];
	Double_t CopAngleEta       = (fabs(EtaVec.Phi() - Candidate.Phi())) * TMath::RadToDeg();
	Double_t Chi2AngleEta      = TMath::Power((180.0  - CopAngleEta) / 30.0,2.0);
	
	Double_t CopAngleEtaprim1  = (fabs(EtaprimVec1.Phi() - Candidate.Phi())) * TMath::RadToDeg();
	Double_t Chi2AngleEtaprim1 = TMath::Power((180.0  - CopAngleEtaprim1) / 30.0,2.0);
	
	Double_t CopAngleEtaprim2  = (fabs(EtaprimVec2.Phi() - Candidate.Phi())) * TMath::RadToDeg();
	Double_t Chi2AngleEtaprim2 = TMath::Power((180.0  - CopAngleEtaprim2) / 30.0,2.0);
	
	Double_t CopAngleEtaprim3  = (fabs(EtaprimVec3.Phi() - Candidate.Phi())) * TMath::RadToDeg();
	Double_t Chi2AngleEtaprim3 = TMath::Power((180.0  - CopAngleEtaprim3) / 30.0,2.0);
	
	double Chi2_3pi0           = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2] + Chi2AngleEta; 
	double Chi2_2pi0eta_0      = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_etaMass[2] + Chi2AngleEtaprim1;
	double Chi2_2pi0eta_1      = Chi2_pi0Mass[0] + Chi2_etaMass[1] + Chi2_pi0Mass[2] + Chi2AngleEtaprim2;
	double Chi2_2pi0eta_2      = Chi2_etaMass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2] + Chi2AngleEtaprim3;
	/*
	  double Chi2_3pi0           = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2];
	  double Chi2_2pi0eta_0      = Chi2_pi0Mass[0] + Chi2_pi0Mass[1] + Chi2_etaMass[2];
	  double Chi2_2pi0eta_1      = Chi2_pi0Mass[0] + Chi2_etaMass[1] + Chi2_pi0Mass[2];
	  double Chi2_2pi0eta_2      = Chi2_etaMass[0] + Chi2_pi0Mass[1] + Chi2_pi0Mass[2];
	*/
	bool AnEta         = false; 
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEta = true;
	
	bool AnEtaPrim  = false; 
	bool AnEtaPrim1 = false; 
	bool AnEtaPrim2 = false; 
	bool AnEtaPrim3 = false; 
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 500.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 600.0e-3)
	  AnEtaPrim1 = true;
	if (GG[0].M() > 110.0e-3 && 
	    GG[1].M() > 500.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 160.0e-3 && 
	    GG[1].M() < 600.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEtaPrim2 = true;
	if (GG[0].M() > 500.0e-3 && 
	    GG[1].M() > 110.0e-3 && 
	    GG[2].M() > 110.0e-3 
	    && 
	    GG[0].M() < 600.0e-3 && 
	    GG[1].M() < 160.0e-3 && 
	    GG[2].M() < 160.0e-3)
	  AnEtaPrim3 = true;
	if(AnEtaPrim1 || AnEtaPrim2 || AnEtaPrim3)
	  AnEtaPrim = true;
	
	if (AnEta && !AnEtaPrim && Esum > 500.0e-3) {
	  if (Chi2_3pi0 < bestChi0Eta) {
	    bestChi0Eta = Chi2_3pi0;
	    PhotonEta6gList.clear();
	    PhotonEta6gList.push_back(EMList[combi7->combi[0]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[1]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[2]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[3]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[4]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[5]]);
	    PhotonEta6gList.push_back(EMList[combi7->combi[6]]);
	  }
	}
	
	if (!AnEta && AnEtaPrim && Esum > 900.0e-3) {
	  double Esum2gg1 = EMList[combi7->combi[4]].E() + EMList[combi7->combi[5]].E();
	  if (AnEtaPrim1 && Esum2gg1 > 500.0e-3) {
	    if(Chi2_2pi0eta_0 < bestChi0EtaPrim) {
	      bestChi0EtaPrim = Chi2_2pi0eta_0;
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[1]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[3]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[5]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[6]]);
	    }
	  }
	  double Esum2gg2 = EMList[combi7->combi[2]].E() + EMList[combi7->combi[3]].E();
	  if (AnEtaPrim2 && Esum2gg2 > 500.0e-3) {
	    if(Chi2_2pi0eta_1 < bestChi0EtaPrim) {
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[1]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[5]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[3]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[6]]);
	    }
	  }
	  double Esum2gg3 = EMList[combi7->combi[0]].E() + EMList[combi7->combi[1]].E();
	  if (AnEtaPrim3 && Esum2gg3 > 500.0e-3) {
	    if(Chi2_2pi0eta_2 < bestChi0EtaPrim) {
	      PhotonEtaprim6gList.clear();
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[2]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[3]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[4]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[5]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[0]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[1]]);
	      PhotonEtaprim6gList.push_back(EMList[combi7->combi[6]]);
	    }
	  }
	}
      }
    }
  }
  if(PhotonEta6gList.size()>0)
    PhotonEtaprim6gList.clear();
}
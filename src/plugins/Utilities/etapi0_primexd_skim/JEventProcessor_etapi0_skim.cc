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

#include "JEventProcessor_etapi0_skim.h"

// Routine used to create our JEventProcessor
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
#include "TLorentzVector.h"
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_etapi0_skim());
  }
} // "C"


//------------------
// JEventProcessor_etapi0_skim (Constructor)
//------------------
JEventProcessor_etapi0_skim::JEventProcessor_etapi0_skim()
{
  SetTypeName("JEventProcessor_etapi0_skim");
}

//------------------
// ~JEventProcessor_etapi0_skim (Destructor)
//------------------
JEventProcessor_etapi0_skim::~JEventProcessor_etapi0_skim()
{
}

//------------------
// Init
//------------------
void JEventProcessor_etapi0_skim::Init()
{
	auto app = GetApplication();
	// lockService = app->GetService<JLockService>();

	WRITE_EVIO = 1;
	WRITE_HDDM = 0;

	app->SetDefaultParameter( "ETAPI0_SKIM:WRITE_EVIO", WRITE_EVIO );
	app->SetDefaultParameter( "ETAPI0_SKIM:WRITE_HDDM", WRITE_HDDM );

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

	app->SetDefaultParameter( "ETAPI0_SKIM:MIN_MASS", MIN_MASS );
	app->SetDefaultParameter( "ETAPI0_SKIM:MAX_MASS", MAX_MASS );
	app->SetDefaultParameter( "ETAPI0_SKIM:MIN_E", MIN_E );
	app->SetDefaultParameter( "ETAPI0_SKIM:MIN_R", MIN_R );
	app->SetDefaultParameter( "ETAPI0_SKIM:MAX_DT", MAX_DT );
	app->SetDefaultParameter( "ETAPI0_SKIM:MAX_ETOT", MAX_ETOT );
	app->SetDefaultParameter( "ETAPI0_SKIM:MIN_BLOCKS", MIN_BLOCKS );
	app->SetDefaultParameter( "ETAPI0_SKIM:WRITE_ROOT", WRITE_ROOT );
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
    
  combi4  = new Combination (4);
  combi5  = new Combination (5);
}

//------------------
// BeginRun
//------------------
void JEventProcessor_etapi0_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
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
void JEventProcessor_etapi0_skim::Process(const std::shared_ptr<const JEvent>& event)
{
 
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

  // always write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "etapi0_skim" );
      return;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "etapi0_skim" );
      num_epics_events++;
      return;
  }

  
  vector< const JObject* > locObjectsToSave;  

  bool Candidate = false;
  
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
  event->Get(locEventRFBunches);
  if(locEventRFBunches.size() > 0) {
    locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches[0]));
  }
  vector <TLorentzVector> PhotonList; PhotonList.clear(); 
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
    TLorentzVector PhotonVec(px, py, pz, e);
    PhotonList.push_back(PhotonVec);
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
    TLorentzVector PhotonVec(px, py, pz, e);
    PhotonList.push_back(PhotonVec);
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
    TLorentzVector PhotonVec(px, py, pz, e);
    PhotonList.push_back(PhotonVec);
  }
  
  Double_t bestChi22Pi0 = 1.0e30;
  Double_t bestChi2Pi0Eta = 1.0e30;
  Double_t bestChi22Eta = 1.0e30;
  vector <TLorentzVector> Photon2Pi0List;Photon2Pi0List.clear();
  vector <TLorentzVector> PhotonPi0EtaList;PhotonPi0EtaList.clear();
  vector <TLorentzVector> Photon2EtaList;Photon2EtaList.clear();
  Combined4g(PhotonList,
	     bestChi22Pi0,
	     bestChi2Pi0Eta,
	     bestChi22Eta,
	     Photon2Pi0List,
	     PhotonPi0EtaList,
	     Photon2EtaList);
  Combined5g(PhotonList,
	     bestChi22Pi0,
	     bestChi2Pi0Eta,
	     bestChi22Eta,
	     Photon2Pi0List,
	     PhotonPi0EtaList,
	     Photon2EtaList);
  
  //Candidate |= ( (4 <= photon_nb && photon_nb <= 5) && (Photon2Pi0List.size() > 0 || PhotonPi0EtaList.size() > 0 || Photon2EtaList.size() > 0) );
  Candidate |= ( (4 <= photon_nb && photon_nb <= 5) && (PhotonPi0EtaList.size() > 0 || Photon2EtaList.size() > 0) );
  
  if ( Candidate ){
    //cout <<"etapi0_skim"<<endl;
    if( WRITE_EVIO ){
      //locEventWriterEVIO->Write_EVIOEvent( loop, "etapi0_skim", locObjectsToSave );
      locEventWriterEVIO->Write_EVIOEvent( event, "etapi0_skim");
    }
    if( WRITE_HDDM ) {
      vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
      event->Get(locEventWriterHDDMVector);
      locEventWriterHDDMVector[0]->Write_HDDMEvent(event, "");
    }
    
 }
}

//------------------
// EndRun
//------------------
void JEventProcessor_etapi0_skim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Fin
//------------------
void JEventProcessor_etapi0_skim::Finish()
{
  // Called before program exit after event processing is finished.
}


void JEventProcessor_etapi0_skim::Combined4g(vector<TLorentzVector>&EMList,
					     Double_t &bestChi22Pi0,
					     Double_t &bestChi2EtaPi0,
					     Double_t &bestChi22Eta,
					     vector<TLorentzVector>&Photon2Pi0List,
					     vector<TLorentzVector>&PhotonEtaPi0List,
					     vector<TLorentzVector>&Photon2EtaList)
{	  
  bestChi22Pi0   = 1.0e30;
  bestChi2EtaPi0 = 1.0e30;
  bestChi22Eta   = 1.0e30;

  if(EMList.size() == 4) {
    for (int i_comb = 0; i_comb < (*combi4).getMaxCombi(); i_comb ++) {
      combi4->getCombi(i_comb);
      
      double Esum = 0.0;		  
      for (int i = 0; i < 4; i ++) {
	Esum += EMList[combi4->combi[i]].E();
      }
      
      double Chi2_pi0Mass[3];
      double Chi2_etaMass[3];
      vector<TLorentzVector>GG;GG.clear();
      vector<TLorentzVector>Pi0Cor;Pi0Cor.clear();
      vector<TLorentzVector>EtaCor;EtaCor.clear();
      
      for (int i = 0; i < 2; i ++) {
	GG.push_back(EMList[combi4->combi[2*i]] + EMList[combi4->combi[2*i+1]] );
	Pi0Cor.push_back( pi0Mass / GG[i].M() * GG[i] );
	EtaCor.push_back( etaMass / GG[i].M() * GG[i] );
	Chi2_pi0Mass[i] = TMath::Power((GG[i].M() - pi0Mass) / 12.8e-3,2.0);
	Chi2_etaMass[i] = TMath::Power((GG[i].M() - etaMass) / 31.1e-3,2.0);
      }
      
      double Chi2_2pi0     = Chi2_pi0Mass[0] + Chi2_pi0Mass[1];
      double Chi2_pi0eta_0 = Chi2_pi0Mass[0] + Chi2_etaMass[1];
      double Chi2_pi0eta_1 = Chi2_etaMass[0] + Chi2_pi0Mass[1];
      double Chi2_2eta     = Chi2_etaMass[0] + Chi2_etaMass[1];
      bool An2Eta  = false; 
      bool An2Pi0 = false; 
      bool AnPi0Eta0 = false; 
      bool AnPi0Eta1 = false; 
      
      if (GG[0].M() > 400.0e-3 && 
	  GG[1].M() > 400.0e-3 &&
	  GG[0].M() < 1000.0e-3 && 
	  GG[1].M() < 1000.0e-3)
	An2Eta = true;

      if (GG[0].M() > 40.0e-3 && 
	  GG[1].M() > 40.0e-3 &&
	  GG[0].M() < 210.0e-3 && 
	  GG[1].M() < 210.0e-3)
	An2Pi0 = true;
      
      if (GG[0].M() > 40.0e-3 && 
	  GG[1].M() > 400.0e-3 &&
	  GG[0].M() < 210.0e-3 && 
	  GG[1].M() < 1000.0e-3)
	AnPi0Eta0 = true;

      if (GG[0].M() > 400.0e-3 && 
	  GG[1].M() > 40.0e-3 &&
	  GG[0].M() < 1000.0e-3 && 
	  GG[1].M() < 210.0e-3)
	AnPi0Eta1 = true;
      
      if (Chi2_2pi0 < bestChi22Pi0 && Esum > 0.25 && An2Pi0) {
	bestChi22Pi0 = Chi2_2pi0;
	Photon2Pi0List.clear();
	Photon2Pi0List.push_back(EMList[combi4->combi[0]]);
	Photon2Pi0List.push_back(EMList[combi4->combi[1]]);
	Photon2Pi0List.push_back(EMList[combi4->combi[2]]);
	Photon2Pi0List.push_back(EMList[combi4->combi[3]]);
      }
      if (Chi2_2eta < bestChi22Eta && Esum > 1.0 && An2Eta) {
	bestChi22Eta = Chi2_2eta;
	Photon2EtaList.clear();
	Photon2EtaList.push_back(EMList[combi4->combi[0]]);
	Photon2EtaList.push_back(EMList[combi4->combi[1]]);
	Photon2EtaList.push_back(EMList[combi4->combi[2]]);
	Photon2EtaList.push_back(EMList[combi4->combi[3]]);
      }
      double Esum2gg1 = EMList[combi4->combi[2]].E() + EMList[combi4->combi[3]].E();
      double Esum2gg2 = EMList[combi4->combi[0]].E() + EMList[combi4->combi[1]].E();
      if (Chi2_pi0eta_0 < bestChi2EtaPi0 && Esum2gg1 > 0.5 && Esum > 0.65 && AnPi0Eta0) {
	bestChi2EtaPi0 = Chi2_pi0eta_0;
	PhotonEtaPi0List.clear();
	PhotonEtaPi0List.push_back(EMList[combi4->combi[0]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[1]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[2]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[3]]);
      }
      if (Chi2_pi0eta_1 < bestChi2EtaPi0 && Esum2gg2 > 0.5 && Esum > 0.65 && AnPi0Eta1) {
	bestChi2EtaPi0 = Chi2_pi0eta_1;
	PhotonEtaPi0List.clear();
	PhotonEtaPi0List.push_back(EMList[combi4->combi[2]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[3]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[0]]);
	PhotonEtaPi0List.push_back(EMList[combi4->combi[1]]);
      }
    }
  }
}
void JEventProcessor_etapi0_skim::Combined5g(vector<TLorentzVector>&EMList,
					     Double_t &bestChi22Pi0,
					     Double_t &bestChi2EtaPi0,
					     Double_t &bestChi22Eta,
					     vector<TLorentzVector>&Photon2Pi0List,
					     vector<TLorentzVector>&PhotonEtaPi0List,
					     vector<TLorentzVector>&Photon2EtaList)
{	  
  bestChi22Pi0   = 1.0e30;
  bestChi2EtaPi0 = 1.0e30;
  bestChi22Eta   = 1.0e30;
  if (EMList.size() == 5) {
    for (int i_comb = 0; i_comb < (*combi5).getMaxCombi(); i_comb ++) {{
	combi5->getCombi(i_comb);
	
	double Esum = 0.0;		  
	for (int i = 0; i < 4; i ++) {
	  Esum += EMList[combi5->combi[i]].E();
	}
	
	double Chi2_pi0Mass[3];
	double Chi2_etaMass[3];
	vector<TLorentzVector>GG;GG.clear();
	vector<TLorentzVector>Pi0Cor;Pi0Cor.clear();
	vector<TLorentzVector>EtaCor;EtaCor.clear();
	
	for (int i = 0; i < 2; i ++) {
	  GG.push_back( EMList[combi5->combi[2*i]] + EMList[combi5->combi[2*i+1]] );
	  Pi0Cor.push_back( pi0Mass / GG[i].M() * GG[i] );
	  EtaCor.push_back( etaMass / GG[i].M() * GG[i] );
	  Chi2_pi0Mass[i] = TMath::Power((GG[i].M() - pi0Mass) / 12.8e-3,2.0);
	  Chi2_etaMass[i] = TMath::Power((GG[i].M() - etaMass) / 31.1e-3,2.0);
	}
	
	double Chi2_2pi0     = Chi2_pi0Mass[0] + Chi2_pi0Mass[1];
	double Chi2_pi0eta_0 = Chi2_pi0Mass[0] + Chi2_etaMass[1];
	double Chi2_pi0eta_1 = Chi2_etaMass[0] + Chi2_pi0Mass[1];
	double Chi2_2eta     = Chi2_etaMass[0] + Chi2_etaMass[1];
	if (Chi2_2pi0<bestChi22Pi0 && Esum > 0.25) {
	  bestChi22Pi0 = Chi2_2pi0;
	  Photon2Pi0List.clear();
	  Photon2Pi0List.push_back(EMList[combi5->combi[0]]);
	  Photon2Pi0List.push_back(EMList[combi5->combi[1]]);
	  Photon2Pi0List.push_back(EMList[combi5->combi[2]]);
	  Photon2Pi0List.push_back(EMList[combi5->combi[3]]);
	  Photon2Pi0List.push_back(EMList[combi5->combi[4]]);
	}
	if (Chi2_2eta<bestChi22Eta && Esum > 1.0) {
	  bestChi22Eta = Chi2_2eta;
	  Photon2EtaList.clear();
	  Photon2EtaList.push_back(EMList[combi5->combi[0]]);
	  Photon2EtaList.push_back(EMList[combi5->combi[1]]);
	  Photon2EtaList.push_back(EMList[combi5->combi[2]]);
	  Photon2EtaList.push_back(EMList[combi5->combi[3]]);
	  Photon2EtaList.push_back(EMList[combi5->combi[4]]);
	}
	double Esum2gg1 = EMList[combi5->combi[2]].E() + EMList[combi5->combi[3]].E();
	double Esum2gg2 = EMList[combi5->combi[0]].E() + EMList[combi5->combi[1]].E();
	if (Chi2_pi0eta_0<bestChi2EtaPi0 && Esum2gg1 > 0.5 && Esum > 0.65) {
	  bestChi2EtaPi0 = Chi2_pi0eta_0;
	  PhotonEtaPi0List.clear();
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[0]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[1]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[2]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[3]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[4]]);
	}
	if (Chi2_pi0eta_1<bestChi2EtaPi0 && Esum2gg2 > 0.5 && Esum > 0.65) {
	  bestChi2EtaPi0 = Chi2_pi0eta_1;
	  PhotonEtaPi0List.clear();
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[2]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[3]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[0]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[1]]);
	  PhotonEtaPi0List.push_back(EMList[combi5->combi[4]]);
	}
      }
    }
  }
}

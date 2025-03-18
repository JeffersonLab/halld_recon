// $Id$
//
//    File: JEventProcessor_pi0bcalskim.cc
// Created: Mon Dec  1 14:57:11 EST 2014 (copied structure from pi0fcalskim plugin)
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#include <math.h>
#include <TLorentzVector.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include "JEventProcessor_pi0bcalskim.h"


#include "TRACKING/DMCThrown.h"
#include "PID/DVertex.h"
#include "BCAL/DBCALShower.h"
#include "RF/DRFTime.h"
#include "PID/DEventRFBunch.h"
#include "HDDM/DEventWriterHDDM.h"

#include "DLorentzVector.h"
#include "TTree.h"
#include "units.h"
#include "ANALYSIS/DAnalysisUtilities.h"

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_pi0bcalskim());
  }
} // "C"


//------------------
// JEventProcessor_pi0bcalskim (Constructor)
//------------------
JEventProcessor_pi0bcalskim::JEventProcessor_pi0bcalskim()
{
}

//------------------
// ~JEventProcessor_pi0bcalskim (Destructor)
//------------------
JEventProcessor_pi0bcalskim::~JEventProcessor_pi0bcalskim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_pi0bcalskim::Init()
{
	auto app = GetApplication();

	MIN_SH1_E = 0.2;
	MIN_SH2_E = 0.2;

	WRITE_EVIO = 1;
	WRITE_HDDM = 0;

	app->SetDefaultParameter( "PI0BCALSKIM:WRITE_EVIO", WRITE_EVIO );
	app->SetDefaultParameter( "PI0BCALSKIM:WRITE_HDDM", WRITE_HDDM );
	app->SetDefaultParameter( "PI0BCALSKIM:MIN_SH1_E" , MIN_SH1_E );
	app->SetDefaultParameter( "PI0BCALSKIM:MIN_SH2_E" , MIN_SH2_E );

	num_epics_events = 0;

	//if( ! WRITE_EVIO) cerr << " output isnt working " << endl;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_pi0bcalskim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    /* Example
    // only write out BCAL data for these events
	const DEventWriterEVIO* locEventWriterEVIO = NULL;
	event->GetSingle(locEventWriterEVIO);
    
    if(locEventWriterEVIO) {
        locEventWriterEVIO->SetDetectorsToWriteOut("BCAL","pi0bcalskim");
    }
    */
  DGeometry* dgeom = GetDGeometry(event);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    return;
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
void JEventProcessor_pi0bcalskim::Process(const std::shared_ptr<const JEvent>& event)
{
  vector< const DBCALShower* > locBCALShowers;
  event->Get(locBCALShowers);
  vector< const DTrackTimeBased*> locTrackTimeBased;
  event->Get(locTrackTimeBased);
  vector<const DVertex*> kinfitVertex;
  event->Get(kinfitVertex);

  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);

  vector< const JObject* > locObjectsToSave;

  // always write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
      //jout << "Found BOR!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "pi0bcalskim" );
      return;
  }

  // write out the first few EPICS events to save run number & other meta info
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT) && (num_epics_events<5)) {
      //jout << "Found EPICS!" << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "pi0bcalskim" );
      num_epics_events++;
      return;
  }

  if(locBCALShowers.size() < 2 ) return;

  vector<const DTrackFitter*>fitters;
  event->Get(fitters);
  
  if(fitters.size()<1){
    _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
    return;
  }
  
  const DTrackFitter *fitter = fitters[0];

	bool Candidate = false;
	double sh1_E, sh2_E, inv_mass;
	double kinfitVertexX=m_beamSpotX;
	double kinfitVertexY=m_beamSpotY;
	double kinfitVertexZ=m_targetZ;
	vector <const DBCALShower *> matchedShowers;
	DVector3 mypos(0.0,0.0,0.0);
	for(unsigned int i = 0 ; i < locTrackTimeBased.size() ; ++i)
	{
	  vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_BCAL);
	  if (extrapolations.size()>0){
		for(unsigned int j = 0 ; j < locBCALShowers.size() ; ++j)
		{
			double x = locBCALShowers[j]->x;
			double y = locBCALShowers[j]->y;
			double z = locBCALShowers[j]->z;
		//	double E = locBCALShowers[j]->E;
			DVector3 pos_bcal(x,y,z);
			double R = pos_bcal.Perp();
		//	double phi = pos_bcal.Phi();
		//	double L2 = 0.81*2.54+65.0;
		//	double L3 = L2 + 0.81*2.54*2;
		//	double L4 = L3 + 0.81*2.54*3;
		//	double L5 = L4 + 0.97*2.54*4;
			if (fitter->ExtrapolateToRadius(R,extrapolations,mypos)){

			double dPhi =mypos.Phi()-pos_bcal.Phi();
			if (dPhi< -M_PI) dPhi+=2.*M_PI;
			if (dPhi> M_PI) dPhi-=2.*M_PI;
			double dZ = TMath::Abs(mypos.Z() - z);	
			if(dZ < 30.0 && fabs(dPhi) < 0.18) {
			  matchedShowers.push_back(locBCALShowers[j]);
			}
			}
		}
	  }
	}


	//lockService->RootWriteLock();
	
    vector<const DEventRFBunch*> locEventRFBunches;
    event->Get(locEventRFBunches);
    if(locEventRFBunches.size() > 0) {
        locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches[0]));
    }

	for (unsigned int i = 0 ; i < kinfitVertex.size(); i++)
	{
        // if the vertex information exists, save it as well
        if(i == 0)
            locObjectsToSave.push_back(static_cast<const JObject *>(kinfitVertex[0]));

		kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
		kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
		kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
		//kinfitVertexT = kinfitVertex[i]->dSpacetimeVertex.T();
	}

  for(unsigned int i=0; i<locBCALShowers.size() ; i++)	
  {
	if (find(matchedShowers.begin(), matchedShowers.end(),locBCALShowers[i]) != matchedShowers.end()) continue;
	 sh1_E = locBCALShowers[i]->E_raw;
	const DBCALShower *s1 = locBCALShowers[i];
	double sh1_x = s1->x - kinfitVertexX ;
	double sh1_y = s1->y - kinfitVertexY ;
	double sh1_z = s1->z - kinfitVertexZ ;
	double sh1_R = sqrt(sh1_x*sh1_x+sh1_y*sh1_y+sh1_z*sh1_z);
	TLorentzVector sh1_p(sh1_E*sh1_x/sh1_R,sh1_E*sh1_y/sh1_R,sh1_E*sh1_z/sh1_R,sh1_E);
	for(unsigned int j = i+1 ; j < locBCALShowers.size() ; j++)
	{
		if (find(matchedShowers.begin(), matchedShowers.end(),locBCALShowers[j]) != matchedShowers.end()) continue;
		const DBCALShower *s2 = locBCALShowers[j];
		 sh2_E = locBCALShowers[j]->E_raw;
		double sh2_x = s2->x - kinfitVertexX;
		double sh2_y = s2->y - kinfitVertexY;
		double sh2_z = s2->z - kinfitVertexZ;
		double sh2_R = sqrt(sh2_x*sh2_x + sh2_y*sh2_y + sh2_z*sh2_z);
		TLorentzVector sh2_p(sh2_E*sh2_x/sh2_R,sh2_E*sh2_y/sh2_R,sh2_E*sh2_z/sh2_R,sh2_E);
		TLorentzVector ptot = sh1_p+sh2_p;
		inv_mass = ptot.M();
		Candidate |= ( (sh2_E>0.4) && (inv_mass<0.25) && (inv_mass>0.05));
        if((sh2_E>0.4) && (inv_mass<0.25) && (inv_mass>0.05)) {
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locBCALShowers[i]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locBCALShowers[i]));
            if(find(locObjectsToSave.begin(), locObjectsToSave.end(), locBCALShowers[j]) == locObjectsToSave.end())
                locObjectsToSave.push_back(static_cast<const JObject *>(locBCALShowers[j]));
        }
	}
  }

  if(Candidate){
    if( WRITE_EVIO ) {
      //	cout << " inv mass = " << inv_mass << " sh1 E = " << sh1_E << " sh2 E = " << sh2_E << " event num = " << eventnumber << endl;
      //cout << "writing out " << eventnumber << endl;
      locEventWriterEVIO->Write_EVIOEvent( event, "pi0bcalskim", locObjectsToSave );
    }
    if( WRITE_HDDM ) {
      vector<const DEventWriterHDDM*> locEventWriterHDDMVector;
      event->Get(locEventWriterHDDMVector);
      locEventWriterHDDMVector[0]->Write_HDDMEvent(event, "");
    }
  }
  

  //lockService->RootUnLock();
  
   

}

//------------------
// EndRun
//------------------
void JEventProcessor_pi0bcalskim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Fin
//------------------
void JEventProcessor_pi0bcalskim::Finish()
{
  // Called before program exit after event processing is finished.
}


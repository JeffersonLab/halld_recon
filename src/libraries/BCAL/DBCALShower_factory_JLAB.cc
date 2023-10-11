// $Id: DBCALShower_factory_JLAB.cc  $
//
//    File: DBCALShower_factory_JLAB.cc
// Created: Mon Mar 18 09:42:29 EDT 2013
// Creator: Benedikt Zihlmann version 0.1
//

#include "TMath.h"
#include <JANA/JEvent.h>

#include "BCAL/DBCALClump.h"
#include "BCAL/DBCALShower_factory_JLAB.h"

using namespace std;

//------------------
// DBCALShower_factory_JLAB
//------------------
DBCALShower_factory_JLAB::DBCALShower_factory_JLAB()
{
	SetTag("JLAB");
}

//------------------
// BeginRun
//------------------
void DBCALShower_factory_JLAB::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // at this point load parameters from data base
}

//------------------
// Process
//------------------
void DBCALShower_factory_JLAB::Process(const std::shared_ptr<const JEvent>& event)
{

  vector<const DBCALClump*> ClumpList = event->Get<DBCALClump>();
  
  // at this point ClumpList contains all information to build Clusters
  // event over all Clumps in ClumpList and generate Clusters

  int id = 0;
  for (unsigned int i = 0; i < ClumpList.size(); i++){

    const DBCALClump* locClump = ClumpList[i];    
    DBCALShower *shower = new DBCALShower;
    
    shower->E_raw               = locClump->ClumpE[0];
    shower->x                   = TMath::Cos(locClump->ClumpPhi[0])*65.; // HARD CODED VALUE!!!!   
    shower->y                   = TMath::Sin(locClump->ClumpPhi[0])*65.; // HARD CODED VALUE!!!!   
    shower->z                   = locClump->ClumpPos[0] + 17.; // HARD CODED VALUE!!!!   
    shower->t                   = locClump->ClumpMT[0];
    shower->N_cell              = locClump->HitsU.size()+locClump->HitsD.size();
    
    // shower->xErr                = 0.5; // HARD CODED VALUE!!!!   
    // shower->yErr                = 0.5; // HARD CODED VALUE!!!!   
    // shower->zErr                = 2.5; // HARD CODED VALUE!!!!   
    // shower->tErr                = 0.2; // HARD CODED VALUE!!!!   
                                  
      
    shower->E = locClump->ClumpE[0];  // NO CALIBRATION DONE!!!!!

    //copy xyz errors into covariance matrix
    // shower->xyzCovariance.ResizeTo(3,3);
    // shower->xyzCovariance[0][0] = shower->xErr*shower->xErr;
    // shower->xyzCovariance[1][1] = shower->yErr*shower->yErr;
    // shower->xyzCovariance[2][2] = shower->zErr*shower->zErr;
    
    // add BCAL Hits as associated objects.
    for(unsigned int j=0; j<locClump->HitsU.size(); j++){
      shower->AddAssociatedObject(locClump->HitsU[j]);
    }
    for(unsigned int j=0; j<locClump->HitsD.size(); j++){
      shower->AddAssociatedObject(locClump->HitsD[j]);
    }
    
    Insert(shower);  
  }
}



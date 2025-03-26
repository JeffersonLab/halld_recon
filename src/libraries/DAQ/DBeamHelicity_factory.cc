// $Id$
//
//    File: DBeamHelicity_factory.cc
//

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <mutex>
using namespace std;

#include <DANA/DStatusBits.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DHELIDigiHit.h>
#include <DAQ/DHelicityData.h>
#include <DAQ/DEPICSvalue.h>
#include "DBeamHelicity_factory.h"

//init static class variable
int DBeamHelicity_factory::dIHWP   = 0;
int DBeamHelicity_factory::dBeamOn = 1;

//------------------
// Init
//------------------
void DBeamHelicity_factory::Init()
{

	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void DBeamHelicity_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

	// Grab information from CCDB tables here

	// Constants for determined helicity pattern (from Ken) 

	return; //NOERROR;
}

//------------------
// Process
//------------------
void DBeamHelicity_factory::Process(const std::shared_ptr<const JEvent>& event){
  auto eventnumber = event->GetEventNumber();
  double beamcurr     = 0.0;
  double beamthresh   = 10.0;
  
  // get info from EPICS variables
  vector<const DEPICSvalue*> epicsvalues;
  event->Get(epicsvalues);
  bool isEPICS = event->GetSingle<DStatusBits>()->GetStatusBit(kSTATUS_EPICS_EVENT);
  //kl I don't thing this works. Each thread has a separate instance of the factory, so these values only get set in the thread which
  //processed the EPICS event. Locking doesn't help. Worry about this lated when everything is moved into a factory.
  //sdobbs - note that for the "beam on" tests, in principle we could use the DBeamCurrent objects, however
  // that information isn't filled until after 
    if(isEPICS) {
    for(vector<const DEPICSvalue*>::const_iterator val_itr = epicsvalues.begin(); val_itr != epicsvalues.end(); val_itr++) {
      const DEPICSvalue* epics_val = *val_itr;
      if(epics_val->name == "IGL1I00DI24_24M"){
	dIHWP = atoi(epics_val->sval.c_str());
      }
      if(epics_val->name == "IBCAD00CRCUR6"){
	beamcurr = atof(epics_val->sval.c_str());
	if(beamcurr > beamthresh) dBeamOn = 1;
	else                      dBeamOn = 0;
	std::cout << "Got IBCAD00CRCUR6 = " << beamcurr << ". Setting dBeamOn = " <<  dBeamOn << "Event = "<< eventnumber << endl;
      }
    }
    return; //NOERROR;
  }
  
  DBeamHelicity *locBeamHelicity = nullptr;
  
  // get helicity bits from fADC (only for 2023 data)
  vector<const DHELIDigiHit*> locHELIDigiHits;
  event->Get(locHELIDigiHits);

  // or get helicity bits from new decoder board
  vector<const DHelicityData*> locHelicityDatas;
  event->Get(locHelicityDatas);

  if(!locHELIDigiHits.empty() && !locHelicityDatas.empty()) {
  	jerr << "both DHELIDigiHit and DHelicityData objects are in the data stream???" << endl;
  	jerr << "  not sure what to do, not creating DBeamHelicity objects ... " << endl;
  } 

  // make the object depending on which data type we have
  if(!locHELIDigiHits.empty())
  	locBeamHelicity = Make_DBeamHelicity(locHELIDigiHits);
  
  if(!locHELIDigiHits.empty())
  	locBeamHelicity = Make_DBeamHelicity(locHelicityDatas[0]);
  

  if(locBeamHelicity == nullptr)  return;
  
  Insert(locBeamHelicity);
  
  return; //NOERROR;
}

//------------------
// Make_DBeamHelicity
//------------------
DBeamHelicity *DBeamHelicity_factory::Make_DBeamHelicity(vector<const DHELIDigiHit*> &locHELIDigiHits)
{
	DBeamHelicity *locBeamHelicity = new DBeamHelicity;
	locBeamHelicity->pattern_sync  = 0;
	locBeamHelicity->t_settle      = 0;
	locBeamHelicity->helicity      = 0;
	locBeamHelicity->pair_sync     = 0;
	locBeamHelicity->ihwp          = dIHWP;
	locBeamHelicity->beam_on       = dBeamOn;
	locBeamHelicity->valid         = true;  //?
	
	for(size_t loc_i=0; loc_i<locHELIDigiHits.size(); loc_i++) {
		const DHELIDigiHit *locHELIDigiHit = locHELIDigiHits[loc_i];
		
		if(locHELIDigiHit->pulse_integral < 1000) continue; // threshold for signal
		
		if(locHELIDigiHit->chan == 0) locBeamHelicity->pattern_sync = 1;
		if(locHELIDigiHit->chan == 1) locBeamHelicity->t_settle = 1;
		if(locHELIDigiHit->chan == 2) locBeamHelicity->helicity = 1;
		if(locHELIDigiHit->chan == 3) locBeamHelicity->pair_sync = 1;
	}

	return locBeamHelicity;
}

//------------------
// Make_DBeamHelicity
//------------------
DBeamHelicity *DBeamHelicity_factory::Make_DBeamHelicity(const DHelicityData *locHelicityData)
{
	DBeamHelicity *locBeamHelicity = new DBeamHelicity;
	locBeamHelicity->pattern_sync  = locHelicityData->trigger_pattern_sync;
	locBeamHelicity->t_settle      = locHelicityData->trigger_tstable;
	locBeamHelicity->helicity      = locHelicityData->trigger_helicity_state;
	locBeamHelicity->pair_sync     = locHelicityData->trigger_pair_sync;
	locBeamHelicity->ihwp          = dIHWP;
	locBeamHelicity->beam_on       = dBeamOn;
	locBeamHelicity->valid         = true;  //?
	
	return locBeamHelicity;
}


//------------------
// EndRun
//------------------
void DBeamHelicity_factory::EndRun()
{
	return; //NOERROR;
}

//------------------
// Finish
//------------------
void DBeamHelicity_factory::Finish()
{
	return; //NOERROR;
}

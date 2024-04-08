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
#include <DAQ/DEPICSvalue.h>
#include "DBeamHelicity_factory.h"
using namespace jana;

//init static class variable
int DBeamHelicity_factory::dIHWP   = 0;
int DBeamHelicity_factory::dBeamOn = 1;

//------------------
// init
//------------------
jerror_t DBeamHelicity_factory::init(void)
{

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DBeamHelicity_factory::brun(jana::JEventLoop *loop, int32_t runnumber)
{

	// Grab information from CCDB tables here

	// Constants for determined helicity pattern (from Ken) 

	// Half Waveplate status at run start (could be from RCDB?)

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DBeamHelicity_factory::evnt(JEventLoop *loop, uint64_t eventnumber){
  double beamcurr     = 0.0;
  double beamthresh   = 10.0;
  
  // get info from EPICS variables
  vector<const DEPICSvalue*> epicsvalues;
  loop->Get(epicsvalues);
  bool isEPICS = loop->GetJEvent().GetStatusBit(kSTATUS_EPICS_EVENT);

  //kl I don't thing this works. Each thread has a separate instance of the factory, so these values only get set in the thread which
  //processed the EPICS event. Locking doesn't help. Worry about this lated when everything is moved into a factory.
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
    return NOERROR;
  }
  
  // get helicity bits from fADC
  vector<const DHELIDigiHit*> locHELIDigiHits;
  loop->Get(locHELIDigiHits);
  if(locHELIDigiHits.empty()) return NOERROR;
  
  DBeamHelicity *locBeamHelicity = new DBeamHelicity;
  locBeamHelicity->pattern_sync  = 0;
  locBeamHelicity->t_settle      = 0;
  locBeamHelicity->helicity      = 0;
  locBeamHelicity->pair_sync     = 0;
  locBeamHelicity->ihwp          = dIHWP;
  locBeamHelicity->beam_on       = dBeamOn;
  
  for(size_t loc_i=0; loc_i<locHELIDigiHits.size(); loc_i++) {
    
    const DHELIDigiHit *locHELIDigiHit = locHELIDigiHits[loc_i];
    
    if(locHELIDigiHit->pulse_integral < 1000) continue; // threshold for signal
    
    if(locHELIDigiHit->chan == 0) locBeamHelicity->pattern_sync = 1;
    if(locHELIDigiHit->chan == 1) locBeamHelicity->t_settle = 1;
    if(locHELIDigiHit->chan == 2) locBeamHelicity->helicity = 1;
    if(locHELIDigiHit->chan == 3) locBeamHelicity->pair_sync = 1;
  }
  
  _data.push_back(locBeamHelicity);
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DBeamHelicity_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DBeamHelicity_factory::fini(void)
{
	return NOERROR;
}
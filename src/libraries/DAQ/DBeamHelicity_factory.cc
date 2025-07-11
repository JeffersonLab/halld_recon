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
#include "DANA/DEvent.h"
#include "DBeamHelicity_factory.h"

//init static class variable
int DBeamHelicity_factory::dIHWP   = 0;
int DBeamHelicity_factory::dBeamOn = 1;

//------------------
// Init
//------------------
void DBeamHelicity_factory::Init()
{
	PREFER_PROMPT_HELICITY_DATA = true;
	
	auto app = GetApplication();
	app->SetDefaultParameter("PREFER_PROMPT_HELICITY_DATA", PREFER_PROMPT_HELICITY_DATA, "If both prompt and delayed helicity data are in the data stream, prefer the prompt. (default: true)");

	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void DBeamHelicity_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

	// Grab information from CCDB tables here
	dHDBoardDelay = 8;

    map<string,double> shift_factors;

    if (DEvent::GetCalib(event, "/ELECTRON_BEAM/helicity_board_shift", shift_factors))
        jout << "Error loading /ELECTRON_BEAM/helicity_board_shift !" << jendl;
    if (shift_factors.find("shift") != shift_factors.end()) {
        dHDBoardDelay = shift_factors["shift"];
    } else {
        jerr << "Unable to get correction from /ELECTRON_BEAM/helicity_board_shift !" << endl;
	}
	// Constants for determined helicity pattern? (from Ken) 


	// override parameters on command line
	auto app = GetApplication();
	app->SetDefaultParameter("HELICITY:HB_SHIFT", dHDBoardDelay, "Helicity board bits to shift to get prompt helicity. (default: 8)");
	
	
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
  
  // get helicity bits from fADC for delayed helicity signal
  vector<const DHELIDigiHit*> locHELIDigiHits;
  event->Get(locHELIDigiHits);

  // or get helicity bits from new decoder board
  vector<const DHelicityData*> locHelicityDatas;
  event->Get(locHelicityDatas);

  if(locHELIDigiHits.empty() && locHelicityDatas.empty())
    return;


  // make the object depending on which data type we have
  if(!locHelicityDatas.empty() && PREFER_PROMPT_HELICITY_DATA) {
  	locBeamHelicity = Make_DBeamHelicity(locHelicityDatas[0]);
  } else {
    if(!locHELIDigiHits.empty())
  	  locBeamHelicity = Make_DBeamHelicity(locHELIDigiHits);
  }
  

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
	if(!checkPredictor(locHelicityData->last_helicity_state_pattern_sync)) {
		jerr << "Consistency check of Helicity Decoder Board data failed!" << endl;
		return nullptr;
	}

	DBeamHelicity *locBeamHelicity = new DBeamHelicity;
	locBeamHelicity->pattern_sync  = locHelicityData->trigger_pattern_sync;
	locBeamHelicity->t_settle      = locHelicityData->trigger_tstable;
	locBeamHelicity->helicity      = helicityDecoderCalcPolarity(locHelicityData->trigger_event_polarity, locHelicityData->helicity_seed, dHDBoardDelay);
	locBeamHelicity->pair_sync     = locHelicityData->trigger_pair_sync;
	locBeamHelicity->ihwp          = dIHWP;
	locBeamHelicity->beam_on       = dBeamOn;
	locBeamHelicity->valid         = true;  //?
	
	return locBeamHelicity;
}

//------------------
// advanceSeed
//------------------
uint32_t DBeamHelicity_factory::advanceSeed(uint32_t seed) const
{
    const uint32_t bit7  = (seed & 0x00000040) != 0;
    const uint32_t bit28 = (seed & 0x08000000) != 0;
    const uint32_t bit29 = (seed & 0x10000000) != 0;
    const uint32_t bit30 = (seed & 0x20000000) != 0;
    const uint32_t newbit = (bit30 ^ bit29 ^ bit28 ^ bit7) & 0x1;
    return (newbit | (seed << 1)) & 0x3FFFFFFF;
}

//------------------
// helicityDecoderCalcPolarity
//------------------
 uint32_t DBeamHelicity_factory::helicityDecoderCalcPolarity(uint32_t event_polarity, uint32_t seed, uint32_t delay)
{
    for (uint32_t i = 0; i < delay; ++i)
        seed = advanceSeed(seed);
    return (seed ^ event_polarity) & 0x01;

}

//------------------
// checkPredictor
//------------------
bool DBeamHelicity_factory::checkPredictor(uint32_t testval) const
{
	UInt_t rval = (testval)&0x3fffffff;
	UInt_t lval = (testval>>2)&0x3fffffff;
	lval = advanceSeed(lval);
	lval = advanceSeed(lval);
	
	return (rval == lval);
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

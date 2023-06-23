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

//------------------
// init
//------------------
jerror_t DBeamHelicity_factory::init(void)
{
	dIHWP = 0;

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DBeamHelicity_factory::brun(jana::JEventLoop *loop, int32_t runnumber)
{

	// Grab information from CCDB tables here


	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DBeamHelicity_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// get half waveplate status from EPICS variable
	vector<const DEPICSvalue*> epicsvalues;
        loop->Get(epicsvalues);
        bool isEPICS = loop->GetJEvent().GetStatusBit(kSTATUS_EPICS_EVENT);
	if(isEPICS) {
		for(vector<const DEPICSvalue*>::const_iterator val_itr = epicsvalues.begin(); val_itr != epicsvalues.end(); val_itr++) {
			const DEPICSvalue* epics_val = *val_itr;
			int conv = atoi(epics_val->sval.c_str());
			if(epics_val->name == "IGL1I00OD24_24") 
				dIHWP = conv;
		}
		return NOERROR;
	}

	// get helicity bits from fADC
	vector<const DHELIDigiHit*> locHELIDigiHits;
	loop->Get(locHELIDigiHits);
	if(locHELIDigiHits.empty()) return NOERROR;

	DBeamHelicity *locBeamHelicity = new DBeamHelicity;
	locBeamHelicity->pattern_sync = 0;
	locBeamHelicity->t_settle = 0;
	locBeamHelicity->helicity = 0;
	locBeamHelicity->pair_sync = 0;
	locBeamHelicity->ihwp = dIHWP;

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

// $Id$
//
//    File: DBeamHelicity_factory_CORRECTED.cc
//

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <mutex>
using namespace std;

#include "DBeamHelicity_factory_CORRECTED.h"
#include "DANA/DEvent.h"

//------------------
// Init
//------------------
void DBeamHelicity_factory_CORRECTED::Init()
{
	auto app = GetApplication();
	app->SetDefaultParameter("HELICITY:REJECT_TSETTLE", REJECT_TSETTLE, "Reject events when the helicity is changing (t_settle is on). (default: 1)");

	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void DBeamHelicity_factory_CORRECTED::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // load correction factors
    map<string,double> scale_factors;

    if (DEvent::GetCalib(event, "/ELECTRON_BEAM/helicity_correction", scale_factors))
        jout << "Error loading /ELECTRON_BEAM/helicity_correction !" << jendl;
    if (scale_factors.find("correction") != scale_factors.end())
        dCorrectionFactor = scale_factors["correction"];
    else
        jerr << "Unable to get correction from /ELECTRON_BEAM/helicity_correction !" << endl;

    return; //NOERROR;
}

//------------------
// Process
//------------------
void DBeamHelicity_factory_CORRECTED::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DBeamHelicity*> locBeamHelicities;
	event->Get(locBeamHelicities);
	
	//for(auto beam_helicity : locBeamHelicities) {
	for(size_t loc_i = 0; loc_i < locBeamHelicities.size(); ++loc_i) {
  		// make some (optional) quality selections
		if(!locBeamHelicities[loc_i]->valid) continue;
  		if(REJECT_TSETTLE && locBeamHelicities[loc_i]->t_settle) {
  			return;
  		}
	
		DBeamHelicity *new_beam_helicity = new DBeamHelicity(*locBeamHelicities[loc_i]);
		new_beam_helicity->helicity *= dCorrectionFactor;
		
		new_beam_helicity->AddAssociatedObject(locBeamHelicities[loc_i]);
		Insert(new_beam_helicity);
	}
		
	return; //NOERROR;
}


//------------------
// EndRun
//------------------
void DBeamHelicity_factory_CORRECTED::EndRun()
{
	return; //NOERROR;
}

//------------------
// Finish
//------------------
void DBeamHelicity_factory_CORRECTED::Finish()
{
	return; //NOERROR;
}

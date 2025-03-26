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
	
	for(auto beam_helicity : locBeamHelicities) {
		DBeamHelicity *new_beam_helicity = new DBeamHelicity(*beam_helicity);
		new_beam_helicity->helicity *= dCorrectionFactor;
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

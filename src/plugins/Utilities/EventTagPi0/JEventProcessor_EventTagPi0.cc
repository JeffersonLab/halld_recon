// $Id$
//
//    File: JEventProcessor_EventTagPi0.cc
// Created: Fri Feb  5 23:23:22 EST 2016
// Creator: davidl (on Darwin harriet 13.4.0 i386)
//

#include "JEventProcessor_EventTagPi0.h"

#include <DANA/DStatusBits.h>
#include <FCAL/DFCALHit.h>

using namespace std;

// Routine used to create our JEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_EventTagPi0());
}
} // "C"


//------------------
// JEventProcessor_EventTagPi0 (Constructor)
//------------------
JEventProcessor_EventTagPi0::JEventProcessor_EventTagPi0()
{
	SetTypeName("JEventProcessor_EventTagPi0");
}

//------------------
// ~JEventProcessor_EventTagPi0 (Destructor)
//------------------
JEventProcessor_EventTagPi0::~JEventProcessor_EventTagPi0()
{

}

//------------------
// Init
//------------------
void JEventProcessor_EventTagPi0::Init()
{
	auto app = GetApplication();
	// lockService = app->GetService<JLockService>();

	Emin_MeV = 200.0;
	Rmin_cm  = 30.0;
	
	app->SetDefaultParameter("PI0TAG:Emin_MeV", Emin_MeV , "Minimum energy in MeV of each single block hit to tag event as FCAL pi0");
	app->SetDefaultParameter("PI0TAG:Rmin_cm" , Rmin_cm  , "Minimum distance in cm between single blocks with energy > PI0TAG:Emin_MeV to tag event as FCAL pi0");

	Rmin_cm_2 = Rmin_cm*Rmin_cm;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_EventTagPi0::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_EventTagPi0::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DFCALHit*> fcalhits;
	event->Get(fcalhits);
	
	if(fcalhits.size() < 2) return;
	
	for(uint32_t i=0; i<(fcalhits.size()-1); i++){
		const DFCALHit *hit1 = fcalhits[i];
		if( hit1->E < Emin_MeV ) continue;

		for(uint32_t j=i+1; j<fcalhits.size(); j++){
			const DFCALHit *hit2 = fcalhits[j];
			if( hit2->E < Emin_MeV ) continue;
	
			double deltaX = hit1->x - hit2->x;
			double deltaY = hit1->y - hit2->y;
			double r2 = deltaX*deltaX + deltaY*deltaY;
			if( r2 <= Rmin_cm_2 ){

				// TODO: NWB: This is dangerous, why are we doing this?
				const DStatusBits* sbc = event->GetSingle<DStatusBits>();
				DStatusBits* sb = const_cast<DStatusBits*>(sbc);
				sb->SetStatusBit(kSTATUS_FCAL_PI0);
				sb->SetStatusBit(kSTATUS_PI0);
				return;
			}
		}
	}
}

//------------------
// EndRun
//------------------
void JEventProcessor_EventTagPi0::EndRun()
{
}

//------------------
// Finish
//------------------
void JEventProcessor_EventTagPi0::Finish()
{
	// Called before program exit after event processing is finished.
}


// $Id$
//
//    File: JEventProcessor_HELI_online.cc
// Created: Thu Jun 22 16:59:57 EDT 2023
// Creator: jrsteven (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_HELI_online.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_HELI_online());
}
} // "C"

#include <TDirectory.h>

//------------------
// JEventProcessor_HELI_online (Constructor)
//------------------
JEventProcessor_HELI_online::JEventProcessor_HELI_online()
{

}

//------------------
// ~JEventProcessor_HELI_online (Destructor)
//------------------
JEventProcessor_HELI_online::~JEventProcessor_HELI_online()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_HELI_online::init(void)
{
	// This is called once at program startup. 

	const char *name[5] = {"pattern_sync","t_settle","helicity","pair_sync","ihwp"};

	// create root folder for trig and cd to it, store main dir
	TDirectory *main = gDirectory;
	gDirectory->mkdir("EPICS_dump")->cd();

	dBeamHelicity = new TH1F("BeamHelicity","Beam Helicity ",5,0,5);
	for(int i=1; i<=5; i++)
		dBeamHelicity->GetXaxis()->SetBinLabel(i,name[i-1]);

	// back to main dir
	main->cd();

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_HELI_online::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_HELI_online::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	vector<const DBeamHelicity*> locBeamHelicity;
	loop->Get(locBeamHelicity);

	if(locBeamHelicity.empty()) 
		return NOERROR;
	
	japp->RootFillLock(this);
	if(locBeamHelicity[0]->pattern_sync) dBeamHelicity->Fill(0);
	if(locBeamHelicity[0]->t_settle) dBeamHelicity->Fill(1);
	if(locBeamHelicity[0]->helicity) dBeamHelicity->Fill(2);
	if(locBeamHelicity[0]->pair_sync) dBeamHelicity->Fill(3);
	if(locBeamHelicity[0]->ihwp) dBeamHelicity->Fill(4);
	japp->RootFillUnLock(this);

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_HELI_online::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_HELI_online::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}


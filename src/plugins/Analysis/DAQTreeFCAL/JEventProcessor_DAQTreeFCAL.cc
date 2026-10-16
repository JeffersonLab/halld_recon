// $Id$
//
//    File: JEventProcessor_DAQTreeFCAL.cc
// Created: Thu Jan  9 12:29:31 PM EST 2025
// Creator: dalton (on Linux ifarm2402.jlab.org 5.14.0-427.42.1.el9_4.x86_64 x86_64)
//

#include "JEventProcessor_DAQTreeFCAL.h"
//using namespace jana;

#include <FCAL/DFCALDigiHit.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DANA/DEvent.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_DAQTreeFCAL());
}
} // "C"


//------------------
// JEventProcessor_DAQTreeFCAL (Constructor)
//------------------
JEventProcessor_DAQTreeFCAL::JEventProcessor_DAQTreeFCAL()
{
  SetTypeName("JEventProcessor_DAQTreeFCAL");
}

//------------------
// ~JEventProcessor_DAQTreeFCAL (Destructor)
//------------------
JEventProcessor_DAQTreeFCAL::~JEventProcessor_DAQTreeFCAL()
{

}

//------------------
// init
//------------------
void JEventProcessor_DAQTreeFCAL::Init()
{
	// This is called once at program startup. 
	FCALdigi = new TTree("FCALdigi","DFCALDigiHit objects (w/ waveform samples) for each channel and event");
	FCALdigi->Branch("channelnum",&channelnum,"channelnum/i");
	FCALdigi->Branch("eventnum",&eventnum,"eventnum/i");
	FCALdigi->Branch("rocid",&rocid,"rocid/i");
	FCALdigi->Branch("slot",&slot,"slot/i");
	FCALdigi->Branch("channel",&channel,"channel/i");
	FCALdigi->Branch("itrigger",&itrigger,"itrigger/i");
	FCALdigi->Branch("peak_raw",&peak_raw,"peak_raw/i");
	FCALdigi->Branch("integral",&integral,"integral/i");
	FCALdigi->Branch("pedestal",&pedestal,"pedestal/i");
	FCALdigi->Branch("time",&time,"time/i");
	FCALdigi->Branch("nsamples",&nsamples,"nsamples/i");
	FCALdigi->Branch("QF",&QF,"QF/i");
	FCALdigi->Branch("row",&row,"row/i");
	FCALdigi->Branch("column",&column,"column/i");
	FCALdigi->Branch("peak",&peak,"peak/f");
}

//------------------
// brun
//------------------
void JEventProcessor_DAQTreeFCAL::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// evnt
//------------------
void JEventProcessor_DAQTreeFCAL::Process(const std::shared_ptr<const JEvent>& event)
{
	eventnum = event->GetEventNumber();

	// Get the DFCALDigiHit objects
	vector<const DFCALDigiHit*> fcaldigihits;
	event->Get(fcaldigihits);

	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK

	// Loop over DFCALDigiHit objects
	for(unsigned int i=0; i< fcaldigihits.size(); i++){

		try {
			const DFCALDigiHit *fcaldigihit = fcaldigihits[i];
			
			const Df250PulseIntegral *pulseintegral = NULL;
			fcaldigihit->GetSingle(pulseintegral);
			//const Df250PulseData *pulsedata = NULL;
			//fcaldigihit->GetSingle(pulsedata);
			
			channelnum = i;
			if (pulseintegral) {
			    rocid = pulseintegral->rocid;
			    slot = pulseintegral->slot;
			    channel = pulseintegral->channel;
			    itrigger = pulseintegral->itrigger;
			} else {
			    rocid = slot = channel = itrigger = 0;
			}
			peak_raw = fcaldigihit->pulse_peak;
			integral = fcaldigihit->pulse_integral;
			pedestal = fcaldigihit->pedestal;
			time = fcaldigihit->pulse_time;
			nsamples = fcaldigihit->nsamples_integral;
			QF = fcaldigihit->QF;
			row = fcaldigihit->row;
			column = fcaldigihit->column;

			peak = peak_raw - pedestal;

			// Fill tree
			FCALdigi->Fill();

		} catch (...) {}
	}

	GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK

}

//------------------
// erun
//------------------
void JEventProcessor_DAQTreeFCAL::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// fini
//------------------
void JEventProcessor_DAQTreeFCAL::Finish()
{
	// Called before program exit after event processing is finished.
}


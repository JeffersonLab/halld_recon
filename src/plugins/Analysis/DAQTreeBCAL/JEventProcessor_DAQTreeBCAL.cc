// $Id$
//
//    File: JEventProcessor_DAQTreeBCAL.cc
// Created: Mon May  5 15:20:49 EDT 2014
// Creator: davidl (on Darwin harriet.jlab.org 13.1.0 i386)
//

#include <iostream>
using namespace std;

#include "JEventProcessor_DAQTreeBCAL.h"

#include <BCAL/DBCALDigiHit.h>
#include <BCAL/DBCALTDCDigiHit.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/DF1TDCHit.h>
#include <DANA/DEvent.h>


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_DAQTreeBCAL());
}
} // "C"


//------------------
// JEventProcessor_DAQTreeBCAL (Constructor)
//------------------
JEventProcessor_DAQTreeBCAL::JEventProcessor_DAQTreeBCAL()
{
	SetTypeName("JEventProcessor_DAQTreeBCAL");
}

//------------------
// ~JEventProcessor_DAQTreeBCAL (Destructor)
//------------------
JEventProcessor_DAQTreeBCAL::~JEventProcessor_DAQTreeBCAL()
{

}

//------------------
// Init
//------------------
void JEventProcessor_DAQTreeBCAL::Init()
{
	BCALdigi = new TTree("BCALdigi","DBCALDigiHit objects (w/ waveform samples) for each channel and event");
	BCALdigi->Branch("channelnum",&channelnum,"channelnum/i");
	BCALdigi->Branch("eventnum",&eventnum,"eventnum/i");
	BCALdigi->Branch("rocid",&rocid,"rocid/i");
	BCALdigi->Branch("slot",&slot,"slot/i");
	BCALdigi->Branch("channel",&channel,"channel/i");
	BCALdigi->Branch("itrigger",&itrigger,"itrigger/i");
	BCALdigi->Branch("waveform",&waveform);
	BCALdigi->Branch("nsamples",&nsamples,"nsamples/i");
	BCALdigi->Branch("w_integral",&w_integral,"w_integral/i");
	BCALdigi->Branch("w_min",&w_min,"w_min/i");
	BCALdigi->Branch("w_max",&w_max,"w_max/i");
	BCALdigi->Branch("w_samp1",&w_samp1,"w_samp1/i");

	BCALdigi->Branch("module",&module,"module/i");
	BCALdigi->Branch("layer",&layer,"layer/i");
	BCALdigi->Branch("sector",&sector,"sector/i");
	BCALdigi->Branch("end",&end,"end/i");



	BCALTDCdigi = new TTree("BCALTDCdigi","DBCALTDCDigiHit objects for each channel and event");
	BCALTDCdigi->Branch("channelnum",&channelnum,"channelnum/i");
	BCALTDCdigi->Branch("eventnum",&eventnum,"eventnum/i");
	BCALTDCdigi->Branch("rocid",&rocid,"rocid/i");
	BCALTDCdigi->Branch("slot",&slot,"slot/i");
	BCALTDCdigi->Branch("channel",&channel,"channel/i");
	BCALTDCdigi->Branch("itrigger",&itrigger,"itrigger/i");
	BCALTDCdigi->Branch("time",&time,"time/i");
	//BCALTDCdigi->Branch("trigger_time",&trigger_time,"trigger_time/i");

	BCALTDCdigi->Branch("module",&module,"module/i");
	BCALTDCdigi->Branch("layer",&layer,"layer/i");
	BCALTDCdigi->Branch("sector",&sector,"sector/i");
	BCALTDCdigi->Branch("end",&end,"end/i");

}

//------------------
// BeginRun
//------------------
void JEventProcessor_DAQTreeBCAL::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_DAQTreeBCAL::Process(const std::shared_ptr<const JEvent>& event)
{
	eventnum = event->GetEventNumber();

	// Get the DBCALDigiHit objects
	vector<const DBCALDigiHit*> bcaldigihits;
	event->Get(bcaldigihits);

	// Get the DBCALTDCDigiHit objects
	vector<const DBCALTDCDigiHit*> bcaltdcdigihits;
	event->Get(bcaltdcdigihits);

	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK

	// Loop over DBCALDigiHit objects
	for(unsigned int i=0; i< bcaldigihits.size(); i++){

		// Get Df250PulseIntegral and Df250WindowRawData objects
		// associated with this. If either fails then an exeption
		// is thrown (and caught) and the hit is ignored.
		try {
			const DBCALDigiHit *bcaldigihit = bcaldigihits[i];
			const Df250WindowRawData *windorawdata = NULL;
			
			const Df250PulseData *pulsedata = NULL;
			bcaldigihit->GetSingle(pulsedata);
			if(pulsedata){
				pulsedata->GetSingle(windorawdata);
			}else{
				const Df250PulseIntegral *pulseintegral = NULL;
				bcaldigihit->GetSingle(pulseintegral);
				pulseintegral->GetSingle(windorawdata);
			}
			
			if(!windorawdata){
				jerr << "No Df250WindowRawData objects associate with DBCALDigiHit!" << endl;
				continue;
			}

			pulsedata->GetSingle(windorawdata);
			
			waveform.clear();
			channelnum = i;
			rocid = windorawdata->rocid;
			slot = windorawdata->slot;
			channel = windorawdata->channel;
			itrigger = windorawdata->itrigger;

			// Get a vector of the samples for this channel
			const vector<uint16_t> &samplesvector = windorawdata->samples;
			nsamples=samplesvector.size();

			// loop over the samples to calculate integral, min, max
			for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
				waveform.push_back(samplesvector[c_samp]); // push the sample into the waveform vector
				if (c_samp==0) {  // use first sample for initialization
					w_integral = samplesvector[0]; 
					w_min = samplesvector[0];
					w_max = samplesvector[0];
					w_samp1 = samplesvector[0];
				} else {
					w_integral += samplesvector[c_samp];
					if (w_min > samplesvector[c_samp]) w_min = samplesvector[c_samp];
					if (w_max < samplesvector[c_samp]) w_max = samplesvector[c_samp];
				}
			}
			
			module = bcaldigihit->module;
			layer = bcaldigihit->layer;
			sector = bcaldigihit->sector;
			end = bcaldigihit->end;


			// Fill tree
			BCALdigi->Fill();

		} catch (...) {}
	}

	// Loop over DBCALTDCDigiHit objects
	for(unsigned int i=0; i< bcaltdcdigihits.size(); i++){

		const DBCALTDCDigiHit *bcaltdcdigihit = bcaltdcdigihits[i];
		const DF1TDCHit *tdchit;
		bcaltdcdigihit->GetSingle(tdchit);

		channelnum = i;
		rocid = tdchit->rocid;
		slot = tdchit->slot;
		channel = tdchit->channel;
		itrigger = tdchit->itrigger;
		
		time = bcaltdcdigihit->time;
		
		module = bcaltdcdigihit->module;
		layer = bcaltdcdigihit->layer;
		sector = bcaltdcdigihit->sector;
		end = bcaltdcdigihit->end;
		
		// Fill tree
		BCALTDCdigi->Fill();
	}

	GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_DAQTreeBCAL::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_DAQTreeBCAL::Finish()
{
	// Called before program exit after event processing is finished.
}


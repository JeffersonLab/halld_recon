// $Id$
//
//    File: JEventProcessor_event_size.cc
// Created: Tue Jun  7 10:07:36 EDT 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.7.0 i386)
//

#include "JEventProcessor_event_size.h"

// From level1_trigger plugin
#include "../level1_trigger/DTrigger.h"

#include <PID/DBeamPhoton.h>
#include <BCAL/DBCALHit.h>
#include <FCAL/DFCALHit.h>
#include <CCAL/DCCALHit.h>
#include <CDC/DCDCHit.h>
#include <FDC/DFDCHit.h>
#include <TOF/DTOFRawHit.h>
#include <START_COUNTER/DSCHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_event_size());
}
} // "C"


//------------------
// JEventProcessor_event_size (Constructor)
//------------------
JEventProcessor_event_size::JEventProcessor_event_size()
{
	SetTypeName("JEventProcessor_event_size");
}

//------------------
// ~JEventProcessor_event_size (Destructor)
//------------------
JEventProcessor_event_size::~JEventProcessor_event_size()
{

}

//------------------
// Init
//------------------
void JEventProcessor_event_size::Init()
{
	evt_tree = new TTree("event","Event Size info");
	evt = new Event();
	evt_tree->Branch("A", &evt);
	pthread_mutex_init(&evt_mutex, NULL);

	fdc_cathode_tree = new TTree("fdc_cathode","FDC Cathode hit info");
	fdc_cathode = new FDC_cathode();
	fdc_cathode_tree->Branch("A", &fdc_cathode);
	pthread_mutex_init(&fdc_mutex, NULL);

	fdc_anode_tree = new TTree("fdc_anode","FDC Anode hit info");
	fdc_anode = new FDC_anode();
	fdc_anode_tree->Branch("A", &fdc_anode);

	cdc_tree = new TTree("cdc","CDC hit info");
	cdc = new CDC();
	cdc_tree->Branch("A", &cdc);
	pthread_mutex_init(&cdc_mutex, NULL);

	fcal_tree = new TTree("fcal","FCAL hit info");
	fcal = new FCAL();
	fcal_tree->Branch("A", &fcal);
	pthread_mutex_init(&fcal_mutex, NULL);

	tof_tree = new TTree("tof","TOF hit info");
	tof = new TOF();
	tof_tree->Branch("A", &tof);
	pthread_mutex_init(&tof_mutex, NULL);

	toffset_bcal = 25.0;    // ns (lead time before trigger)
	twindow_bcal = 125.0;   // ns (full window width)
	toffset_fcal = 25.0;    // ns (lead time before trigger)
	twindow_fcal = 125.0;   // ns (full window width)
	toffset_ccal = 25.0;    // ns (lead time before trigger)
	twindow_ccal = 125.0;   // ns (full window width)
	toffset_cdc = 25.0;     // ns (lead time before trigger)
	twindow_cdc = 500.0;    // ns (full window width)
	toffset_fdc = 25.0;     // ns (lead time before trigger)
	twindow_fdc = 500.0;    // ns (full window width)
	toffset_tof = 25.0;     // ns (lead time before trigger)
	twindow_tof = 125.0;    // ns (full window width)
	toffset_sc = 25.0;      // ns (lead time before trigger)
	twindow_sc = 125.0;     // ns (full window width)
	toffset_tagger = 25.0;  // ns (lead time before trigger)
	twindow_tagger = 125.0; // ns (full window width)

	auto app = GetApplication();

	app->SetDefaultParameter("EVENTSIZE:toffset_bcal", toffset_bcal, "Time offset used to determine BCAL event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_bcal", twindow_bcal, "Time window used to determine BCAL event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_fcal", toffset_fcal, "Time offset used to determine FCAL event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_fcal", twindow_fcal, "Time window used to determine FCAL event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_ccal", toffset_ccal, "Time offset used to determine CCAL event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_ccal", twindow_ccal, "Time window used to determine CCAL event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_cdc", toffset_cdc, "Time offset used to determine CDC event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_cdc", twindow_cdc, "Time window used to determine CDC event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_fdc", toffset_fdc, "Time offset used to determine FDC event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_fdc", twindow_fdc, "Time window used to determine FDC event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_tof", toffset_tof, "Time offset used to determine TOF event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_tof", twindow_tof, "Time window used to determine TOF event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_sc", toffset_sc, "Time offset used to determine Start Counter event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_sc", twindow_sc, "Time window used to determine Start Counter event size");
	app->SetDefaultParameter("EVENTSIZE:toffset_tagger", toffset_tagger, "Time offset used to determine TAGGER event size");
	app->SetDefaultParameter("EVENTSIZE:twindow_tagger", twindow_tagger, "Time window used to determine TAGGER event size");

	// Calculate limits from offset and window width
	tmin_bcal   = -toffset_bcal;
	tmax_bcal   = tmin_bcal + twindow_bcal;
	tmin_fcal   = -toffset_fcal;
	tmax_fcal   = tmin_fcal + twindow_fcal;
	tmin_ccal   = -toffset_ccal;
	tmax_ccal   = tmin_ccal + twindow_ccal;
	tmin_cdc    = -toffset_cdc;
	tmax_cdc    = tmin_cdc + twindow_cdc;
	tmin_fdc    = -toffset_fdc;
	tmax_fdc    = tmin_fdc + twindow_fdc;
	tmin_tof    = -toffset_tof;
	tmax_tof    = tmin_tof + twindow_tof;
	tmin_sc     = -toffset_sc;
	tmax_sc     = tmin_sc + twindow_sc;
	tmin_tagger = -toffset_tagger;
	tmax_tagger = tmin_tagger + twindow_tagger;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_event_size::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// load BCAL geometry
  	vector<const DBCALGeometry *> BCALGeomVec;
  	event->Get(BCALGeomVec);
  	if(BCALGeomVec.size() == 0)
		throw JException("Could not load DBCALGeometry object!");
	const DBCALGeometry *dBCALGeom = BCALGeomVec[0];

	dBCALMid = dBCALGeom->GetBCAL_middle_cell();  // THIS IS PROBABLY WRONG!!!
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_event_size::Process(const std::shared_ptr<const JEvent>& event)
{
	// Ignore events with no Level 1 trigger info
	const DTrigger *trig=NULL;
	try{
		event->GetSingle(trig);
	}catch(...){
		return;
	};

	// Get hit data objects
	vector<const DBeamPhoton*> beamphotons;
	vector<const DBCALHit*> bcalhits;
	vector<const DFCALHit*> fcalhits;
	vector<const DCCALHit*> ccalhits;
	vector<const DCDCHit*> cdchits;
	vector<const DFDCHit*> fdchits;
	vector<const DTOFRawHit*> tofhits;
	vector<const DSCHit*> schits;
	vector<const DTAGMHit*> tagmhits;
	vector<const DTAGHHit*> taghhits;
	
	event->Get(beamphotons);
	event->Get(bcalhits);
	event->Get(fcalhits);
	event->Get(ccalhits);
	event->Get(cdchits);
	event->Get(fdchits);
	event->Get(tofhits);
	event->Get(schits);
	event->Get(tagmhits);
	event->Get(taghhits);
	
	// Count inner and outer BCAL hits
	unsigned int Nbcalhits_inner = 0;
	unsigned int Nbcalhits_outer = 0;
	for(unsigned int i=0; i<bcalhits.size(); i++){
	
		// Apply time window
		if(bcalhits[i]->t < tmin_bcal)continue;
		if(bcalhits[i]->t > tmax_bcal)continue;
		
		if(bcalhits[i]->layer < dBCALMid){
			Nbcalhits_inner++;
		}else{
			Nbcalhits_outer++;
		}
	}
	
	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK

	// Count FCAL hits
	unsigned int Nfcalhits = 0;
	for(unsigned int i=0; i<fcalhits.size(); i++){
		// Apply time window
		if(fcalhits[i]->t < tmin_fcal)continue;
		if(fcalhits[i]->t > tmax_fcal)continue;

		fcal->L1a_fired = trig->L1a_fired;
		fcal->L1b_fired = trig->L1b_fired;
		fcal->row = fcalhits[i]->row;
		fcal->column = fcalhits[i]->column;
		fcal_tree->Fill();

		Nfcalhits++;
	}
	
	// Count CCAL hits
	unsigned int Nccalhits = 0;
	for(unsigned int i=0; i<ccalhits.size(); i++){
		// Apply time window
		if(ccalhits[i]->t < tmin_ccal)continue;
		if(ccalhits[i]->t > tmax_ccal)continue;

		Nccalhits++;
	}
	
	// Count CDC hits
	unsigned int Ncdchits = 0;
	for(unsigned int i=0; i<cdchits.size(); i++){
		// Apply time window
		if(cdchits[i]->t < tmin_cdc)continue;
		if(cdchits[i]->t > tmax_cdc)continue;

		cdc->L1a_fired = trig->L1a_fired;
		cdc->L1b_fired = trig->L1b_fired;
		cdc->ring = cdchits[i]->ring;
		cdc->straw = cdchits[i]->straw;
		cdc_tree->Fill();

		Ncdchits++;
	}
	
	// Count FDC wire and FDC strip hits
	unsigned int Nfdchits_anode = 0;
	unsigned int Nfdchits_cathode = 0;
	for(unsigned int i=0; i<fdchits.size(); i++){
	
		// Apply time window
		if(fdchits[i]->t < tmin_fdc)continue;
		if(fdchits[i]->t > tmax_fdc)continue;

		if(fdchits[i]->type == 1){
			Nfdchits_cathode++;
			
			fdc_cathode->L1a_fired = trig->L1a_fired;
			fdc_cathode->L1b_fired = trig->L1b_fired;
			fdc_cathode->gPlane = fdchits[i]->gPlane;
			fdc_cathode->element = fdchits[i]->element;
			fdc_cathode_tree->Fill();
		}else{
			fdc_anode->L1a_fired = trig->L1a_fired;
			fdc_anode->L1b_fired = trig->L1b_fired;
			fdc_anode->gPlane = fdchits[i]->gPlane;
			fdc_anode->element = fdchits[i]->element;
			fdc_anode_tree->Fill();
			Nfdchits_anode++;
		}
	}
	
	// Count TOF hits
	unsigned int Ntofhits = 0;
	for(unsigned int i=0; i<tofhits.size(); i++){
		// Apply time window
		if(tofhits[i]->t < tmin_tof)continue;
		if(tofhits[i]->t > tmax_tof)continue;
		
		tof->plane = tofhits[i]->plane;
		tof->bar = tofhits[i]->bar;
		tof->lr = tofhits[i]->lr;
		tof_tree->Fill();

		Ntofhits++;
	}
	
	// Count Start Counter hits
	unsigned int Nschits = 0;
	for(unsigned int i=0; i<schits.size(); i++){
		// Apply time window
		if(schits[i]->t < tmin_sc)continue;
		if(schits[i]->t > tmax_sc)continue;

		Nschits++;
	}
	
	// Count Tagger hits
	unsigned int Ntagmhits = 0;
	for(unsigned int i=0; i<tagmhits.size(); i++){
		// Apply time window
		if(tagmhits[i]->t < tmin_tagger)continue;
		if(tagmhits[i]->t > tmax_tagger)continue;

		Ntagmhits++;
	}
	unsigned int Ntaghhits = 0;
	for(unsigned int i=0; i<taghhits.size(); i++){
		// Apply time window
		if(taghhits[i]->t < tmin_tagger)continue;
		if(taghhits[i]->t > tmax_tagger)continue;

		Ntaghhits++;
	}

	evt->event = eventnumber;
	evt->Egamma = beamphotons.size()>0 ? beamphotons[0]->momentum().Mag():0.0;
	evt->L1a_fired = trig->L1a_fired;
	evt->L1b_fired = trig->L1b_fired;
	evt->Ebcal_trig = trig->Ebcal;
	evt->Efcal_trig = trig->Efcal;
	evt->Nsc_trig = trig->Nschits;
	
	evt->Nbcalhits_inner = Nbcalhits_inner;
	evt->Nbcalhits_outer = Nbcalhits_outer;
	evt->Nfcalhits = Nfcalhits;
	evt->Nccalhits = Nccalhits;
	evt->Ncdchits = Ncdchits;
	evt->Nfdchits_anode = Nfdchits_anode;
	evt->Nfdchits_cathode = Nfdchits_cathode;
	evt->Ntofhits = Ntofhits;
	evt->Nschits = Nschits;
	evt->Ntagmhits = Ntagmhits;
	evt->Ntaghhits = Ntaghhits;

	evt->Ndigitized_values = 0;
	evt->Ndigitized_values += 3*evt->Nbcalhits_inner;    // fADC and TDC
	evt->Ndigitized_values += 2*evt->Nbcalhits_outer;    // fADC only
	evt->Ndigitized_values += 2*evt->Nfcalhits;          // fADC only
	evt->Ndigitized_values += 2*evt->Nccalhits;          // fADC only
	evt->Ndigitized_values += 2*evt->Ncdchits;           // fADC only
	evt->Ndigitized_values += 1*evt->Nfdchits_anode;     // TDC only
	evt->Ndigitized_values += 2*evt->Nfdchits_cathode;   // fADC only
	evt->Ndigitized_values += 3*evt->Ntofhits;           // fADC and TDC
	evt->Ndigitized_values += 3*evt->Nschits;            // fADC and TDC
	evt->Ndigitized_values += 3*evt->Ntagmhits;          // fADC and TDC
	evt->Ndigitized_values += 3*evt->Ntaghhits;          // fADC and TDC

	// Fill event tree
	evt_tree->Fill();
	
	GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_event_size::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_event_size::Finish()
{
	// Called before program exit after event processing is finished.
}


// $Id$
//
//    File: JEventProcessor_FMWPC_online.cc
// Created: Fri Oct 22 13:34:04 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_FMWPC_online.h"
using namespace jana;

#include "FMWPC/DFMWPCDigiHit.h"
#include "FMWPC/DFMWPCHit.h"
#include "FMWPC/DCTOFDigiHit.h"
#include "FMWPC/DCTOFTDCDigiHit.h"

#include <TDirectory.h>
#include <TH1.h>

static TH1I *fmwpc_num_events;
static TH1F *fmwpc_occ_layer[6];
static TH1F *fmwpc_hit_layer[6];

static TH1I *ctof_adc_events;
static TH1F *ctof_adc_occ_up;
static TH1F *ctof_adc_occ_down;
static TH1I *ctof_tdc_events;
static TH1F *ctof_tdc_occ_up;
static TH1F *ctof_tdc_occ_down;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_FMWPC_online());
}
} // "C"


//------------------
// JEventProcessor_FMWPC_online (Constructor)
//------------------
JEventProcessor_FMWPC_online::JEventProcessor_FMWPC_online(){
}

//------------------
// ~JEventProcessor_FMWPC_online (Destructor)
//------------------
JEventProcessor_FMWPC_online::~JEventProcessor_FMWPC_online(){
}

//------------------
// init
//------------------
jerror_t JEventProcessor_FMWPC_online::init(void)
{
	// This is called once at program startup.

        // create root folder for fdc and cd to it, store main dir
        TDirectory *main = gDirectory;
        gDirectory->mkdir("FMWPC")->cd();

        // book hist
        fmwpc_num_events = new TH1I("fmwpc_num_events","FMWPC Number of events",1, 0.5, 1.5);
        ctof_adc_events = new TH1I("ctof_adc_events","CTOF ADC Number of events",1, 0.5, 1.5);
        ctof_tdc_events = new TH1I("ctof_tdc_events","CTOF TDC Number of events",1, 0.5, 1.5);
 
        for(int iLayer = 0; iLayer < 6; iLayer++){
         char hname[256];
         char htitle[256];
         sprintf(hname, "fmwpc_occ_layer_%01d", iLayer+1);
         sprintf(htitle, "FMWPC Occupancy Layer %01d", iLayer+1);
         fmwpc_occ_layer[iLayer] = new TH1F(hname, htitle, 144, 0.5, 144.5);
         fmwpc_occ_layer[iLayer]->SetXTitle("Wire Number");

         sprintf(hname, "fmwpc_hit_layer_%01d", iLayer+1);
         sprintf(htitle, "FMWPC Calibrated Hits Layer %01d", iLayer+1);
         fmwpc_hit_layer[iLayer] = new TH1F(hname, htitle, 144, 0.5, 144.5);
         fmwpc_hit_layer[iLayer]->SetXTitle("Wire Number");
        }

	ctof_adc_occ_up = new TH1F("ctof_adc_occ_up", "CTOF ADC Occupancy", 4, 0.5, 4.5);
	ctof_adc_occ_up->SetXTitle("Bar Number");
	ctof_adc_occ_down = new TH1F("ctof_adc_occ_down", "CTOF ADC Occupancy", 4, 0.5, 4.5);
	ctof_adc_occ_down->SetXTitle("Bar Number");
	ctof_tdc_occ_up = new TH1F("ctof_tdc_occ_up", "CTOF TDC Occupancy", 4, 0.5, 4.5);
	ctof_tdc_occ_up->SetXTitle("Bar Number");
	ctof_tdc_occ_down = new TH1F("ctof_tdc_occ_down", "CTOF TDC Occupancy", 4, 0.5, 4.5);
	ctof_tdc_occ_down->SetXTitle("Bar Number");

        // back to main dir
        main->cd();

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FMWPC_online::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FMWPC_online::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

        // get wire digis
        vector<const DFMWPCDigiHit*>fmwpcdigis;
        loop->Get(fmwpcdigis);

        // get calibrated hits
        vector<const DFMWPCHit*>fmwpchits;
        loop->Get(fmwpchits);

        // get CTOF ADC digis
        vector<const DCTOFDigiHit*>ctofdigis;
        loop->Get(ctofdigis);

        // get CTOF TDC digis
        vector<const DCTOFTDCDigiHit*>ctoftdcdigis;
        loop->Get(ctoftdcdigis);

        // FILL HISTOGRAMS
        // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
        japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

        if( fmwpcdigis.size()>0 )
         fmwpc_num_events->Fill(1);

        for (unsigned int i=0; i<fmwpcdigis.size();i++){
         const DFMWPCDigiHit *digi = fmwpcdigis[i];

         fmwpc_occ_layer[digi->layer-1]->Fill(digi->wire);
        }

        for (unsigned int i=0; i<fmwpchits.size();i++){
         const DFMWPCHit *hit = fmwpchits[i];

         fmwpc_hit_layer[hit->layer-1]->Fill(hit->wire);
        }

        if( ctofdigis.size()>0 )
         ctof_adc_events->Fill(1);

        for (unsigned int i=0; i<ctofdigis.size();i++){
         const DCTOFDigiHit *digi = ctofdigis[i];
	 if (digi->end == 0)
	   ctof_adc_occ_up->Fill(digi->bar);
	 else if (digi->end == 1)
	   ctof_adc_occ_down->Fill(digi->bar);
	}

        if( ctoftdcdigis.size()>0 )
         ctof_tdc_events->Fill(1);

        for (unsigned int i=0; i<ctoftdcdigis.size();i++){
         const DCTOFTDCDigiHit *digi = ctoftdcdigis[i];
	 if (digi->end == 0)
	   ctof_tdc_occ_up->Fill(digi->bar);
	 else if (digi->end == 1)
	   ctof_tdc_occ_down->Fill(digi->bar);
	}

	japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FMWPC_online::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FMWPC_online::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}


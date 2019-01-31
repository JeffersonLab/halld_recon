// $Id$
//
//    File: JEventProcessor_DIRC_online.cc


#include <stdint.h>
#include <vector>

#include "JEventProcessor_DIRC_online.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include "TTAB/DTTabUtilities.h"
#include <DAQ/DDIRCTDCHit.h>
#include <DIRC/DDIRCTDCDigiHit.h>
#include <DIRC/DDIRCPmtHit.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

const int Nboxes = 2;
const int Nchannels = 108*64;
const int Npixelrows = 48;
const int Npixelcolumns = 144;

const int NmultBins = 50;
const int Nmult = 500;
const int NmultDigi = 1000;

// root hist pointers
static TH1I *dirc_num_events;
static TH1I *hLEDRefTime;
static TH1I *hHit_NHits;
static TH1I *hHit_Box;
static TH2I *hHit_NHitsVsBox;
static TH1I *hHit_TimeOverThreshold[Nboxes];
static TH2I *hHit_TimeOverThresholdVsChannel[Nboxes];
static TH2I *hHit_TimeOverThresholdVsTime[Nboxes];
static TH1I *hHit_tdcTime[Nboxes];
static TH2I *hHit_tdcTimeVsChannel[Nboxes];
static TH2I *hHit_tdcTimeDiffVsChannel[Nboxes];
static TH2I *hHit_pixelOccupancy[Nboxes];

static TH1I *hDigiHit_NtdcHits;
static TH1I *hDigiHit_Box;
static TH2I *hDigiHit_NtdcHitsVsBox;
static TH1I *hDigiHit_tdcTime[Nboxes];
static TH2I *hDigiHit_tdcTimeVsChannel[Nboxes];
static TH2I *hDigiHit_pixelOccupancy[Nboxes];

//----------------------------------------------------------------------------------

// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_DIRC_online());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_DIRC_online::JEventProcessor_DIRC_online() {
}


//----------------------------------------------------------------------------------


JEventProcessor_DIRC_online::~JEventProcessor_DIRC_online() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_DIRC_online::init(void) {

    // create root folder for psc and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *dircDir = gDirectory->mkdir("DIRC_online");
    dircDir->cd();

    hLEDRefTime = new TH1I("LEDRefTime", "LED reference SiPM time; time (ns)", 500, 0, 500);

    // book hists
    dirc_num_events = new TH1I("dirc_num_events","DIRC Number of events",1,0.5,1.5);
    TDirectory *hitDir = gDirectory->mkdir("Hit"); hitDir->cd();
    
    hHit_NHits = new TH1I("Hit_NHits","DIRCPmtHit multiplicity; hits; events",NmultBins,0.5,0.5+Nmult);
    hHit_Box = new TH1I("Hit_Box","DIRCPmtHit box; box; hits",2,-0.5,-0.5+2);
    hHit_NHitsVsBox = new TH2I("Hit_NHitsVsBox","DIRCPmtHit multiplicity vs. box; box; hits",2,-0.5,-0.5+2,NmultBins,0.5,0.5+Nmult);
    TString box_str[] = {"NorthUpper","SouthLower"};
    for (int i = 0; i < Nboxes; i++) {
        gDirectory->mkdir(box_str[i]+"Box")->cd();
        TString strN = "_" + box_str[i] + "Box";
        TString strT = ", " + box_str[i] + " box";
	hHit_pixelOccupancy[i] = new TH2I("Hit_PixelOccupancy"+strN,"DIRCPmtHit pixel occupancy"+strT+"; pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hHit_TimeOverThreshold[i] = new TH1I("Hit_TimeOverThreshold"+strN,"DIRCPmtHit time-over-threshold"+strT+"; time-over-threshold (ns); hits",100,0.0,100.);
	hHit_TimeOverThresholdVsChannel[i] = new TH2I("Hit_TimeOverThresholdVsChannel"+strN,"DIRCPmtHit time-over-threshold vs channel"+strT+"; channel; time-over-threshold [ns]",Nchannels,-0.5,-0.5+Nchannels,100,0.0,100.);
	hHit_TimeOverThresholdVsTime[i] = new TH2I("Hit_TimeOverThresholdVsTime"+strN,"DIRCPmtHit time-over-threshold vs time"+strT+"; time; time-over-threshold [ns]",500,0,500,100,0.0,100.);
	hHit_tdcTime[i] = new TH1I("Hit_Time"+strN,"DIRCPmtHit time"+strT+";time [ns]; hits",500,0.0,500.0);
	hHit_tdcTimeVsChannel[i] = new TH2I("Hit_TimeVsChannel"+strN,"DIRCPmtHit time vs. channel"+strT+"; channel;time [ns]",Nchannels,-0.5,-0.5+Nchannels,500,0.0,500.0);
	hHit_tdcTimeDiffVsChannel[i] = new TH2I("Hit_TimeDiffVsChannel"+strN,"DIRCPmtHit time vs. channel"+strT+"; channel;time [ns]",Nchannels,-0.5,-0.5+Nchannels,500,-500.0,500.0);
        hitDir->cd();
    }

    // DIRCTDC digihit-level hists
    dircDir->cd();
    TDirectory *digihitDir = gDirectory->mkdir("DigiHit"); digihitDir->cd();
    hDigiHit_NtdcHits = new TH1I("DigiHit_NHits","DIRCTDCDigiHit multiplicity;hits;events",NmultBins,0.5,0.5+NmultDigi);
    hDigiHit_Box = new TH1I("DigiHit_Box","DIRCTDCDigiHit box;box;hits",Nboxes,-0.5,-0.5+Nboxes);
    hDigiHit_NtdcHitsVsBox = new TH2I("DigiHit_NHitsVsBox","DIRCTDCDigiHit multiplicity vs box;box;hits",Nboxes,-0.5,-0.5+Nboxes,NmultBins,0.5,0.5+NmultDigi);
    for (int i = 0; i < Nboxes; i++) {
        gDirectory->mkdir(box_str[i]+"Box")->cd();
        TString strN = "_" + box_str[i] + "Box";
        TString strT = ", " + box_str[i] + " box";
	hDigiHit_pixelOccupancy[i] = new TH2I("TDCDigiHit_PixelOccupancy"+strN,"DIRCTDCDigiHit pixel occupancy"+strT+"; pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hDigiHit_tdcTime[i] = new TH1I("TDCDigiHit_Time"+strN,"DIRCTDCDigiHit time"+strT+";time [ns]; hits",500,0.0,500.0);
	hDigiHit_tdcTimeVsChannel[i] = new TH2I("TDCDigiHit_TimeVsChannel"+strN,"DIRCTDCDigiHit time"+strT+"; channel; time [ns]",Nchannels,-0.5,-0.5+Nchannels,500,0.0,500.0);
	digihitDir->cd();
    }
    // back to main dir
    mainDir->cd();

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_online::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

    vector<const DDIRCGeometry*> locDIRCGeometry;
    eventLoop->Get(locDIRCGeometry);
    dDIRCGeometry = locDIRCGeometry[0];

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_online::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

    // Get data for DIRC
    vector<const DDIRCTDCDigiHit*> digihits;
    eventLoop->Get(digihits);
    vector<const DDIRCPmtHit*> hits;
    eventLoop->Get(hits);

    // get LED trigger
    double locLEDRefTime = 0;
    if(true) {
	    
	    // Get LED SiPM reference
	    vector<const DCAEN1290TDCHit*> sipmtdchits;
	    eventLoop->Get(sipmtdchits);
	    vector<const Df250PulseData*> sipmadchits;
	    eventLoop->Get(sipmadchits);

	    for(uint i=0; i<sipmadchits.size(); i++) {
		    const Df250PulseData* sipmadchit = (Df250PulseData*)sipmadchits[i];
		    //cout<<sipmadchit->slot<<" "<<sipmadchit->channel<<" "<<sipmadchit->course_time<<" "<<sipmadchit->fine_time<<endl;
		    if(sipmadchit->rocid == 77 && sipmadchit->slot == 16 && sipmadchit->channel == 15) {
			    locLEDRefTime = ((sipmadchit->course_time<<6) + sipmadchit->fine_time) * 0.0625; // convert time from flash to ns
			    hLEDRefTime->Fill(locLEDRefTime); 
		    }
	    }
    }

    const DTTabUtilities* ttabUtilities = nullptr;
    eventLoop->GetSingle(ttabUtilities);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    if (digihits.size() > 0) dirc_num_events->Fill(1);

    // Fill digihit hists
    int NDigiHits[] = {0,0};
    hDigiHit_NtdcHits->Fill(digihits.size());
    for (const auto& hit : digihits) {
	int box = (hit->channel < Nchannels) ? 1 : 0;
        int channel = (hit->channel < Nchannels) ? hit->channel : (hit->channel - Nchannels);
        NDigiHits[box]++;
        hDigiHit_Box->Fill(box);
	hDigiHit_pixelOccupancy[box]->Fill(dDIRCGeometry->GetPixelRow(hit->channel), dDIRCGeometry->GetPixelColumn(hit->channel));
        hDigiHit_tdcTime[box]->Fill(hit->time);
        hDigiHit_tdcTimeVsChannel[box]->Fill(channel,hit->time);
    }
    hDigiHit_NtdcHitsVsBox->Fill(0.,NDigiHits[0]); hDigiHit_NtdcHitsVsBox->Fill(1.,NDigiHits[1]);

    // Fill calibrated-hit hists
    int NHits[] = {0,0};
    for (const auto& hit : hits) {
	int box = (hit->ch < Nchannels) ? 1 : 0;
        int channel = (hit->ch < Nchannels) ? hit->ch : (hit->ch - Nchannels);
	hHit_Box->Fill(box);
	NHits[box]++;
	hHit_pixelOccupancy[box]->Fill(dDIRCGeometry->GetPixelRow(hit->ch), dDIRCGeometry->GetPixelColumn(hit->ch));
	hHit_TimeOverThreshold[box]->Fill(hit->tot);
	hHit_TimeOverThresholdVsChannel[box]->Fill(channel,hit->tot);
	hHit_TimeOverThresholdVsTime[box]->Fill(hit->t, hit->tot);
	hHit_tdcTime[box]->Fill(hit->t);
	hHit_tdcTimeVsChannel[box]->Fill(channel,hit->t);
	hHit_tdcTimeDiffVsChannel[box]->Fill(channel,hit->t-locLEDRefTime);
    }
    hHit_NHits->Fill(NHits[0]+NHits[1]);
    hHit_NHitsVsBox->Fill(0.,NHits[0]); hHit_NHitsVsBox->Fill(1.,NHits[1]);

    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_online::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_online::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

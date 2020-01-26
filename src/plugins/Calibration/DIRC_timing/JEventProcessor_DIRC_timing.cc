// $Id$
//
//    File: JEventProcessor_DIRC_timing.cc


#include <stdint.h>
#include <vector>

#include "JEventProcessor_DIRC_timing.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include "TTAB/DTTabUtilities.h"
#include <DAQ/DDIRCTDCHit.h>
#include <DIRC/DDIRCTDCDigiHit.h>
#include <DIRC/DDIRCPmtHit.h>
#include <DIRC/DDIRCLEDRef.h>
#include <DAQ/DDIRCTriggerTime.h>
#include <DAQ/Df250PulseData.h>
#include <TRIGGER/DL1Trigger.h>

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

// Hit pointers
static TH1I *dirc_num_events;
static TH1I *hLEDRefAdcTime;
static TH1I *hLEDRefTdcTime;
static TH1I *hLEDRefIntegral;
static TH2I *hLEDRefAdcVsTdcTime;
static TH1I *hLEDRefTdcChannelTimeDiff;
static TH2I *hLEDRefTdcVsChannelTime;
static TH2I *hLEDRefAdcVsChannelTime;
static TH2I *hLEDRefIntegralVsTdcTime;
static TH1I *hRefTime;
static TH1I *hHit_NHits[2];
static TH1I *hHit_Box[2];
static TH2I *hHit_NHitsVsBox[2];
static TH1I *hHit_TimeOverThreshold[Nboxes][2];
static TH2I *hHit_TimeOverThresholdVsChannel[Nboxes][2];
static TH1I *hHit_tdcTime[Nboxes][2];
static TH2I *hHit_tdcTimeVsEvent[Nboxes][2];
static TH2I *hHit_tdcTimeVsChannel[Nboxes][2];
static TH2I *hHit_pixelOccupancy[Nboxes][2];

static TH2I *hHit_TimeEventMeanVsLEDRef[Nboxes];
static TH2I *hHit_TimeDiffEventMeanLEDRefVsTimestamp[Nboxes];

// DigiHit pointers
static TH1I *hDigiHit_NtdcHits[2];
static TH1I *hDigiHit_Box[2];
static TH2I *hDigiHit_NtdcHitsVsBox[2];
static TH1I *hDigiHit_tdcTime[Nboxes][2];
static TH2I *hDigiHit_tdcTimeVsChannel[Nboxes][2];
static TH2I *hDigiHit_pixelOccupancy[Nboxes][2];

// LED specific histograms
static TH2I *hHit_tdcTimeDiffVsChannel[Nboxes];
static TH1I *hHit_tdcTimeDiffEvent[Nboxes];
static TH2I *hHit_Timewalk[Nboxes][Nchannels];

/*--------------------------------------------------*/
// Data: Nov/25
// WL
// row is on the x axis, column is on the y axis

static TH2I *hHit_pixelOccupancy_LED_1[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_2[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_3[Nboxes];

static TH2I *hHit_pixelOccupancy_LED_2_narrow[Nboxes];

static TH2I *hHit_pixelOccupancy_LED_Event_1[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_Event_2[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_Event_3[Nboxes];

static TH2I *hHit_pixelOccupancy_LED_Event_5000[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_Event_5000_cut[Nboxes];

static TH2I *hHit_pixelOccupancy_LED_timing_cut_in[Nboxes];
static TH2I *hHit_pixelOccupancy_LED_timing_cut_out[Nboxes];

static TH2I *hHit_pixelOccupancy_LED_fourth_peak[Nboxes];

static TH1I *hHit_LEDRefTime[Nboxes];
static TH1I *hHit_LEDRefTdcChannelTimeDiff[Nboxes];

static TH1I *hHit_tdcTime_LED_5000[Nboxes];

uint64_t event_counter;

static TH1I *hHit_areaTime[Nboxes][18];

static TH2I *hHit_pixelOccupancy_LED_column[Nboxes][18];

//static TH1I *hHit_per_Event_photon_LED[Nboxes][2];

static TH2I *hHit_pixelRow_timing[Nboxes];


//----------------------------------------------------------------------------------

// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_DIRC_timing());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_DIRC_timing::JEventProcessor_DIRC_timing() {
}


//----------------------------------------------------------------------------------


JEventProcessor_DIRC_timing::~JEventProcessor_DIRC_timing() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_DIRC_timing::init(void) {

	event_counter = 0;

    FillTimewalk = false;
    gPARMS->SetDefaultParameter("DIRC:FILLTIMEWALK", FillTimewalk, "Fill timewalk histograms, default = false");

    // create root folder for psc and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *dircDir = gDirectory->mkdir("DIRC_timing");
    dircDir->cd();

    hLEDRefAdcTime = new TH1I("LEDRefAdcTime", "LED ADC reference SiPM time; time (ns)", 100, 0, 100);
    hLEDRefTdcTime = new TH1I("LEDRefTdcTime", "LED TDC reference SiPM time; time (ns)", 100, 0, 100);
    hLEDRefIntegral = new TH1I("LEDRefIntegral", "LED reference SiPM integral; integral (ADC)", 100, 1000, 1500);
    hLEDRefAdcVsTdcTime = new TH2I("LEDRefAdcVsTdcTime", "LED ADC vs TDC reference SiPM time; TDC time (ns); ADC time (ns)", 100, 0, 100, 100, 0, 100);
    hLEDRefTdcChannelTimeDiff = new TH1I("LEDRefTdcChannelTimeDiff", "PMT pixel hit - LED TDC reference SiPM time; PMT Channel - SiPM TDC time (ns)", 100, 0, 100);
    hLEDRefTdcVsChannelTime = new TH2I("LEDRefTdcVsChannelTime", "LED TDC reference SiPM time vs PMT pixel hit time; PMT Channel time (ns); TDC time (ns)", 100, 0, 1000, 100, 0, 100);
    hLEDRefAdcVsChannelTime = new TH2I("LEDRefAdcVsChannelTime", "LED ADC reference SiPM time vs PMT pixel hit time; PMT Channel time (ns); ADC time (ns)", 100, 0, 1000, 100, 0, 100);
    hLEDRefIntegralVsTdcTime = new TH2I("LEDRefIntegralVsTdcTime", "LED TDC reference SiPM time; TDC time (ns); Pulse Integral", 100, 0, 100, 100, 1000, 1500);
    hRefTime = new TH1I("RefTime", "Reference time from mean hit time; time (ns)", 100, 0, 1000);



    // book hists
    dirc_num_events = new TH1I("dirc_num_events","DIRC Number of events",1,0.5,1.5);
    TDirectory *hitDir = gDirectory->mkdir("Hit"); hitDir->cd();
  
    TString trig_str[] = {"LED","NonLED"}; 
    for (int j = 0; j < 2; j++) {
	hHit_NHits[j] = new TH1I("Hit_NHits_"+trig_str[j],"DIRCPmtHit multiplicity " + trig_str[j] + "; hits; events",NmultBins,0.5,0.5+Nmult);
	hHit_Box[j] = new TH1I("Hit_Box_"+trig_str[j],"DIRCPmtHit box" + trig_str[j] + "; box; hits",2,-0.5,-0.5+2);
    	hHit_NHitsVsBox[j] = new TH2I("Hit_NHitsVsBox_"+trig_str[j],"DIRCPmtHit multiplicity vs. box" + trig_str[j] + "; box; hits",2,-0.5,-0.5+2,NmultBins,0.5,0.5+Nmult);
    }

    TString box_str[] = {"NorthUpper","SouthLower"};
    for (int i = 0; i < Nboxes; i++) {
	gDirectory->mkdir(box_str[i]+"Box")->cd();
	for (int j = 0; j < 2; j++) {
		TString strN = "_" + trig_str[j];
		TString strT = ", " + box_str[i] + " box " + trig_str[j] + " trigger";
		hHit_pixelOccupancy[i][j] = new TH2I("Hit_PixelOccupancy"+strN,"DIRCPmtHit pixel occupancy "+strT+"; pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

		hHit_TimeOverThreshold[i][j] = new TH1I("Hit_TimeOverThreshold"+strN,"DIRCPmtHit time-over-threshold "+strT+"; time-over-threshold (ns); hits",100,0.0,100.);
		hHit_TimeOverThresholdVsChannel[i][j] = new TH2I("Hit_TimeOverThresholdVsChannel"+strN,"DIRCPmtHit time-over-threshold vs channel "+strT+"; channel; time-over-threshold [ns]",Nchannels,-0.5,-0.5+Nchannels,100,0.0,100.);
		hHit_tdcTime[i][j] = new TH1I("Hit_Time"+strN,"DIRCPmtHit time "+strT+";time [ns]; hits",500,0.0,500.0);
		hHit_tdcTimeVsEvent[i][j] = new TH2I("Hit_TimeVsEvent"+strN,"DIRCPmtHit time "+strT+"; event #; time [ns]; hits",1000,0,100e6,500,0.0,500.0);
		hHit_tdcTimeVsChannel[i][j] = new TH2I("Hit_TimeVsChannel"+strN,"DIRCPmtHit time vs. channel "+strT+"; channel;time [ns]",Nchannels,-0.5,-0.5+Nchannels,500,0.0,500.0);

//		hHit_per_Event_photon_LED[i][j] = new TH1I("per_event_photon_"+strN,"Per Event Photon Distribution"+strT+";time [ns]; hits",200,0.0,1000.0);

	}

	hHit_pixelOccupancy_LED_1[i] = new TH2I("Hit_PixelOccupancy_LED_1","DIRCPmtHit pixel occupancy LED 1",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hHit_pixelOccupancy_LED_2[i] = new TH2I("Hit_PixelOccupancy_LED_2","DIRCPmtHit pixel occupancy LED 2",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hHit_pixelOccupancy_LED_3[i] = new TH2I("Hit_PixelOccupancy_LED_3","DIRCPmtHit pixel occupancy LED 3",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);


	hHit_pixelOccupancy_LED_2_narrow[i] = new TH2I("Hit_PixelOccupancy_LED_2_narrow","DIRCPmtHit pixel occupancy LED 2",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_pixelOccupancy_LED_Event_1[i] = new TH2I("Hit_PixelOccupancy_Event_5001","DIRCPmtHit pixel occupancy event 1",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hHit_pixelOccupancy_LED_Event_2[i] = new TH2I("Hit_PixelOccupancy_Event_5002","DIRCPmtHit pixel occupancy event 2",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
	hHit_pixelOccupancy_LED_Event_3[i] = new TH2I("Hit_PixelOccupancy_Event_5003","DIRCPmtHit pixel occupancy event 3",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_pixelOccupancy_LED_Event_5000[i] = new TH2I("Hit_PixelOccupancy_Event_5000_event","DIRCPmtHit pixel occupancy event first",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_pixelOccupancy_LED_Event_5000_cut[i] = new TH2I("Hit_PixelOccupancy_Event_5000_event_cut","DIRCPmtHit pixel occupancy event first",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_tdcTime_LED_5000[i] = new TH1I("Hit_Time_5000","DIRCPmtHit time; time [ns]; hits",500,0.0,500.0);

	hHit_pixelOccupancy_LED_timing_cut_in[i] = new TH2I("Hit_PixelOccupancy_timing_cut_in","DIRCPmtHit pixel occupancy  pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_pixelOccupancy_LED_timing_cut_out[i] = new TH2I("Hit_PixelOccupancy_timing_cut_out","DIRCPmtHit pixel occupancy  pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	hHit_pixelOccupancy_LED_fourth_peak[i] = new TH2I("Hit_PixelOccupancy_timing_fourth_peak","DIRCPmtHit pixel occupancy  pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	// LED specific histograms
	hHit_tdcTimeDiffVsChannel[i] = new TH2I("Hit_LEDTimeDiffVsChannel","LED DIRCPmtHit time diff vs. channel; channel;time [ns]",Nchannels,-0.5,-0.5+Nchannels,100,-10.0,30.0);
	hHit_tdcTimeDiffEvent[i] = new TH1I("Hit_LEDTimeDiffEvent","LED DIRCPmtHit time diff in event; #Delta t [ns]", 300,-50, 250);
	hHit_TimeEventMeanVsLEDRef[i] = new TH2I("Hit_TimeEventMeanVsLEDRef","LED Time Event Mean DIRCPmtHit time vs. LED Reference time; LED reference time [ns] ; LED pixel event mean time [ns]", 100, 100, 150, 400, -10, 30);
    hHit_TimeDiffEventMeanLEDRefVsTimestamp[i] = new TH2I("Hit_TimeDiffeventMeanLEDRefVsTimestamp","LED Time Event Mean DIRCPmtHit time - LED reference time vs. event timestamp; event timestamp [ns?] ; time difference [ns]", 1000, 0, 1e10, 400, 100, 140);
	hHit_pixelRow_timing[i] =   new TH2I("Hit_pixelRow_timing","Hit_pixelRow_timing; channel; time-over-threshold [ns]", 150,0,150,100,-50,50);

	/*--------------------------------------------------*/
	// LED SiPM reference time
	
    hHit_LEDRefTime[i] = new TH1I("Hit_LEDRefTime", "LED TDC reference SiPM time; SiPM TDC time (ns)", 100, 0,100);
    hHit_LEDRefTdcChannelTimeDiff[i] = new TH1I("Hit_LEDRefTdcChannelTimeDiff", "PMT pixel hit - LED TDC reference SiPM time; PMT Channel - SiPM TDC time (ns)", 300, 0, 150);

	for(int k = 0; k < 18; k++) {

		TString name; 
		name.Form("%d", k); 

		hHit_areaTime[i][k] = new TH1I("Hit_LEDTimeDiffEvent_" + name,"LED DIRCPmtHit time diff in event; #Delta t [ns]",200,-50,50);

		hHit_pixelOccupancy_LED_column[i][k] = new TH2I("hHit_pixelOccupancy_LED_column_" + name, "LED DIRCPmtHit time diff in event; #Delta t [ns]", Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);

	}

	if(FillTimewalk) {
		gDirectory->mkdir("Timewalk")->cd();	
		for(int j=0; j<Nchannels; j++) {
			hHit_Timewalk[i][j] = new TH2I(Form("Hit_Timewalk_%d",j),Form("DIRCPmtHit channel %d: #Delta t vs time-over-threshold; time-over-threshold [ns]; #Delta t [ns]",j),100,0,100,100,-50.,50.);
		}
	}

        hitDir->cd();
    }

    // DIRCTDC digihit-level hists
    dircDir->cd();
    TDirectory *digihitDir = gDirectory->mkdir("DigiHit"); digihitDir->cd();

    for (int j = 0; j < 2; j++) {
	hDigiHit_NtdcHits[j] = new TH1I("DigiHit_NHits_"+trig_str[j],"DIRCTDCDigiHit multiplicity "+ trig_str[j] +";hits;events",NmultBins,0.5,0.5+NmultDigi);
    	hDigiHit_Box[j] = new TH1I("DigiHit_Box_"+trig_str[j],"DIRCTDCDigiHit box" + trig_str[j] + ";box;hits",Nboxes,-0.5,-0.5+Nboxes);
    	hDigiHit_NtdcHitsVsBox[j] = new TH2I("DigiHit_NHitsVsBox_"+trig_str[j],"DIRCTDCDigiHit multiplicity vs box" + trig_str[j] +";box;hits",Nboxes,-0.5,-0.5+Nboxes,NmultBins,0.5,0.5+NmultDigi);
    }

    for (int i = 0; i < Nboxes; i++) {
        gDirectory->mkdir(box_str[i]+"Box")->cd();
	for (int j = 0; j < 2; j++) {
                TString strN = "_" + trig_str[j];
                TString strT = ", " + box_str[i] + " box " + trig_str[j] + " trigger";
		hDigiHit_pixelOccupancy[i][j] = new TH2I("TDCDigiHit_PixelOccupancy"+strN,"DIRCTDCDigiHit pixel occupancy"+strT+"; pixel rows; pixel columns",Npixelcolumns,-0.5,-0.5+Npixelcolumns,Npixelrows,-0.5,-0.5+Npixelrows);
		hDigiHit_tdcTime[i][j] = new TH1I("TDCDigiHit_Time"+strN,"DIRCTDCDigiHit time"+strT+";time [ns]; hits",500,0.0,500.0);
		hDigiHit_tdcTimeVsChannel[i][j] = new TH2I("TDCDigiHit_TimeVsChannel"+strN,"DIRCTDCDigiHit time"+strT+"; channel; time [ns]",Nchannels,-0.5,-0.5+Nchannels,500,0.0,500.0);
	}

	digihitDir->cd();
    }
    // back to main dir
    mainDir->cd();

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_timing::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_timing::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

    vector<const DDIRCGeometry*> locDIRCGeometryVec;
    eventLoop->Get(locDIRCGeometryVec);
    auto locDIRCGeometry = locDIRCGeometryVec[0];

    // Get data for DIRC
    vector<const DDIRCTDCDigiHit*> digihits;
    eventLoop->Get(digihits);
    vector<const DDIRCPmtHit*> hits;
    eventLoop->Get(hits);
    vector<const DDIRCTriggerTime*> timestamps;
    eventLoop->Get(timestamps);

    const DTTabUtilities* locTTabUtilities = nullptr;
    eventLoop->GetSingle(locTTabUtilities);

    // Get DCODAROCInfo for this ROC
    vector<const DCODAROCInfo*> locCODAROCInfos;
    eventLoop->Get(locCODAROCInfos);
    uint64_t locReferenceClockTime = 0;
    for (const auto& locCODAROCInfo : locCODAROCInfos) {
	if(locCODAROCInfo->rocid == 92) {
		locReferenceClockTime = locCODAROCInfo->timestamp;
	}
    }    
    //if(locReferenceClockTime%2 != 0) 
    //	return NOERROR;

    // check for LED triggers
    bool locDIRCLEDTrig = false;
    bool locPhysicsTrig = false;
    vector<const DL1Trigger*> trig;
    eventLoop->Get(trig);
    if (trig.size() > 0) {
	    // LED appears as "bit" 15 in L1 front panel trigger monitoring plots
	    if (trig[0]->fp_trig_mask & 0x4000){ 
		    locDIRCLEDTrig = true;
	    }
	    // Physics trigger appears as "bit" 1 in L1 trigger monitoring plots
	    if (trig[0]->trig_mask & 0x1){ 
	    	    locPhysicsTrig = true;
	    }
    }
    int loc_itrig = 1;
    if(locDIRCLEDTrig) loc_itrig = 0;
    else if(locPhysicsTrig) loc_itrig = 1;
    else return NOERROR;

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

	/*--------------------------------------------------*/
	// First 5000 events: treatment, determine LED fiber #1 average hit time.
    for (const auto& hit : hits) {
		
		if (event_counter < 5000) {

			int box = (hit->ch < Nchannels) ? 1 : 0;
		    int channel = (hit->ch < Nchannels) ? hit->ch : (hit->ch - Nchannels);
	
	        int pmtrow = locDIRCGeometry->GetPmtRow(channel);
	        int pmtcol = locDIRCGeometry->GetPmtColumn(channel);
	
			if (locDIRCLEDTrig) {
					hHit_pixelOccupancy_LED_Event_5000[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
	
					if ( pmtrow > 13 && pmtrow < 16 && pmtcol >0 && pmtcol < 5) {			
						hHit_tdcTime_LED_5000[box]->Fill(hit->t);
						hHit_pixelOccupancy_LED_Event_5000_cut[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
	
				}
			}
		}
	}

	if (event_counter < 5000){ 
		event_counter++;
		return NOERROR;
	}

    // LED specific information
    // next line commented out to supress warning: variable not used
    //    double locLEDRefTime = 0;
    double locLEDRefAdcTime;
    double locLEDRefTdcTime;

    if(locDIRCLEDTrig) {


		locLEDRefAdcTime = 0;
		locLEDRefTdcTime = 0;	    

	    // Get LED SiPM reference
	    //vector<const DCAEN1290TDCHit*> sipmtdchits;
	    //eventLoop->Get(sipmtdchits);
	    //vector<const Df250PulseData*> sipmadchits;
	    //eventLoop->Get(sipmadchits);

	    vector<const DDIRCLEDRef*> dircLEDRefs;
            eventLoop->Get(dircLEDRefs);

	    for(uint i=0; i<dircLEDRefs.size(); i++) {

			const DDIRCLEDRef* dircLEDRef = (DDIRCLEDRef*)dircLEDRefs[i];
			locLEDRefAdcTime = dircLEDRef->t_fADC;
			locLEDRefTdcTime = dircLEDRef->t_TDC;



//			cout << dircLEDRefs.size() << endl;

		// next line commented out to supress warning: variable not used
		//		locLEDRefTime = dircLEDRef->t_TDC;
// //		japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
//                 hLEDRefAdcTime->Fill(locLEDRefAdcTime); 
//                 hLEDRefIntegral->Fill(dircLEDRef->integral);
// 		hLEDRefTdcTime->Fill(locLEDRefTdcTime);
// 		hLEDRefAdcVsTdcTime->Fill(locLEDRefTdcTime, locLEDRefAdcTime);
// 		hLEDRefIntegralVsTdcTime->Fill(locLEDRefTdcTime, dircLEDRef->integral);
// 		japp->RootFillUnLock(this); //ACQUIRE ROOT FILL LOCK

			japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

        	hLEDRefAdcTime->Fill(locLEDRefAdcTime); 
        	hLEDRefIntegral->Fill(dircLEDRef->integral);

			hLEDRefTdcTime->Fill(locLEDRefTdcTime);
			hLEDRefAdcVsTdcTime->Fill(locLEDRefTdcTime, locLEDRefAdcTime);

			hLEDRefIntegralVsTdcTime->Fill(locLEDRefTdcTime, dircLEDRef->integral);
			japp->RootFillUnLock(this); //ACQUIRE ROOT FILL LOCK

//			cout << "q11111:    " << locLEDRefTdcTime << "    " << locLEDRefAdcTime << endl;

	    }
    }



/*
	    for(uint i=0; i<sipmadchits.size(); i++) {
		    const Df250PulseData* sipmadchit = (Df250PulseData*)sipmadchits[i];
		    if(sipmadchit->rocid == 77 && sipmadchit->slot == 16 && sipmadchit->channel == 15) {
			    locLEDRefAdcTime = (double)((sipmadchit->course_time<<6) + sipmadchit->fine_time);
			    locLEDRefAdcTime *= 0.0625; // convert time from flash to ns
			    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
			    hLEDRefAdcTime->Fill(locLEDRefAdcTime); 
			    hLEDRefIntegral->Fill(sipmadchit->integral); 
			    japp->RootFillUnLock(this); //ACQUIRE ROOT FILL LOCK
			    locLEDRefTime = locLEDRefAdcTime;
		    }
	    }
	    
	    for(uint i=0; i<sipmtdchits.size(); i++) {
		    const DCAEN1290TDCHit* sipmtdchit = (DCAEN1290TDCHit*)sipmtdchits[i];
		    if(sipmtdchit->rocid == 78 && sipmtdchit->slot == 8 && sipmtdchit->channel == 30) {
			    locLEDRefTdcTime = (double)((sipmtdchit->time));
			    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
			    hLEDRefTdcTime->Fill(locLEDRefTdcTime); 
			    japp->RootFillUnLock(this); //ACQUIRE ROOT FILL LOCK	     
			    locLEDRefTime = locLEDRefTdcTime;
		    }
	    }
    }
*/

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    if (digihits.size() > 0) dirc_num_events->Fill(1);

    // Fill digihit hists
    int NDigiHits[] = {0,0};
    hDigiHit_NtdcHits[loc_itrig]->Fill(digihits.size());
    for (const auto& hit : digihits) {
	int box = (hit->channel < Nchannels) ? 1 : 0;
        int channel = (hit->channel < Nchannels) ? hit->channel : (hit->channel - Nchannels);
        NDigiHits[box]++;
        hDigiHit_Box[loc_itrig]->Fill(box);
		hDigiHit_pixelOccupancy[box][loc_itrig]->Fill(locDIRCGeometry->GetPixelRow(hit->channel), locDIRCGeometry->GetPixelColumn(hit->channel));
        hDigiHit_tdcTime[box][loc_itrig]->Fill(hit->time);
        hDigiHit_tdcTimeVsChannel[box][loc_itrig]->Fill(channel,hit->time);
    }
    hDigiHit_NtdcHitsVsBox[loc_itrig]->Fill(0.,NDigiHits[0]); hDigiHit_NtdcHitsVsBox[loc_itrig]->Fill(1.,NDigiHits[1]);

    // Loop over calibrated hits to get mean for reference time
    double locRefTime[2];
    int locNHits[2];

    locNHits[0]=0.0;
    locNHits[1]=0.0;

    for (const auto& hit : hits) {

		int box = (hit->ch < Nchannels) ? 1 : 0;
        int channel = (hit->ch < Nchannels) ? hit->ch : (hit->ch - Nchannels);

 //   	double locFirstFiberTime = hHit_tdcTime_LED_5000[box]->GetBinCenter(hHit_tdcTime_LED_5000[box]->GetMaximumBin());

    	double locFirstFiberTime = hHit_tdcTime_LED_5000[box]->GetBinCenter(hHit_tdcTime_LED_5000[box]->GetMaximumBin()) - 20;
		


//    	double locFirstFiberTime = 154.5 - 20;

//    	double locFirstFiberTime = 159;

//    	double locFirstFiberTime = 159;

        int pmtrow = locDIRCGeometry->GetPmtRow(channel);

        if(pmtrow < 6 && fabs(hit->t-locFirstFiberTime) < 5.) {
//			cout<<pmtrow<<" "<<hit->t<<endl;
			locRefTime[box] += hit->t;
			locNHits[box]++;
		} else if(pmtrow > 5 && pmtrow < 12 && fabs(hit->t-locFirstFiberTime-10) < 5.) {
			//cout<<pmtrow<<" "<<hit->t<<endl;
			locRefTime[box] += (hit->t - 10);
			locNHits[box]++;
		} else if(pmtrow > 11 && pmtrow < 18 && fabs(hit->t-locFirstFiberTime-20) < 5.) {
			//cout<<pmtrow<<" "<<hit->t<<endl;
			locRefTime[box] += (hit->t - 20);
			locNHits[box]++;
		}
    }


//    cout << "q22222:    " << locLEDRefTdcTime << "    " << locLEDRefAdcTime << endl;

	
//	cout << "!!!!! box 0 " << locRefTime[0] << "  " << locNHits[0] << endl;
//	cout << "????? box 1 " << locRefTime[1] << "  " << locNHits[1] << endl;

	locRefTime[0] /= locNHits[0];
	locRefTime[1] /= locNHits[1];


//	cout << "box 0 " << locRefTime[0] << "  " << locNHits[0] << endl;
//	cout << "box 1 " << locRefTime[1] << "  " << locNHits[1] << endl;

    hRefTime->Fill(locRefTime[0]);
    hRefTime->Fill(locRefTime[1]);

    // Fill calibrated-hit hists
    int NHits[] = {0,0};
    bool ledFiber[3] = {false, false, false};
    bool first_loop[2] = {true, true};

    for (const auto& hit : hits) {

		int box = (hit->ch < Nchannels) ? 1 : 0;
	    int channel = (hit->ch < Nchannels) ? hit->ch : (hit->ch - Nchannels);
		int pmtrow = locDIRCGeometry->GetPmtRow(channel);

		if(pmtrow < 6) ledFiber[0] = true;
		else if(pmtrow < 12) ledFiber[1] = true;
		else ledFiber[2] = true; 
		hHit_Box[loc_itrig]->Fill(box);
		NHits[box]++;
		hHit_pixelOccupancy[box][loc_itrig]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
		
		if (locDIRCLEDTrig) {

			double led3_time = hHit_tdcTime_LED_5000[box]->GetBinCenter(hHit_tdcTime_LED_5000[box]->GetMaximumBin());

	 		if (hit->t >= led3_time - 40 && hit->t <= led3_time + 20) {
			
 				hHit_pixelOccupancy_LED_timing_cut_in[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));

			} else {

 				hHit_pixelOccupancy_LED_timing_cut_out[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));

			}

		}
	
		hHit_TimeOverThreshold[box][loc_itrig]->Fill(hit->tot);
		hHit_TimeOverThresholdVsChannel[box][loc_itrig]->Fill(channel,hit->tot);
		hHit_tdcTime[box][loc_itrig]->Fill(hit->t);
		hHit_tdcTimeVsEvent[box][loc_itrig]->Fill(eventnumber,hit->t);
		hHit_tdcTimeVsChannel[box][loc_itrig]->Fill(channel,hit->t);
	
		// LED specific histograms
		if(locDIRCLEDTrig) {

// 		    vector<const DDIRCLEDRef*> dircLEDRefs;
//             eventLoop->Get(dircLEDRefs);
// 
// 	    	for(uint i=0; i<dircLEDRefs.size(); i++) {
// 
// 				const DDIRCLEDRef* dircLEDRef = (DDIRCLEDRef*)dircLEDRefs[i];
// 				locLEDRefAdcTime = dircLEDRef->t_fADC;
// 				locLEDRefTdcTime = dircLEDRef->t_TDC;
// 			}


			///*--------------------------------------------------*/
			/// Filling LED timing


//			cout << "(((((((((  " << box << "     " << locRefTime[box] << endl;

			hHit_tdcTimeDiffEvent[box]->Fill(hit->t-locRefTime[box]);
			hHit_tdcTimeDiffVsChannel[box]->Fill(channel,hit->t-locRefTime[box]);


			if(first_loop[box]){
				hHit_LEDRefTime[box]->Fill(locLEDRefTdcTime);
				first_loop[box] = false;
			}

			if(locLEDRefTdcTime != 0.0) {
				hHit_LEDRefTdcChannelTimeDiff[box]->Fill(hit->t-locLEDRefTdcTime);
			}

//			cout << box << "    " << pmtrow << endl;
//			cout << box << "    " << locRefTime[box]  << endl;

			hHit_areaTime[box][pmtrow]->Fill(hit->t-locRefTime[box]);

			hHit_pixelOccupancy_LED_column[box][pmtrow]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));

			hHit_pixelRow_timing[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), hit->t-locRefTime[box]);

			if(hit->t-locRefTime[box] >=  -5 && hit->t-locRefTime[box] <  5 ) {
				hHit_pixelOccupancy_LED_1[box]-> Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
			} else if( hit->t-locRefTime[box] >= 5 &&  hit->t-locRefTime[box] < 15) {
				hHit_pixelOccupancy_LED_2[box]-> Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));

				if( hit->t-locRefTime[box] >= 9 &&  hit->t-locRefTime[box] < 11) {
					hHit_pixelOccupancy_LED_2_narrow[box]-> Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
				}	

			} else if ( hit->t-locRefTime[box] >= 15 &&  hit->t-locRefTime[box] < 25) {
				hHit_pixelOccupancy_LED_3[box]-> Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
			}

			hHit_TimeEventMeanVsLEDRef[box]->Fill(locRefTime[box],locLEDRefTdcTime);
			hHit_TimeDiffEventMeanLEDRefVsTimestamp[box]->Fill(locReferenceClockTime, locRefTime[box]-locLEDRefTdcTime);

			/*--------------------------------------------------*/
			/// Occupancy for the fourth peak in the timing distribution 

 			if (  hit->t-locRefTime[box] >= 111.5 &&  hit->t-locRefTime[box] <= 117) {
				hHit_pixelOccupancy_LED_fourth_peak[box]->Fill(locDIRCGeometry->GetPixelRow(hit->ch), locDIRCGeometry->GetPixelColumn(hit->ch));
			}


		}




	
			if(box==1 && channel==2490) {
				hLEDRefTdcChannelTimeDiff->Fill(hit->t-locLEDRefTdcTime);
				hLEDRefTdcVsChannelTime->Fill(hit->t,locLEDRefTdcTime);
				hLEDRefAdcVsChannelTime->Fill(hit->t,locLEDRefAdcTime);
			}
		
			double locDeltaT = hit->t - locRefTime[box];
			if(ledFiber[1]) locDeltaT -= 10.;
			if(ledFiber[2]) locDeltaT -= 20.;
			if(FillTimewalk) hHit_Timewalk[box][channel]->Fill(hit->tot, locDeltaT);

		}

    hHit_NHits[loc_itrig]->Fill(NHits[0]+NHits[1]);
    hHit_NHitsVsBox[loc_itrig]->Fill(0.,NHits[0]); hHit_NHitsVsBox[loc_itrig]->Fill(1.,NHits[1]);

    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

	event_counter++;

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_timing::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_DIRC_timing::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

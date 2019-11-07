#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_online.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <DAQ/DGEMSRSWindowRawData.h>
#include <TRIGGER/DL1Trigger.h>
#include <TRD/DTRDDigiHit.h>
#include <TRD/DGEMDigiWindowRawData.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

// root hist pointers
const int NTRDplanes = 4;
const int NTRDstrips = 240;
const int NTRDwires = 24;
const int NGEMplanes = 4;
const int NGEMstrips = 256;
const int NAPVchannels = 128;
const int NGEMsamples = 21;

static TH1I *trd_num_events;
static TH1I *hDigiHit_NHits;
static TH1I *hDigiHit_QualityFactor[NTRDplanes];
static TH1I *hDigiHit_Occupancy[NTRDplanes];
static TH2I *hDigiHit_PeakVsStrip[NTRDplanes];
static TH1I *hDigiHit_PulseTime[NTRDplanes];
static TH1I *hDigiHit_Time[NTRDplanes];
static TH2I *hDigiHit_TimeVsStrip[NTRDplanes];
static TH2I *hDigiHit_TimeVsPeak[NTRDplanes];

static TH1I *hGEMHit_NHits;
static TH1I *hGEMHit_Occupancy[NGEMplanes];
static TH2I *hGEMHit_SampleVsStrip[NGEMplanes];

//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_TRD_online());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::JEventProcessor_TRD_online() {
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_online::~JEventProcessor_TRD_online() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_TRD_online::init(void) {

    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD");
    trdDir->cd();
    // book hists
    trd_num_events = new TH1I("trd_num_events","TRD number of events",1,0.5,1.5);

    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("DigiHit")->cd();
    hDigiHit_NHits = new TH1I("DigiHit_NfadcHits","TRD fADC hit multiplicity;raw hits;events",100,0.5,0.5+100);
    
    // histograms for each plane
    for(int i=0; i<NTRDplanes; i++) {
	    if(i<2) { // Wire TRD
		    hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_WirePlane%d", i),"TRD fADC hit occupancy;strip;raw hits / counter",NTRDwires,-0.5,-0.5+NTRDwires);
		    hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_WirePlane%d", i),"TRD fADC quality factor;quality factor;raw hits",4,-0.5,3.5);
		    hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_WirePlane%d", i),"TRD fADC pulse peak vs. strip;strip;pulse peak",NTRDwires,-0.5,-0.5+NTRDwires,410,0.0,4100.0);
		    hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_WirePlane%d", i),"TRD fADC pulse time;pulse time [62.5 ps];raw hits",1000,0.0,6500.0);
		    hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_WirePlane%d", i),"TRD fADC pulse time;pulse time [ns];raw hits / 2 ns",200,0.0,400.0);
		    hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_WirePlane%d", i),"TRD fADC pulse time vs. strip;strip;pulse time [ns]",NTRDwires,-0.5,-0.5+NTRDwires,200,0.0,400.0);
		    hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_WirePlane%d", i),"TRD time vs. peak;pulse peak;time [ns]",410,0.0,4100.0,200,0.0,400.0);
	    }
	    else { // GEM TRD
		    hDigiHit_Occupancy[i] = new TH1I(Form("DigiHit_Occupancy_GEMPlane%d", i),"TRD hit occupancy;strip;raw hits / counter",NTRDstrips,-0.5,-0.5+NTRDstrips);
		    hDigiHit_QualityFactor[i] = new TH1I(Form("DigiHit_QualityFactor_GEMPlane%d", i),"TRD fADC quality factor;quality factor;raw hits",4,-0.5,3.5);
		    hDigiHit_PeakVsStrip[i] = new TH2I(Form("DigiHit_PeakVsStrip_GEMPlane%d", i),"TRD fADC pulse peak vs. strip;strip;pulse peak",NTRDstrips,-0.5,-0.5+NTRDstrips,410,0.0,4100.0);
		    hDigiHit_PulseTime[i] = new TH1I(Form("DigiHit_PulseTime_GEMPlane%d", i),"TRD fADC pulse time;pulse time [62.5 ps];raw hits",1000,0.0,6500.0);
		    hDigiHit_Time[i] = new TH1I(Form("DigiHit_Time_GEMPlane%d", i),"TRD fADC pulse time;pulse time [ns];raw hits / 2 ns",200,0.0,400.0);
		    hDigiHit_TimeVsStrip[i] = new TH2I(Form("DigiHit_TimeVsStrip_GEMPlane%d", i),"TRD fADC pulse time vs. strip;strip;pulse time [ns]",NTRDstrips,-0.5,-0.5+NTRDstrips,200,0.0,400.0);
		    hDigiHit_TimeVsPeak[i] = new TH2I(Form("DigiHit_TimeVsPeak_WirePlane%d", i),"TRD time vs. peak;pulse peak;time [ns]",410,0.0,4100.0,200,0.0,400.0);
	    }
    }

    trdDir->cd();
    gDirectory->mkdir("GEMHit")->cd();
    hGEMHit_NHits = new TH1I("GEMHit_NadcHits","GEM ADC hit multiplicity;raw hits;events",10000,0.5,0.5+10000);

    // histograms for each plane
    for(int i=0; i<NGEMplanes; i++) {
	    hGEMHit_Occupancy[i] = new TH1I(Form("GEMHit_Occupancy_Plane%d", i),"GEM raw data hit occupancy;strip;raw hits / counter",NGEMstrips,-0.5,-0.5+NGEMstrips);
	    hGEMHit_SampleVsStrip[i] = new TH2I(Form("GEMHit_SampleVsStrip_Plane%d", i),"GEM ADC time samples vs strip;strip;sample",NGEMstrips,-0.5,-0.5+NGEMstrips,NGEMsamples,-0.5,-0.5+NGEMsamples);
    }
    
    // back to main dir
    mainDir->cd();

    return NOERROR;
}

//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

/*
    // Get trigger words and filter on PS trigger (if it exists?)
    const DL1Trigger *trig_words = nullptr;
    uint32_t trig_mask, fp_trig_mask;
    try {
        eventLoop->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words != nullptr) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask:trig_mask;
    // Select PS-triggered events
    if (trig_bits != 8) {
        return NOERROR;
    }
*/

    vector<const DTRDDigiHit*> digihits;
    eventLoop->Get(digihits);
    vector<const DGEMDigiWindowRawData*> windowrawdata;
    eventLoop->Get(windowrawdata);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    if (digihits.size() > 0) trd_num_events->Fill(1);
    hDigiHit_NHits->Fill(digihits.size());
    for (const auto& hit : digihits) {
	    int plane = hit->plane;
	    int strip = hit->strip;
	    hDigiHit_Occupancy[plane]->Fill(strip);
	    hDigiHit_QualityFactor[plane]->Fill(hit->QF);
	    hDigiHit_PeakVsStrip[plane]->Fill(strip,hit->pulse_peak);
	    hDigiHit_PulseTime[plane]->Fill(hit->pulse_time);
	    double t_ns = 0.0625*hit->pulse_time;
	    hDigiHit_Time[plane]->Fill(t_ns);
	    hDigiHit_TimeVsStrip[plane]->Fill(strip,t_ns);
	    hDigiHit_TimeVsPeak[plane]->Fill(hit->pulse_peak,t_ns);
    }

    if (windowrawdata.size() > 0) trd_num_events->Fill(2);
    hGEMHit_NHits->Fill(windowrawdata.size());
    for (const auto& window : windowrawdata) {
	    const DGEMSRSWindowRawData* srswindow;
	    window->GetSingle(srswindow);
	    //int apv = srswindow->apv_id;
	    //int channel = srswindow->channel_apv;

	    int plane = window->plane - 4;  //apv / 2; // 2 APVs per plane
	    int strip = window->strip;  //channel + (apv % 2) * NAPVchannels; 
	    hGEMHit_Occupancy[plane]->Fill(strip);

	    // loop over samples for each channel
	    vector<uint16_t> samples = srswindow->samples;
	    uint16_t pedestal = samples[0];
	    int max_adc_zs = 0;
	    for(uint isample=1; isample<samples.size(); isample++) {
		    int adc_zs = -1 * (samples[isample]-pedestal);
		    if(adc_zs > max_adc_zs) 
			    max_adc_zs = adc_zs;
	    }		    

	    // fill all hits in channels with a large signal
	    if(max_adc_zs > 500) {
		    for(uint isample=1; isample<samples.size(); isample++) {
			    int adc_zs = -1 * (samples[isample]-pedestal);
			    hGEMHit_SampleVsStrip[plane]->Fill(strip,isample,adc_zs);
		    }
	    }

	    // next steps: subtract pedestals, find maximum, compare neighboring strips for clusters...
    }
    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

    return NOERROR;
}
//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_online::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

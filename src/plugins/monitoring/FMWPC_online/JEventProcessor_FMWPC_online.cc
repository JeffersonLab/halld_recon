// $Id$
//
//    File: JEventProcessor_FMWPC_online.cc
// Created: Fri Oct 22 13:34:04 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_FMWPC_online.h"

#include "FMWPC/DFMWPCDigiHit.h"
#include "FMWPC/DFMWPCHit.h"
#include "FMWPC/DCTOFDigiHit.h"
#include "FMWPC/DCTOFTDCDigiHit.h"
#include "FMWPC/DCTOFHit.h"
#include <TTAB/DTTabUtilities.h>
#include <DANA/DEvent.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

using std::vector;

static TH1I *fmwpc_num_events;
static TH1F *fmwpc_occ_layer[6];
static TH1F *fmwpc_hit_layer[6];
static TH2I *fmwpc_pedestal[6];
static TH1I *h1_fmwpc_occ_chamber;
//static TH2F *h2_fmwpc_time;
//static TH2F *h2_fmwpc_q;

static TH1I *ctof_adc_events;
static TH1F *ctof_adc_occ_up;
static TH1F *ctof_adc_occ_down;
static TH1I *ctof_tdc_events;
static TH1F *ctof_tdc_occ_up;
static TH1F *ctof_tdc_occ_down;
static TH2F *ctof_tdc_time;
static TH2F *ctof_adc_time;
static TH1I *h1_ctof_occ_up;
static TH1I *h1_ctof_occ_down;
static TH2F *h2_ctof_dE;
static TH2D *h2_ctof_t;
static TH2D *h2_ctof_t_adc;
static TH2D *h2_ctof_t_adc_tdc;
static TH1F *fmwpc_time_chamber[6];
static TH1D *fmwpc_pulse_integral_chamber[6];
static TH2F *h2_fmwpc_time_chamber;
static TH2D *h2_fmwpc_pi_chamber;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_FMWPC_online());
}
} // "C"


//------------------
// JEventProcessor_FMWPC_online (Constructor)
//------------------
JEventProcessor_FMWPC_online::JEventProcessor_FMWPC_online(){
    SetTypeName("JEventProcessor_FMWPC_online");
}

//------------------
// ~JEventProcessor_FMWPC_online (Destructor)
//------------------
JEventProcessor_FMWPC_online::~JEventProcessor_FMWPC_online(){
}

//------------------
// Init
//------------------
void JEventProcessor_FMWPC_online::Init()
{
	// This is called once at program startup. 
  ctof_t_base_adc = 0.;       // ns
  ctof_t_base_tdc = 0.; // ns

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
	 
         sprintf(hname, "fmwpc_pedestal_layer_%01d", iLayer+1);
         sprintf(htitle, "FMWPC Pedestal vs. wire %01d;Wire Number;Pedestal", iLayer+1);
	 fmwpc_pedestal[iLayer] = new TH2I(hname, htitle, 144, 0.5, 144.5, 400, 0.0, 400.0);
	 fmwpc_pedestal[iLayer]->SetStats(0);
	 
	 sprintf(hname,"fmwpc_time_chamber_%01d", iLayer+1);
	 sprintf(htitle, "Chamber %01d Time (ns)", iLayer+1);
	 fmwpc_time_chamber[iLayer] = new TH1F(hname, htitle, 150,0,2000);
	 fmwpc_time_chamber[iLayer]->SetStats(0);

	 sprintf(hname,"fmwpc_pulse_integral_chamber_%01d", iLayer+1);
	 sprintf(htitle, "Chamber %01d Pulse Integral", iLayer+1);
         fmwpc_pulse_integral_chamber[iLayer] = new TH1D(hname, htitle, 100,0,2000);
	 fmwpc_pulse_integral_chamber[iLayer]->SetStats(0);
        }

	h1_fmwpc_occ_chamber = new TH1I("h1_fmwpc_occ_chamber","FMWPC Hits per Chamber",6,0,6);
	h2_fmwpc_time_chamber = new TH2F("h2_fmwpc_time_chamber","Time (ns) per Chamber",6,0,6,100,0,2000);
	h2_fmwpc_pi_chamber = new TH2D("h2_fmwpc_pi_chamber","Energy (GeV) per Chamber",6,0,6,150,0,7000);

	ctof_adc_occ_up = new TH1F("ctof_adc_occ_up", "CTOF ADC Occupancy", 4, 0.5, 4.5);
	ctof_adc_occ_up->SetXTitle("Bar Number");
	ctof_adc_occ_down = new TH1F("ctof_adc_occ_down", "CTOF ADC Occupancy", 4, 0.5, 4.5);
	ctof_adc_occ_down->SetXTitle("Bar Number");
	ctof_tdc_occ_up = new TH1F("ctof_tdc_occ_up", "CTOF TDC Occupancy", 4, 0.5, 4.5);
	ctof_tdc_occ_up->SetXTitle("Bar Number");
	ctof_tdc_occ_down = new TH1F("ctof_tdc_occ_down", "CTOF TDC Occupancy", 4, 0.5, 4.5);
	ctof_tdc_occ_down->SetXTitle("Bar Number");

	ctof_tdc_time=new TH2F("ctof_tdc_time","CTOF time from TDC;channel;t [ns]",8,-0.5,7.5,2000,-1000,1000);
	ctof_adc_time=new TH2F("ctof_adc_time","CTOF time from ADC;channel;t [ns]",8,-0.5,7.5,2000,-1000,1000);
	
	h1_ctof_occ_up = new TH1I("h1_ctof_occ_up", "CTOF Occupancy", 4, 0.5, 4.5);
	h1_ctof_occ_down = new TH1I("h1_ctof_occ_down", "CTOF Occupancy", 4, 0.5, 4.5);
	h1_ctof_occ_up->SetXTitle("Bar Number");
	h1_ctof_occ_down->SetXTitle("Bar Number");
	h2_ctof_dE = new TH2F("h1_ctof_dE", "Energy in Each BAR",4,0.5,4.5,100,0,0.003);
	h2_ctof_dE->SetXTitle("Bar Number");
	h2_ctof_dE->SetYTitle("Energy (GeV)");
	h2_ctof_t = new TH2D("h2_ctof_t", "TDC Time in Each BAR",4,0.5,4.5,150,-170,170);
	h2_ctof_t->SetXTitle("Bar Number");
	h2_ctof_t->SetYTitle("Time (ns)");
	h2_ctof_t_adc = new TH2D("h2_ctof_t_adc","ADC Time in Each BAR",4,0.5,4.5,150,-170,170);
	h2_ctof_t_adc->SetXTitle("Bar Number");
	h2_ctof_t_adc->SetYTitle("ADC Time (ns)");
	h2_ctof_t_adc_tdc = new TH2D("h2_ctof_t_adc_tdc","TDC - ADC Time in Each BAR",4,0.5,4.5,150,-170,170);
	h2_ctof_t_adc_tdc->SetXTitle("Bar Number");
	h2_ctof_t_adc_tdc->SetYTitle("ADC - TDC Time");
	

        // back to main dir
        main->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FMWPC_online::BeginRun(const std::shared_ptr<const JEvent> &event)
{
  // This is called whenever the run number changes
   // load base time offset
  map<string,double> base_time_offset;
  if ((DEvent::GetCalib(event, "/CTOF/adc_base_time_offset",base_time_offset))==false){
    ctof_t_base_adc = base_time_offset["t0"];
  }
  if ((DEvent::GetCalib(event, "/CTOF/tdc_base_time_offset",base_time_offset))==false){
    ctof_t_base_tdc = base_time_offset["t0"];
  }

  // Channel-by-channel timing offsets
  DEvent::GetCalib(event, "/CTOF/adc_timing_offsets", ctof_adc_time_offsets); 
  DEvent::GetCalib(event, "/CTOF/tdc_timing_offsets", ctof_tdc_time_offsets);


  return;
}

//------------------
// Process
//------------------
void JEventProcessor_FMWPC_online::Process(const std::shared_ptr<const JEvent> &event)
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

        const DTTabUtilities* locTTabUtilities = NULL;
        event->GetSingle(locTTabUtilities);
  
        // get wire digis
        vector<const DFMWPCDigiHit*>fmwpcdigis;
        event->Get(fmwpcdigis);

        // get calibrated hits
        vector<const DFMWPCHit*>fmwpchits;
        event->Get(fmwpchits);

        // get CTOF ADC digis
        vector<const DCTOFDigiHit*>ctofdigis;
        event->Get(ctofdigis);

        // get CTOF TDC digis
        vector<const DCTOFTDCDigiHit*>ctoftdcdigis;
        event->Get(ctoftdcdigis);
		

	vector<const DCTOFHit*>ctofHits;
	event->Get(ctofHits);
        // FILL HISTOGRAMS
        // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
        auto lockService = DEvent::GetLockService(event);
		lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

        fmwpc_num_events->Fill(1);

        for (unsigned int i=0; i<fmwpcdigis.size();i++){
         const DFMWPCDigiHit *digi = fmwpcdigis[i];
	 fmwpc_time_chamber[digi->layer-1]->Fill(digi->pulse_time);
	 fmwpc_pulse_integral_chamber[digi->layer-1]->Fill(digi->pulse_integral);
	 h2_fmwpc_time_chamber->Fill(digi->layer-1, digi->pulse_time);
	 h2_fmwpc_pi_chamber->Fill(digi->layer-1, digi->pulse_integral);
         fmwpc_occ_layer[digi->layer-1]->Fill(digi->wire);
	 fmwpc_pedestal[digi->layer-1]->Fill(digi->wire, digi->pedestal);
        }

        for (unsigned int i=0; i<fmwpchits.size();i++){
         const DFMWPCHit *hit = fmwpchits[i];
         fmwpc_hit_layer[hit->layer-1]->Fill(hit->wire);
	 //h2_fmwpc_time->Fill(hit->layer-1, hit->t);
	 //h2_fmwpc_q->Fill(hit->layer-1, hit->q);
	 if(hit->layer == 1){
	   h1_fmwpc_occ_chamber->Fill(0);
	 }
	 if(hit->layer == 2){
           h1_fmwpc_occ_chamber->Fill(1);
	 }
	 if(hit->layer == 3){
           h1_fmwpc_occ_chamber->Fill(2);
	 }
	 if(hit->layer == 4){
           h1_fmwpc_occ_chamber->Fill(3);
	 }
	 if(hit->layer == 5){
           h1_fmwpc_occ_chamber->Fill(4);
	 }
	 if(hit->layer == 6){
           h1_fmwpc_occ_chamber->Fill(5);
	 }
        }

        ctof_adc_events->Fill(1);

        for (unsigned int i=0; i<ctofdigis.size();i++){
         const DCTOFDigiHit *digi = ctofdigis[i];
	 if (digi->end == 0)
	   ctof_adc_occ_up->Fill(digi->bar);
	 else if (digi->end == 1)
	   ctof_adc_occ_down->Fill(digi->bar);
	 
	 // Time in ns  
	 int ind=digi->bar-1 + 4.*digi->end;
	 const double t_scale=0.0625;
	 double T = t_scale*double(digi->pulse_time) + ctof_adc_time_offsets[ind] + ctof_t_base_adc;
	 ctof_adc_time->Fill(ind,T);
	}

        ctof_tdc_events->Fill(1);

        for (unsigned int i=0; i<ctoftdcdigis.size();i++){
         const DCTOFTDCDigiHit *digi = ctoftdcdigis[i];
	 if (digi->end == 0)
	   ctof_tdc_occ_up->Fill(digi->bar);
	 else if (digi->end == 1)
	   ctof_tdc_occ_down->Fill(digi->bar);

	 // Time in ns  
	 int ind=digi->bar-1 + 4.*digi->end;
	 double T = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(digi);
	 T += ctof_t_base_tdc + ctof_tdc_time_offsets[ind];

	 ctof_tdc_time->Fill(ind,T);
	}
	//For factory hits
	for(unsigned int j = 0; j < ctofHits.size(); j++){
	  const DCTOFHit *hit = ctofHits[j];
	  h2_ctof_dE->Fill(hit->bar,hit->dE);
	  h2_ctof_t->Fill(hit->bar,hit->t);
	  h2_ctof_t_adc->Fill(hit->bar,hit->t_adc);
	  h2_ctof_t_adc_tdc->Fill(hit->bar,hit->t_adc - hit->t);
	  if(hit->end == 0){
	    h1_ctof_occ_up->Fill(hit->bar);
	  }
	  if(hit->end == 1){
	    h1_ctof_occ_down->Fill(hit->bar);
	  }
	}

	lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

	return;
}

//------------------
// EndRun
//------------------
void JEventProcessor_FMWPC_online::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FMWPC_online::Finish()
{
	// Called before program exit after event processing is finished.
}


// $Id$
//
//    File: JEventProcessor_CDC_online.cc
// Created: Wed Oct 22 2014
// Creator: Naomi Jarvis


#include <stdint.h>
#include <vector>

#include <TMath.h>


#include "JEventProcessor_CDC_online.h"
#include <JANA/JApplication.h>


using namespace std;


#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125PulseIntegral.h"
#include "DAQ/Df125PulsePedestal.h"
#include "DAQ/Df125WindowRawData.h"
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"
#include "TRIGGER/DTrigger.h"
#include "DANA/DEvent.h"

#include <JANA/Calibrations/JCalibration.h>

#include <TDirectory.h>
#include <TH2.h>
#include <TH1.h>


// root hist pointers

static TH1I *cdc_num_events = NULL;

static TH2I *cdc_o = NULL; 
static TH2D *cdc_occ_ring[29];

static TH1I *cdc_raw_amp = NULL;
static TH2I *cdc_raw_amp_vs_n = NULL;

static TH1I *cdc_raw_t;
static TH2I *cdc_raw_t_vs_n;

static TH1I *cdc_raw_intpp;  //raw integral including pedestal
static TH2I *cdc_raw_intpp_vs_n;  

static TH1I *cdc_raw_int;  //raw integral minus pedestal  
static TH2I *cdc_raw_int_vs_n;  

static TH1I *cdc_ped = NULL;  
static TH2I *cdc_ped_vs_n = NULL; 

static TH1I *cdc_windata_ped = NULL;  
static TH2I *cdc_windata_ped_vs_n = NULL; 




//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_CDC_online());
  }
}


//----------------------------------------------------------------------------------


JEventProcessor_CDC_online::JEventProcessor_CDC_online() {
	SetTypeName("JEventProcessor_CDC_online");
	initialized_histograms = false;
}


//----------------------------------------------------------------------------------


JEventProcessor_CDC_online::~JEventProcessor_CDC_online() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_CDC_online::Init() {

  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  // I moved all the histogram setup into the brun so that I can use different
  // scales for the later runs using the new firmware


  WG_OCC = false;
  app->SetDefaultParameter("CDC_ONLINE:WG_OCC", WG_OCC, "Fill occupancy only if wire_gain > 0");

  // create root folder for cdc and cd to it, store main dir
  TDirectory *main = gDirectory;
  gDirectory->mkdir("CDC")->cd();


  cdc_num_events = new TH1I("cdc_num_events","CDC number of events",1, 0.5, 1.5);

  cdc_o = new TH2I("cdc_o","CDC occupancy by straw, ring;straw;ring",209,0.5,209.5,28,0.5,28.5);


  gDirectory->mkdir("rings_occupancy","CDC rings: occupancy")->cd();


  // Hit occupancy
  int Nstraws[28] = {42, 42, 54, 54, 66, 66, 80, 80, 93, 93, 106, 106, 123, 123, 135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};
  double radius[28] = {10.72134, 12.08024, 13.7795, 15.14602, 18.71726, 20.2438, 22.01672, 23.50008, 25.15616, 26.61158, 28.33624, 29.77388, 31.3817, 32.75838, 34.43478, 35.81146, 38.28542, 39.7002, 41.31564, 42.73042, 44.34078, 45.75302, 47.36084, 48.77054, 50.37582, 51.76012, 53.36286, 54.74716};
  double phi[28] = {0, 0.074707844, 0.038166294, 0.096247609, 0.05966371, 0.012001551, 0.040721951, 0.001334527, 0.014963808, 0.048683644, 0.002092645, 0.031681749, 0.040719354, 0.015197341, 0.006786058, 0.030005892, 0.019704045, -0.001782064, -0.001306618, 0.018592421, 0.003686784, 0.022132975, 0.019600866, 0.002343723, 0.021301449, 0.005348855, 0.005997358, 0.021018761}; 

  // Define a different 2D histogram for each ring. X-axis is phi, Y-axis is radius (to plot correctly with "pol" option)
  for(int iring=0; iring<28; iring++){
	double r_start = radius[iring] - 0.8;
	double r_end = radius[iring] + 0.8;
	double phi_start = phi[iring]; // this is for center of straw. Need additional calculation for phi at end plate
	double phi_end = phi_start + TMath::TwoPi();
		
	char hname[256];
	sprintf(hname, "cdc_occ_ring[%d]", iring+1);
	cdc_occ_ring[iring+1] = new TH2D(hname, "", Nstraws[iring], phi_start, phi_end, 1, r_start, r_end);
  }
  
  
  // back to main dir
  main->cd();
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
  // This is called whenever the run number changes

  auto runnumber = event->GetRunNumber();
  JCalibration *jcalib = DEvent::GetJCalibration(event);
   
  // max values for histogram scales, modified fa250-format readout


    Int_t AMAX = 4096;    //max for amplitude, fa250-format, 12 bits
  
    //  Int_t IMAX = 524288;  //max for raw integral, fa250-format, 19 bits
    Int_t IMAX = 400000;  //max for raw integral, fa250-format, 19 bits

    Int_t PMAX = 512;     //max for pedestal, fa250-format max is 512
    //  Int_t RTMAX = 32768;   //max for raw time, fa250-format, 15 bits
    Int_t RTMAX = 12000; //max for raw time, less than full field width

    Char_t rtunits[8] = "0.125ns";  //raw time is in units of sample/64 = ns/8



  if (runnumber > 3675) { //new fa125 format firmware, from 11 Sept 2015

 
    // raw quantities for read out (125 format) are
    //   time                    field max 2047   scaled x 1, units 0.8ns
    //   time qf                 field max 1 
    //   overflow count          field max 7
    //   pedestal                field max 255    scaled x 1/1 initially
    //   max amplitude 9 bits,   field max 511    scaled x 1/8
    //   integral                field max 16383  scaled x 1/16

    // max values for histogram scales, fa125-format readout

    IMAX = 16384; //max for raw integral, fa125-format, 14 bits
    PMAX = 256;   //max for pedestal, fa125-format, 8 bits
    RTMAX = 2048;  //max for raw time, fa125-format, 11 bits
    AMAX = 512;    //max for amplitude, fa125-format, 9 bits

    sprintf(rtunits,"0.8ns");  //raw time is in units of sample/10 = 0.8ns

  }


  const Int_t NSTRAWS = 3522;
  const Float_t HALF = 0.5;
  const Float_t NSTRAWSPH = 3522.5;

  lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!

  if(initialized_histograms) //don't init twice!
  {
	  lockService->RootUnLock(); //RELEASE ROOT LOCK
	  return;
  }

  gDirectory->cd("CDC");

  // book histograms

  cdc_raw_amp   = new TH1I("cdc_raw_amp","CDC amplitude (ADC units, scaled); ADC units",AMAX,0,AMAX);

  cdc_raw_amp_vs_n   = new TH2I("cdc_raw_amp_vs_n","CDC amplitude (ADC units, scaled) vs straw number;straw;ADC units",NSTRAWS,HALF,NSTRAWSPH,128,0,AMAX);

  cdc_raw_t = new TH1I("cdc_raw_t",Form("CDC raw time (units of %s); raw time (%s)",rtunits,rtunits),200,0,RTMAX);

  cdc_raw_t_vs_n = new TH2I("cdc_raw_t_vs_n",Form("CDC raw time (units of %s) vs straw number;straw;time (%s)",rtunits,rtunits),NSTRAWS,HALF,NSTRAWSPH,100,0,RTMAX);

  cdc_raw_int   = new TH1I("cdc_raw_int","CDC integral (ADC units, scaled), pedestal subtracted; ADC units",200,0,IMAX);

  cdc_raw_int_vs_n   = new TH2I("cdc_raw_int_vs_n","CDC integral (ADC units,scaled), pedestal subtracted, vs straw number;straw;ADC units",NSTRAWS,HALF,NSTRAWSPH,100,0,IMAX);

  cdc_raw_intpp   = new TH1I("cdc_raw_intpp","CDC integral (ADC units, scaled), includes pedestal; ADC units",200,0,IMAX);

  cdc_raw_intpp_vs_n   = new TH2I("cdc_raw_intpp_vs_n","CDC integral (ADC units, scaled), including pedestal, vs straw number;straw;ADC units",NSTRAWS,HALF,NSTRAWSPH,100,0,IMAX);

  cdc_ped   = new TH1I("cdc_ped","CDC pedestal (ADC units);pedestal (ADC units)",(Int_t)(PMAX/2),0,PMAX);

  cdc_ped_vs_n   = new TH2I("cdc_ped_vs_n","CDC pedestal (ADC units) vs straw number;straw;pedestal (ADC units)",NSTRAWS,HALF,NSTRAWSPH,(Int_t)(PMAX/4),0,PMAX);

  cdc_windata_ped   = new TH1I("cdc_windata_ped","CDC pedestal (ADC units) from raw window data;pedestal (ADC units)",(Int_t)(PMAX/2),0,PMAX);

  cdc_windata_ped_vs_n   = new TH2I("cdc_windata_ped_vs_n","CDC pedestal (ADC units) from raw window data vs straw number;straw;pedestal (ADC units)",NSTRAWS,HALF,NSTRAWSPH,(Int_t)(PMAX/4),0,PMAX);

  gDirectory->cd(".."); //RETURN TO MAIN FOLDER

  initialized_histograms = true;

  lockService->RootUnLock(); //RELEASE ROOT LOCK

    unsigned int numstraws[28]={42,42,54,54,66,66,80,80,93,93,106,106,123,123,
        135,135,146,146,158,158,170,170,182,182,197,197,
        209,209};


    //    cout << "WG_OCC: " << WG_OCC << endl; 
    
   // Get the wire_gain parameters from the database
   vector< map<string, double> > tvals;
   wire_gain.clear();

   unsigned int straw_count=0,ring_count=0;
   if (jcalib->Get("CDC/wire_gains", tvals)==false){
      vector<double>temp;
      for(unsigned int i=0; i<tvals.size(); i++){
         map<string, double> &row = tvals[i];

         temp.push_back(row["Acorr"]);
	 //	 if (i<15) cout << i << " " << row["Acorr"] << endl;
         straw_count++;
         if (straw_count==numstraws[ring_count]){
            wire_gain.push_back(temp);
            temp.clear();
            straw_count=0;
            ring_count++;
         }
      }
   }

   //   for (uint i=0; i<100; i++) cout << "wire_gain[" << i << "] " << wire_gain[0][i] << endl;
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_online::Process(const std::shared_ptr<const JEvent>& event) {
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop-Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.


  uint32_t tr,p,a; // dcdcdigihits raw quantities: time, pedestal, amplitude, quality factor, overflow count
  uint32_t integral; // dcdcdigihits integral, includes pedestal
  uint32_t integ;    // dcdcdigihits integral minus pedestal

  uint16_t ring,straw; // ring and straw numbers from either dcdchits or dcdcdigihits
  uint16_t n;         // straw number, 1 to 3522

  uint32_t total_ped;  //total pedestal during integration period

  Bool_t PED_SUB;  // if this is false, integration window info is missing, so don't plot integrals


  uint32_t nsamples_integral=0;    ///< number of samples used in integral 
  uint32_t nsamples_pedestal=0;    ///< number of samples used in pedestal

  const uint16_t NPEDSAMPLES=16;

//  Bool_t FoundRawData=kFALSE;   //set true if found window raw data, present in mode 8 and raw mode

  //add extra 0 at front to use offset[1] for ring 1
  int straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};


  const DTrigger* locTrigger = NULL; 
  event->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
    return;
  if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
    return;
  }


  // get raw data for cdc
  vector<const DCDCDigiHit*> digihits;
  event->Get(digihits);

  //get WRD data for new format (until it is linked to CDCPulse)
  vector<const Df125WindowRawData*> wrdvector;
  event->Get(wrdvector);



  // FILL HISTOGRAMS
  // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
  lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

  if(digihits.size() > 0)
	  cdc_num_events->Fill(1);

  for(uint32_t i=0; i<digihits.size(); i++) {

    const DCDCDigiHit *digihit = digihits[i];  

    // Get pointers to the underlying objects of interest
    const Df125PulseIntegral *pi = NULL;
    const Df125PulsePedestal *pp = NULL;
    const Df125WindowRawData *windat = NULL;
    const Df125CDCPulse *cp = NULL;
    const Df125Config *cf = NULL;

    vector<uint16_t> samples;
    uint32_t winped=0;
    


    PED_SUB = kFALSE;  //set this to true when we find the config params
    total_ped = 0;
    a = 0;

    //get raw window data via pulse integral
    digihit->GetSingle(pi);
    if (pi) {
      pi->GetSingle(windat);

      nsamples_integral = pi ? pi->nsamples_integral : 0;
      nsamples_pedestal = pi ? pi->nsamples_pedestal : 0;

      if ((nsamples_integral > 0) && (nsamples_pedestal > 0)) PED_SUB = kTRUE;

    } else if (i < (uint32_t)wrdvector.size()) { 
      windat = wrdvector[i];
    }
      
    //get amplitude from pulse peak in pulse pedestal
    digihit->GetSingle(pp);
    if (pp) a = pp->pulse_peak;

    //get amplitude from CDCPulseData for new firmware
    digihit->GetSingle(cp);
    if (cp) a = cp->first_max_amp;

    //get IE from Df125Config when available
    digihit->GetSingle(cf);
    if (cf) nsamples_integral = cf->IE - (int)(0.1*digihit->pulse_time);


    ring     = digihit->ring;
    straw    = digihit->straw;
    n = straw_offset[ring] + straw;

    if ((digihit->pulse_integral > 0)||(digihit->pulse_time > 0)) {

      p        = digihit->pedestal;   
      tr       = digihit->pulse_time;    // raw time in 0.8 ns units
      integral = digihit->pulse_integral; // pulse integral in fadc units, pedestal not subtracted


      // If WG_OCC is set true, then only fill the occupancy histos if the straw's wire_gain > 0
      // If it is set false, then fill the occupancy histo anyway
      
      bool live_straw = 1;
      
      if (WG_OCC && (wire_gain[ring-1][straw-1] <= 0)) live_straw=0;

      //      if (n<30) cout << " straw " << n << " live:" << live_straw << endl;
      
      if (live_straw) {

          cdc_o->Fill(straw,ring);  

   	  Double_t w = cdc_occ_ring[ring]->GetBinContent(straw, 1) + 1.0;
          cdc_occ_ring[ring]->SetBinContent(straw, 1, w);
      }

      integ = 0;

      //ok to use p for pedestal subtraction here because if fa250 algo fails with p=0, integral=0 and amplitude=0 also

      if (PED_SUB) {
        total_ped = p*nsamples_integral/nsamples_pedestal;
        integ = integral - total_ped;
      }

      if (tr>0) {
        cdc_raw_t->Fill(tr);
        cdc_raw_t_vs_n->Fill(n,tr);
      }


      if (PED_SUB && (integ>0)) {
        cdc_raw_int->Fill(integ); 
        cdc_raw_int_vs_n->Fill(n,integ); 
      }

      if (integral>0) {
        cdc_raw_intpp->Fill(integral); 
        cdc_raw_intpp_vs_n->Fill(n,integral); 
      }

      if (p > 0) {
        cdc_ped->Fill(p); 
        cdc_ped_vs_n->Fill(n,p);     
      }

      if (a > 0) {
        //a = a - p;   //not subtracting pedestal as scaling factors may differ
        cdc_raw_amp->Fill(a);
        cdc_raw_amp_vs_n->Fill(n,a); 
      }


    }      

    // get raw window data for cdc

    if (windat) {

      if (windat->samples.size()>=NPEDSAMPLES) {

//        FoundRawData = kTRUE;

        winped = 0;

        for (uint16_t i=0; i<NPEDSAMPLES; i++) winped += (uint32_t)windat->samples[i]; 
          
        winped = (uint32_t)winped/16.0;

        if (winped > 0) {
          cdc_windata_ped->Fill(winped); 
          cdc_windata_ped_vs_n->Fill(n,winped);     
        }

      }//sample size
    } //windat

  } //end of loop through digihits

	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_online::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_CDC_online::Finish() {
  // Called before program exit after event processing is finished.

}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

// $Id$
//
//    File: JEventProcessor_PSC_TW.cc
// Created: Fri Aug 21 10:42:28 EDT 2015
// Creator: aebarnes (on Linux ifarm1102 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_PSC_TW.h"

// ROOT header files
#include <TH2.h>
#include <TDirectory.h>
// RF header files
#include <RF/DRFTime_factory.h>
// PSC header files
#include <PAIR_SPECTROMETER/DPSCPair.h>

// Define constants
const uint32_t NMODULES = 8;

const double TMIN = -5;
const double TMAX = 5;
const double TBINS = (TMAX - TMIN)/0.1;

const double PMIN = 0;
const double PMAX = 2000;
const double PBINS = (PMAX - PMIN)/2;

// Define histograms
static TH2F* h_dt_vs_pp_tdc_l[NMODULES];
static TH2F* h_dt_vs_pp_tdc_r[NMODULES];
static TH2F* h_dt_vs_pp_t_l[NMODULES];
static TH2F* h_dt_vs_pp_t_r[NMODULES];

// Define variables
Int_t psc_mod_l;
Int_t psc_mod_r;
Double_t pp_l;
Double_t pp_r;
Double_t adc_l;
Double_t adc_r;
Double_t tdc_l;
Double_t tdc_r;
Double_t t_l;
Double_t t_r;
Double_t rf_l;
Double_t rf_r;

// Define RFTime_factory
DRFTime_factory* locRFTimeFactory;

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_PSC_TW());
}
} // "C"


//------------------
// JEventProcessor_PSC_TW (Constructor)
//------------------
JEventProcessor_PSC_TW::JEventProcessor_PSC_TW()
{
	SetTypeName("JEventProcessor_PSC_TW");
}

//------------------
// ~JEventProcessor_PSC_TW (Destructor)
//------------------
JEventProcessor_PSC_TW::~JEventProcessor_PSC_TW()
{

}

//------------------
// Init //------------------
void JEventProcessor_PSC_TW::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();
	//

   TDirectory *main = gDirectory;
   TDirectory *pscDir = gDirectory->mkdir("PSC_TW");
   pscDir->cd();

   gDirectory->mkdir("tdc-rf")->cd();
   for (uint32_t i = 0; i < NMODULES; ++i)
   {
      h_dt_vs_pp_tdc_l[i] = new TH2F(Form("h_dt_vs_pp_tdc_l_%i",i+1),
                                     Form("#Delta t vs. pulse peak, raw TDC, left PSC %i;\
                                           Pulse peak; #Delta t (raw TDC - RF)",i+1),
                                     PBINS, PMIN, PMAX, TBINS, TMIN, TMAX);
      h_dt_vs_pp_tdc_r[i] = new TH2F(Form("h_dt_vs_pp_tdc_r_%i",i+1),
                                     Form("#Delta t vs. pulse peak, raw TDC, left PSC %i;\
                                           Pulse peak; #Delta t (raw TDC - RF)",i+1),
                                     PBINS, PMIN, PMAX, TBINS, TMIN, TMAX);
   }
   pscDir->cd();

   gDirectory->mkdir("t-rf")->cd();
   for (uint32_t i = 0; i < NMODULES; ++i)
   {
      h_dt_vs_pp_t_l[i] = new TH2F(Form("h_dt_vs_pp_t_l_%i",i+1),
                                   Form("#Delta t vs. pulse peak, corrected TDC, left PSC %i;\
                                         Pulse Peak; #Delta t (raw TDC - RF)",i+1),
                                   PBINS, PMIN, PMAX, TBINS, TMIN, TMAX);
      h_dt_vs_pp_t_r[i] = new TH2F(Form("h_dt_vs_pp_t_r_%i",i+1),
                                   Form("#Delta t vs. pulse peak, corrected TDC, left PSC %i;\
                                         Pulse peak; #Delta t (raw TDC - RF)",i+1),
                                   PBINS, PMIN, PMAX, TBINS, TMIN, TMAX);
   }
   main->cd();

   return NOERROR;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_PSC_TW::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  
  //////////////
  // Initialize RF time factory
  locRFTimeFactory = static_cast<DRFTime_factory*>(event->GetFactory("DRFTime", ""));
  
  // be sure that DRFTime_factory::init() and brun() are called
  vector<const DRFTime*> locRFTimes;
  event->Get(locRFTimes);
}

//------------------
// Process
//------------------
void JEventProcessor_PSC_TW::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// event->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// event->Get(mydataclasses);
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();


   vector<const DRFTime*>	locRFTimes;
   vector<const DPSCPair*>	pairs;

   event->Get(pairs);
   event->Get(locRFTimes,"PSC");

   const DRFTime* locRFTime = NULL;

   if (locRFTimes.size() > 0)
      locRFTime = locRFTimes[0];
   else
      return;


   // Since we are filling histograms local to this plugin, 
   // it will not interfere with other ROOT operations:
   // can use plugin-wide ROOT fill lock
   lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

   for (uint32_t i = 0; i < pairs.size(); ++i)
   {
      // Left modules
      psc_mod_l = pairs[i]->ee.first->module;
      pp_l = pairs[i]->ee.first->pulse_peak;
      adc_l = pairs[i]->ee.first->time_fadc;
      tdc_l = pairs[i]->ee.first->time_tdc;
      t_l = pairs[i]->ee.first->t;
      // Right modules
      psc_mod_r = pairs[i]->ee.second->module;
      pp_r = pairs[i]->ee.second->pulse_peak;
      adc_r = pairs[i]->ee.second->time_fadc;
      tdc_r = pairs[i]->ee.second->time_tdc;
      t_r = pairs[i]->ee.second->t;

      // Use the ADC time to find the closest RF time.
      rf_l = locRFTimeFactory->Step_TimeToNearInputTime(locRFTime->dTime,adc_l);
      rf_r = locRFTimeFactory->Step_TimeToNearInputTime(locRFTime->dTime,adc_r);

      h_dt_vs_pp_tdc_l[psc_mod_l - 1]->Fill(pp_l, tdc_l - rf_l);
      h_dt_vs_pp_t_l[psc_mod_l - 1]->Fill(pp_l, t_l - rf_l);
      h_dt_vs_pp_tdc_r[psc_mod_r - 1]->Fill(pp_r, tdc_r - rf_r);
      h_dt_vs_pp_t_r[psc_mod_r - 1]->Fill(pp_r, t_r - rf_r);
   }

   lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_PSC_TW::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_PSC_TW::Finish()
{
	// Called before program exit after event processing is finished.
}

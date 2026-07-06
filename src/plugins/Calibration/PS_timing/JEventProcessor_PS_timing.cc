// $Id$
//
//    File: JEventProcessor_PS_timing.cc
// Created: Sat Nov 21 17:21:28 EST 2015
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.el7.x86_64 x86_64)
// A.S. Select RF closest to the FADC hit, 1/13/2022

#include "JEventProcessor_PS_timing.h"

using namespace std;

#include "TTAB/DTTabUtilities.h"
//#include <JANA/Calibrations/JCalibration.h>

#include <TRIGGER/DL1Trigger.h>

#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>
#include <PAIR_SPECTROMETER/DPSHit.h>

#include <TAGGER/DTAGHHit.h>
#include <RF/DRFTime.h>

// RF header files
#include <RF/DRFTime_factory.h>

// Routine used to create our JEventProcessor
#include <JANA/JFactoryT.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

const int NC_PSC = 16; // number of PSC modules (counters)
const int NC_PS = 290; // number of PS columns (tiles)
const int NC_TAGH = 274; // number of TAGH counter slots

static TH2I *hPSC_tdcadcTimeDiffVsID;
static TH2I *hPSCRF_tdcTimeDiffVsID;
static TH2I *hPSRF_adcTimeDiffVsID;
static TH2I *hTAGHRF_tdcTimeDiffVsID;

static TH2I *pscADC[2];
static TH2I *pscTDC[2];
static TH2I *psADC[2];
static TH2I *pscHITtiming[2];
static TH2I *psHITtiming[2];

// Define RFTime_factory
DRFTime_factory* locRFTimeFactory;

extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_PS_timing());
    }
} // "C"


//------------------
// JEventProcessor_PS_timing (Constructor)
//------------------
JEventProcessor_PS_timing::JEventProcessor_PS_timing()
{
    SetTypeName("JEventProcessor_PS_timing");
}

//------------------
// ~JEventProcessor_PS_timing (Destructor)
//------------------
JEventProcessor_PS_timing::~JEventProcessor_PS_timing()
{

}

//------------------
// Init
//------------------
void JEventProcessor_PS_timing::Init()
{
    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

    // This is called once at program startup. If you are creating
    // and filling historgrams in this plugin, you should lock the
    // ROOT mutex like this:

    const double Tl = -200.0;
    const double Th = 600.0;
    const int NTb = 8000;
    TDirectory *mainDir = gDirectory;
    TDirectory *psDir = gDirectory->mkdir("PS_timing");
    psDir->cd();
    hPSC_tdcadcTimeDiffVsID = new TH2I("PSC_tdcadcTimeDiffVsID","PSC TDC-ADC time difference vs. counter ID;counter ID;time(TDC) - time(ADC) [ns]",NC_PSC,0.5,0.5+NC_PSC,NTb,Tl,Th);
    hPSCRF_tdcTimeDiffVsID = new TH2I("PSCRF_tdcTimeDiffVsID","PSC-RF TDC time difference vs. counter ID;counter ID;time(TDC) - time(RF) [ns]",NC_PSC,0.5,0.5+NC_PSC,NTb,Tl,Th);
    hPSRF_adcTimeDiffVsID = new TH2I("PSRF_adcTimeDiffVsID","PS-RF ADC time difference vs. counter ID;counter ID;time(ADC) - time(RF) [ns]",NC_PS,0.5,0.5+NC_PS,NTb,Tl,Th);
    hTAGHRF_tdcTimeDiffVsID = new TH2I("TAGHRF_tdcTimeDiffVsID","TAGH-RF TDC time difference vs. counter ID;counter ID;time(TDC) - time(RF) [ns]",NC_TAGH,0.5,0.5+NC_TAGH,NTb,Tl,Th);

    // new timing histos
    pscADC[0] = new TH2I("pscADC0", "RAWDATA PSC ID vs PSC ADC time  NORTH; ADC time [ns]; PSC ID",  1000, 30., 400., 10, -0.5, 9.5);
    pscADC[1] = new TH2I("pscADC1", "RAWDATA PSC ID vs PSC ADC time  SOUTH; ADC time [ns]; PSC ID",  1000, 30., 400., 10, -0.5, 9.5);
    
    pscTDC[0] = new TH2I("pscTDC0", "RAWDATA PSC ID vs PSC TDC time  NORTH; TDC time [ns]; PSC ID",  1000, 30., 700., 10, -0.5, 9.5);
    pscTDC[1] = new TH2I("pscTDC1", "RAWDATA PSC ID vs PSC TDC time  SOUTH; TDC time [ns]; PSC ID",  1000, 30., 700., 10, -0.5, 9.5);
    
    psADC[0] = new TH2I("psADC0", "RAWDATA PSC ID vs PS ADC time  NORTH; ADC time [ns]; PS ID",  1000, 30., 300., 151, -0.5, 150.5);
    psADC[1] = new TH2I("psADC1", "RAWDATA PSC ID vs PS ADC time  SOUTH; ADC time [ns]; PS ID",  1000, 30., 300., 151, -0.5, 150.5);
    
    pscHITtiming[0] = new TH2I("pscHITtiming0","PSC HIT ID vs PSC HITtime-RFtime NORTH; PSC HITtime-RFtime [ns]; PSC ID", 2000, -100., 0., 10, -0.5, 9.5); 
    pscHITtiming[1] = new TH2I("pscHITtiming1","PSC HIT ID vs PSC HITtime-RFtime SOUTH; PSC HITtime-RFtime [ns]; PSC ID", 2000, -100., 0., 10, -0.5, 9.5); 
    
    psHITtiming[0] = new TH2I("psHITtiming0","PS HIT ID vs PSC HITtime-RFtime NORTH; PS HITtime-RFtime [ns]; PS ID", 1000, -100., 0., 150, -0.5, 149.5); 
    psHITtiming[1] = new TH2I("psHITtiming1","PS HIT ID vs PSC HITtime-RFtime SOUTH; PS HITtime-RFtime [ns]; PS ID", 1000, -100., 0., 150, -0.5, 149.5); 

    mainDir->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_PS_timing::BeginRun(const std::shared_ptr<const JEvent>& event)
{

  locRFTimeFactory = static_cast<DRFTime_factory*>(event->GetFactory("DRFTime", ""));
 
  // be sure that DRFTime_factory::init() and brun() are called
  vector<const DRFTime*> rfTimes;
  event->Get(rfTimes);

    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_PS_timing::Process(const std::shared_ptr<const JEvent>& event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

  vector <const DL1Trigger*> trig;
  event->Get(trig);
  if (trig.size() == 0){
    //cout<<"no trigger found"<<endl;
    return;     
  }
  if (!(trig[0]->trig_mask & 0x8)){ //
    //cout<<"no PS TRIGGER"<<endl;
    return;
  }
  
  vector<const DPSCPair*> cpairs;
  event->Get(cpairs);
  vector<const DPSPair*> fpairs;
  event->Get(fpairs);
  
  vector<const DTAGHHit*> taghhits;
  event->Get(taghhits, "Calib");
  
  const DRFTime* rfTime = nullptr;
  vector <const DRFTime*> rfTimes;
  event->Get(rfTimes);
  if (rfTimes.size() > 0)
    rfTime = rfTimes[0];
  else
    return;
  
  
  
  vector <const DPSCDigiHit*> psc_adchits;
  vector <const DPSCTDCDigiHit*> psc_tdchits;
  vector <const DPSCHit*> psc_hits;
  
  vector <const DPSDigiHit*> ps_adchits;
  vector <const DPSHit*> ps_hits;
  
  event->Get(psc_adchits);
  event->Get(psc_tdchits);
  event->Get(psc_hits);
  event->Get(ps_adchits);
  event->Get(ps_hits);

  const DTTabUtilities* locTTabUtilities = nullptr;
  event->GetSingle(locTTabUtilities);
  
  // FILL HISTOGRAMS
  // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
  lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  
  double t_RF = rfTime->dTime;
  if (cpairs.size() >= 1) { // PSC
    
    const DPSCHit* clhit = cpairs[0]->ee.first; // left hit in coarse PS
    const DPSCHit* crhit = cpairs[0]->ee.second;// right hit in coarse PS
    
    double psc_rf_left  = locRFTimeFactory->Step_TimeToNearInputTime(t_RF, clhit->time_fadc);
    double psc_rf_right = locRFTimeFactory->Step_TimeToNearInputTime(t_RF, crhit->time_fadc);
    
    hPSC_tdcadcTimeDiffVsID->Fill(clhit->module,clhit->t-clhit->time_fadc);
    
    hPSCRF_tdcTimeDiffVsID->Fill(clhit->module, clhit->t - psc_rf_left );
    
    hPSC_tdcadcTimeDiffVsID->Fill(crhit->module+8,crhit->t-crhit->time_fadc);
    
    hPSCRF_tdcTimeDiffVsID->Fill(crhit->module+8,crhit->t-psc_rf_right);
    
    for (const auto& h : taghhits) hTAGHRF_tdcTimeDiffVsID->Fill(h->counter_id,h->t-t_RF);
    
    if (fpairs.size() >= 1) { // PS
      
      const DPSPair::PSClust* flhit = fpairs[0]->ee.first;  // left hit in fine PS
      const DPSPair::PSClust* frhit = fpairs[0]->ee.second; // right hit in fine PS
      
      double ps_rf_left  = locRFTimeFactory->Step_TimeToNearInputTime(t_RF, flhit->t_tile);
      double ps_rf_right = locRFTimeFactory->Step_TimeToNearInputTime(t_RF, frhit->t_tile);
      
      hPSRF_adcTimeDiffVsID->Fill(flhit->column,flhit->t_tile - ps_rf_left);
      hPSRF_adcTimeDiffVsID->Fill(frhit->column+145,frhit->t_tile - ps_rf_right);
    }
  }

  // PSC and PS timing w.r.t. RF
  
  for (unsigned int k=0; k<psc_adchits.size(); k++){
    const DPSCDigiHit *hit = psc_adchits[k];
    int ID = hit->counter_id;
    int ARM = 0;
    if (ID>8){
      ID -= 8;
      ARM = 1;
    }
    pscADC[ARM]->Fill(hit->pulse_time*0.0625, ID);
  }
  for (unsigned int k=0; k<psc_tdchits.size(); k++){
    const DPSCTDCDigiHit *hit = psc_tdchits[k];
    int ID = hit->counter_id;
    int ARM = 0;
    if (ID>8){
      ID -= 8;
      ARM = 1;
    }
    double Ttdc = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(hit);
    pscTDC[ARM]->Fill(Ttdc, ID);
  }
  
  for (unsigned int k=0; k<ps_adchits.size(); k++){
    const DPSDigiHit *hit = ps_adchits[k];
    int ID = hit->column;
    int ARM = hit->arm;
    psADC[ARM]->Fill(hit->pulse_time*0.0625, ID);
  }
  
  for (unsigned int k=0; k<psc_hits.size(); k++){
    const DPSCHit *hit = psc_hits[k];
    int ID = hit->module;
    int ARM = hit->arm;
    pscHITtiming[ARM]->Fill(hit->t-t_RF, ID);
  }
  
  for (unsigned int k=0; k<ps_hits.size(); k++){
    const DPSHit *hit = ps_hits[k];
    int ID = hit->column;
    int ARM = hit->arm;
    psHITtiming[ARM]->Fill(hit->t-t_RF, ID);
  }
    
  lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_PS_timing::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_PS_timing::Finish()
{
    // Called before program exit after event processing is finished.
}


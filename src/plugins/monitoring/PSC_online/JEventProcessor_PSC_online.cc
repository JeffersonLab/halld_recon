// $Id$
//
//    File: JEventProcessor_PSC_online.cc
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)


#include <stdint.h>
#include <vector>

#include "JEventProcessor_PSC_online.h"
#include <JANA/JApplication.h>

using namespace std;

#include "TTAB/DTTabUtilities.h"
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

// number of PSC modules (counters) per arm
const int Nmods = 8; 
const int Narms = 2;

const int NmultBins = 10; //number of bins for multiplicity histograms

const double counts_cut = 100.0;
// root hist pointers
static TH1I *psc_num_events;
static TH1I *hHit_NHits;
static TH1I *hHit_Arm;
static TH2I *hHit_NHitsVsArm;
static TH2F *hHit_HasTDCvsHasADC[Narms];
static TH1I *hHit_Occupancy[Narms];
static TH1I *hHit_Integral[Narms];
static TH2I *hHit_IntegralVsModule[Narms];
//static TH1I *hHit_Npe[Narms];
static TH1I *hHit_fadcTime[Narms];
static TH2I *hHit_fadcTimeVsModule[Narms];
static TH1I *hHit_Time[Narms];
static TH2I *hHit_TimeVsModule[Narms];
static TH1I *hHit_tdcTime[Narms];
static TH2I *hHit_tdcadcTimeDiffVsModule[Narms];

static TH1I *hDigiHit_NfadcHits;
static TH1I *hDigiHit_Arm;
static TH2I *hDigiHit_NfadcHitsVsArm;
static TH1I *hDigiHit_NSamplesPedestal[Narms];
static TH1I *hDigiHit_Pedestal[Narms];
static TProfile *hDigiHit_PedestalVsModule[Narms];
static TH1I *hDigiHit_QualityFactor[Narms];
static TH1I *hDigiHit_fadcOccupancy[Narms];
static TH1I *hDigiHit_RawPeak[Narms];
static TH2I *hDigiHit_RawPeakVsModule[Narms];
static TH1I *hDigiHit_RawIntegral[Narms];
static TH2I *hDigiHit_RawIntegralVsModule[Narms];
static TH1I *hDigiHit_NSamplesIntegral[Narms];
static TH2I *hDigiHit_PeakVsModule[Narms];
static TH2I *hDigiHit_IntegralVsPeak[Narms];
static TH2I *hDigiHit_IntegralVsModule[Narms];
static TH1I *hDigiHit_PulseTime[Narms];
static TH1I *hDigiHit_fadcTime[Narms];
static TH2I *hDigiHit_fadcTimeVsModule[Narms];
static TH2I *hDigiHit_fadcTimeVsIntegral[Narms];

static TH1I *hDigiHit_NfadcHits_cut;
static TH1I *hDigiHit_Arm_cut;
static TH2I *hDigiHit_NfadcHitsVsArm_cut;
static TH1I *hDigiHit_fadcOccupancy_cut[Narms];
static TH1I *hDigiHit_fadcTime_cut[Narms];
static TH2I *hDigiHit_fadcTimeVsModule_cut[Narms];
static TH2I *hDigiHit_fadcTimeVsQF_cut[Narms];

static TH1I *hDigiHit_NtdcHits;
static TH1I *hDigiHit_tdcOccupancy[Narms];
static TH1I *hDigiHit_tdcRawTime[Narms];
static TH1I *hDigiHit_tdcTime[Narms];
static TH2I *hDigiHit_tdcTimeVsModule[Narms];
static TH2I *hDigiHit_tdcTimeVsfadcTime[Narms];
static TH1I *hDigiHit_tdcadcTimeDiff[Narms];
static TH2I *hDigiHit_tdcadcTimeDiffVsModule[Narms];
static TH2I *hDigiHit_tdcadcTimeDiffVsIntegral[Narms];
static TH2I *hDigiHit_adctdcMatchesVsModule[Narms];
//----------------------------------------------------------------------------------

// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_PSC_online());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_PSC_online::JEventProcessor_PSC_online() {
  SetTypeName("JEventProcessor_PSC_online");
}


//----------------------------------------------------------------------------------


JEventProcessor_PSC_online::~JEventProcessor_PSC_online() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_PSC_online::Init() {

    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

    // create root folder for psc and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *pscDir = gDirectory->mkdir("PSC");
    pscDir->cd();

    // book hists
    psc_num_events = new TH1I("psc_num_events","PSC Number of events",1,0.5,1.5);
    // fADC250 hit-level hists (after calibration)	  
    TDirectory *hitDir = gDirectory->mkdir("Hit"); hitDir->cd();
    hHit_NHits = new TH1I("Hit_NHits","PSC hit multiplicity;hits;events",NmultBins,0.5,0.5+NmultBins);
    hHit_Arm = new TH1I("Hit_Arm","PSC arm;arm;hits",Narms,-0.5,-0.5+Narms);
    hHit_NHitsVsArm    = new TH2I("Hit_NHitsVsArm","PSC hit multiplicity vs. arm;arm;hits",Narms,-0.5,-0.5+Narms,NmultBins,0.5,0.5+NmultBins);
    TString arm_str[] = {"Left","Right"};
    for (int i = 0; i < Narms; i++) {
        gDirectory->mkdir(arm_str[i]+"Arm")->cd();
        TString strN = "_" + arm_str[i] + "Arm";
        TString strT = ", " + arm_str[i] + " arm";
        hHit_HasTDCvsHasADC[i] = new TH2F("Hit_HasTDCvsHasADC"+strN,"PSC has TDC? vs. has ADC?"+strT+";fADC status;TDC status",2,-0.5,1.5,2,-0.5,1.5);
        hHit_Occupancy[i]    = new TH1I("Hit_Occupancy"+strN,"PSC occupancy"+strT+";module (counter);hits / module",Nmods,0.5,0.5+Nmods);
        hHit_Integral[i]    = new TH1I("Hit_Integral"+strN,"PSC fADC pulse integral"+strT+";pulse integral;hits",250,0.0,10000.0);
        hHit_IntegralVsModule[i]    = new TH2I("Hit_IntegralVsModule"+strN,"PSC fADC pulse integral vs. module"+strT+";module (counter);pulse integral",Nmods,0.5,0.5+Nmods,250,0.0,10000.0);
        //hHit_Npe[i]    = new TH1I("Hit_Npe"+strN,"PSC fADC number of pixels"+strT+";pixels;hits",200,0,100);
        hHit_fadcTime[i]    = new TH1I("Hit_fadcTime"+strN,"PSC fADC time"+strT+";time [ns];hits / 400 ps",200,-40.0,40.0);
        hHit_fadcTimeVsModule[i]    = new TH2I("Hit_fadcTimeVsModule"+strN,"PSC fADC time vs. module"+strT+";module (counter);time [ns]",Nmods,0.5,0.5+Nmods,200,-40.0,40.0);
        hHit_Time[i]    = new TH1I("Hit_Time"+strN,"PSC time"+strT+";time [ns];hits / 400 ps",200,-40.0,40.0);
        hHit_TimeVsModule[i]    = new TH2I("Hit_TimeVsModule"+strN,"PSC time vs. module"+strT+";module (counter);time [ns]",Nmods,0.5,0.5+Nmods,200,-40.0,40.0);
        hHit_tdcTime[i]    = new TH1I("Hit_tdcTime"+strN,"PSC TDC time"+strT+";time [ns];hits / 400 ps",200,-40.0,40.0);
        hHit_tdcadcTimeDiffVsModule[i]    = new TH2I("Hit_tdcadcTimeDiffVsModule"+strN,"PSC TDC/ADC time difference vs. module"+strT+";module (counter);time(TDC) - time(ADC) [ns]",Nmods,0.5,0.5+Nmods,200,-40.0,40.0);
        hitDir->cd();
    }
    // fADC250 and F1TDC digihit-level hists
    pscDir->cd();
    TDirectory *digihitDir = gDirectory->mkdir("DigiHit"); digihitDir->cd();
    hDigiHit_NfadcHits = new TH1I("DigiHit_NfadcHits","PSC fADC hit multiplicity;raw hits;events",NmultBins,0.5,0.5+NmultBins);
    hDigiHit_Arm = new TH1I("DigiHit_Arm","PSC arm;arm;raw hits",Narms,-0.5,-0.5+Narms);
    hDigiHit_NfadcHitsVsArm    = new TH2I("DigiHit_NfadcHitsVsArm","PSC fADC hit multiplicity vs. arm;arm;raw hits",Narms,-0.5,-0.5+Narms,NmultBins,0.5,0.5+NmultBins);
    hDigiHit_NtdcHits = new TH1I("DigiHit_NtdcHits","PSC TDC hit multiplicity;raw hits;events",NmultBins,0.5,0.5+NmultBins);
    hDigiHit_NfadcHits_cut = new TH1I("DigiHit_NfadcHits_cut","PSC fADC hit multiplicity (> 100 ADC integral counts);raw hits;events",NmultBins,0.5,0.5+NmultBins);
    hDigiHit_Arm_cut = new TH1I("DigiHit_Arm_cut","PSC arm (> 100 ADC integral counts);arm;raw hits",Narms,-0.5,-0.5+Narms);
    hDigiHit_NfadcHitsVsArm_cut    = new TH2I("DigiHit_NfadcHitsVsArm_cut","PSC fADC hit multiplicity vs. arm (> 100 ADC integral counts);arm;raw hits",Narms,-0.5,-0.5+Narms,NmultBins,0.5,0.5+NmultBins);
    for (int i = 0; i < Narms; i++) {
        gDirectory->mkdir(arm_str[i]+"Arm")->cd();
        TString strN = "_" + arm_str[i] + "Arm";
        TString strT = ", " + arm_str[i] + " arm";
        hDigiHit_NSamplesPedestal[i]    = new TH1I("DigiHit_NSamplesPedestal"+strN,"PSC fADC pedestal samples"+strT+";pedestal samples;raw hits",50,-0.5,49.5);
        hDigiHit_Pedestal[i] = new TH1I("DigiHit_Pedestal"+strN,"PSC fADC pedestals"+strT+";pedestal [fADC counts];raw hits",200,0.0,200.0);
        hDigiHit_PedestalVsModule[i] = new TProfile("DigiHit_PedestalVsModule"+strN,"PSC pedestal vs. module"+strT+";module (counter);average pedestal [fADC counts]",Nmods,0.5,0.5+Nmods,"s"); 
        hDigiHit_QualityFactor[i] = new TH1I("DigiHit_QualityFactor"+strN,"PSC fADC quality factor"+strT+";quality factor;raw hits",4,-0.5,3.5);
        hDigiHit_fadcOccupancy[i]    = new TH1I("DigiHit_fadcOccupancy"+strN,"PSC fADC hit occupancy"+strT+";module (counter);raw hits / module",Nmods,0.5,0.5+Nmods);
        hDigiHit_RawPeak[i]    = new TH1I("DigiHit_RawPeak"+strN,"PSC fADC pulse peak (raw)"+strT+";pulse peak (raw);raw hits",410,0.0,4100.0);
        hDigiHit_RawPeakVsModule[i]    = new TH2I("DigiHit_RawPeakVsModule"+strN,"PSC fADC pulse peak (raw) vs. module"+strT+";module (counter);pulse peak (raw)",Nmods,0.5,0.5+Nmods,410,0.0,4100.0);
        hDigiHit_RawIntegral[i]    = new TH1I("DigiHit_RawIntegral"+strN,"PSC fADC pulse integral (raw)"+strT+";pulse integral (raw);raw hits",500,0.0,20000.0);
        hDigiHit_RawIntegralVsModule[i]    = new TH2I("DigiHit_RawIntegralVsModule"+strN,"PSC fADC pulse integral (raw) vs. module"+strT+";module (counter);pulse integral (raw)",Nmods,0.5,0.5+Nmods,500,0.0,20000.0);
        hDigiHit_NSamplesIntegral[i]    = new TH1I("DigiHit_NSamplesIntegral"+strN,"PSC fADC integral samples"+strT+";integral samples;raw hits",60,-0.5,59.5);
        hDigiHit_PeakVsModule[i]    = new TH2I("DigiHit_PeakVsModule"+strN,"PSC fADC pulse peak vs. module"+strT+";module (counter);pulse peak",Nmods,0.5,0.5+Nmods,410,0.0,4100.0);
        hDigiHit_IntegralVsPeak[i]    = new TH2I("DigiHit_IntegralVsPeak"+strN,"PSC fADC pulse integral vs. peak"+strT+";pulse peak;pulse integral",410,0.0,4100.0,250,0.0,10000.0);
        hDigiHit_IntegralVsModule[i]    = new TH2I("DigiHit_IntegralVsModule"+strN,"PSC fADC pulse integral vs. module"+strT+";module (counter);pulse integral",Nmods,0.5,0.5+Nmods,250,0.0,10000.0);
        hDigiHit_PulseTime[i]    = new TH1I("DigiHit_PulseTime"+strN,"PSC fADC pulse time"+strT+";pulse time [62.5 ps];raw hits",2000,0.0,6500.0);
        hDigiHit_fadcTime[i]    = new TH1I("DigiHit_fadcTime"+strN,"PSC fADC pulse time"+strT+";pulse time [ns];raw hits / 400 ps",1000,0.0,400.0);
        hDigiHit_fadcTimeVsModule[i]    = new TH2I("DigiHit_fadcTimeVsModule"+strN,"PSC fADC pulse time vs. module"+strT+";module (counter);pulse time [ns]",Nmods,0.5,0.5+Nmods,1000,0.0,400.0);
        hDigiHit_fadcTimeVsIntegral[i]    = new TH2I("DigiHit_fadcTimeVsIntegral"+strN,"PSC fADC pulse time vs. integral"+strT+";pulse integral;pulse time [ns]",250,0.0,10000.0,1000,0.0,400.0);
        // digihit-level hists after cut on ADC integral counts
        hDigiHit_fadcOccupancy_cut[i]    = new TH1I("DigiHit_fadcOccupancy_cut"+strN,"PSC fADC hit occupancy (> 100 ADC integral counts)"+strT+";module (counter);raw hits / module",Nmods,0.5,0.5+Nmods);
        hDigiHit_fadcTime_cut[i]    = new TH1I("DigiHit_fadcTime_cut"+strN,"PSC fADC pulse time (> 100 ADC integral counts)"+strT+";pulse time [ns];raw hits / 400 ps",1000,0.0,400.0);
        hDigiHit_fadcTimeVsModule_cut[i]    = new TH2I("DigiHit_fadcTimeVsModule_cut"+strN,"PSC fADC pulse time vs. module (> 100 ADC integral counts)"+strT+";module (counter);pulse time [ns]",Nmods,0.5,0.5+Nmods,1000,0.0,400.0);
        hDigiHit_fadcTimeVsQF_cut[i]    = new TH2I("DigiHit_fadcTimeVsQF_cut"+strN,"PSC fADC pulse time vs. quality factor (> 100 ADC integral counts)"+strT+";fADC quality factor;pulse time [ns]",4,-0.5,3.5,1000,0.0,400.0);
        // TDC hists
        hDigiHit_tdcOccupancy[i]    = new TH1I("DigiHit_tdcOccupancy"+strN,"PSC TDC hit occupancy"+strT+";module (counter);raw hits / module",Nmods,0.5,0.5+Nmods);
        hDigiHit_tdcRawTime[i]    = new TH1I("DigiHit_tdcRawTime"+strN,"PSC TDC raw time"+strT+";time [60 ps];raw hits",1000,0.0,65500.0);
        hDigiHit_tdcTime[i]    = new TH1I("DigiHit_tdcTime"+strN,"PSC TDC time"+strT+";time [ns];raw hits / 250 ps",3200,0.0,800.0);
        hDigiHit_tdcTimeVsModule[i]    = new TH2I("DigiHit_tdcTimeVsModule"+strN,"PSC TDC time"+strT+";module (counter);TDC time [ns]",Nmods,0.5,0.5+Nmods,3200,0.0,800.0);
        hDigiHit_tdcTimeVsfadcTime[i]    = new TH2I("DigiHit_tdcTimeVsfadcTime"+strN,"PSC TDC time vs. ADC time"+strT+";fADC time [ns];TDC time [ns]",400,0.0,400.0,800,0.0,800.0);
        hDigiHit_tdcadcTimeDiff[i]    = new TH1I("DigiHit_tdcadcTimeDiff"+strN,"PSC TDC/ADC time difference"+strT+";time(TDC) - time(ADC) [ns];hits",1000,0.0,400.0);
        hDigiHit_tdcadcTimeDiffVsModule[i]    = new TH2I("DigiHit_tdcadcTimeDiffVsModule"+strN,"PSC TDC/ADC time difference vs. module"+strT+";module (counter);time(TDC) - time(ADC) [ns]",Nmods,0.5,0.5+Nmods,1000,0.0,400.0);
        hDigiHit_tdcadcTimeDiffVsIntegral[i]    = new TH2I("DigiHit_tdcadcTimeDiffVsIntegral"+strN,"PSC TDC/ADC time difference vs. pulse integral"+strT+";pulse integral;time(TDC) - time(ADC) [ns]",500,0.0,15000.0,1000,0.0,400.0);
        hDigiHit_adctdcMatchesVsModule[i] = new TH2I("DigiHit_adctdcMatchesVsModule"+strN,"PSC #TDC matches / fADC hit vs. module"+strT+";module (counter);#TDC matches / fADC hit",Nmods,0.5,0.5+Nmods,5,-0.5,4.5);
        digihitDir->cd();
    }
    // back to main dir
    mainDir->cd();
}


//----------------------------------------------------------------------------------


void JEventProcessor_PSC_online::BeginRun(const std::shared_ptr<const JEvent>& event) {
    // This is called whenever the run number changes
}


//----------------------------------------------------------------------------------


void JEventProcessor_PSC_online::Process(const std::shared_ptr<const JEvent>& event) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

    // Get data for PSC
    vector<const DPSCDigiHit*> digihits;
    event->Get(digihits);
    vector<const DPSCTDCDigiHit*> tdcdigihits;
    event->Get(tdcdigihits);
    vector<const DPSCHit*> hits;
    event->Get(hits);

    const DTTabUtilities* ttabUtilities = nullptr;
    event->GetSingle(ttabUtilities);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

    if (digihits.size() > 0 || tdcdigihits.size() > 0) psc_num_events->Fill(1);

    // Fill digihit hists
    int NDigiHits[] = {0,0};
    int NDigiHits_cut[] = {0,0};
    hDigiHit_NfadcHits->Fill(digihits.size());
    hDigiHit_NtdcHits->Fill(tdcdigihits.size());
    for (const auto& hit : digihits) {
        int arm = (hit->counter_id <= Nmods) ? 0 : 1;
        int module = (hit->counter_id <= Nmods) ? hit->counter_id : (hit->counter_id - Nmods);
        double ped = (double)hit->pedestal/hit->nsamples_pedestal;
        hDigiHit_NSamplesPedestal[arm]->Fill(hit->nsamples_pedestal);
        hDigiHit_Pedestal[arm]->Fill(ped);
        hDigiHit_RawPeak[arm]->Fill(hit->pulse_peak);
        if (ped == 0.0 || hit->pulse_peak == 0) continue;
        NDigiHits[arm]++;
        hDigiHit_Arm->Fill(arm);
        hDigiHit_PedestalVsModule[arm]->Fill(module,ped);
        hDigiHit_fadcOccupancy[arm]->Fill(module);
        hDigiHit_RawPeakVsModule[arm]->Fill(module,hit->pulse_peak);
        hDigiHit_RawIntegral[arm]->Fill(hit->pulse_integral);
        hDigiHit_RawIntegralVsModule[arm]->Fill(module,hit->pulse_integral);
        hDigiHit_NSamplesIntegral[arm]->Fill(hit->nsamples_integral);
        hDigiHit_PeakVsModule[arm]->Fill(module,hit->pulse_peak-ped);
        double pI = hit->pulse_integral - hit->nsamples_integral*ped;
        hDigiHit_IntegralVsModule[arm]->Fill(module,pI);
        hDigiHit_IntegralVsPeak[arm]->Fill(hit->pulse_peak-ped,pI);
        hDigiHit_PulseTime[arm]->Fill(hit->pulse_time);
        double t_ns = 0.0625*hit->pulse_time;
        hDigiHit_fadcTime[arm]->Fill(t_ns);
        hDigiHit_fadcTimeVsModule[arm]->Fill(module,t_ns);
        hDigiHit_fadcTimeVsIntegral[arm]->Fill(pI,t_ns);
        hDigiHit_QualityFactor[arm]->Fill(hit->QF);
        if (pI > counts_cut)  { 
            NDigiHits_cut[arm]++;
            hDigiHit_Arm_cut->Fill(arm);
            hDigiHit_fadcOccupancy_cut[arm]->Fill(module);
            hDigiHit_fadcTime_cut[arm]->Fill(t_ns);
            hDigiHit_fadcTimeVsModule_cut[arm]->Fill(module,t_ns);
            hDigiHit_fadcTimeVsQF_cut[arm]->Fill(hit->QF,t_ns);
        }
    }
    hDigiHit_NfadcHitsVsArm->Fill(0.,NDigiHits[0]); hDigiHit_NfadcHitsVsArm->Fill(1.,NDigiHits[1]);
    hDigiHit_NfadcHitsVsArm_cut->Fill(0.,NDigiHits_cut[0]); hDigiHit_NfadcHitsVsArm_cut->Fill(1.,NDigiHits_cut[1]);
    hDigiHit_NfadcHits_cut->Fill(NDigiHits_cut[0]+NDigiHits_cut[1]);

    for (const auto& hit : digihits) {
        double ped = (double)hit->pedestal/hit->nsamples_pedestal;
        double pI = hit->pulse_integral - hit->nsamples_integral*ped;
        if (hit->pedestal > 0 && pI > counts_cut) {
            int arm = (hit->counter_id <= Nmods) ? 0 : 1;
            int module = (hit->counter_id <= Nmods) ? hit->counter_id : (hit->counter_id - Nmods);
            int matches = 0;
            for (const auto& tdchit : tdcdigihits) {
                if (hit->counter_id == tdchit->counter_id) {
                    matches++;
                    double T_tdc = ttabUtilities->Convert_DigiTimeToNs_F1TDC(tdchit);
                    double T_adc = 0.0625*hit->pulse_time;
                    hDigiHit_tdcTimeVsfadcTime[arm]->Fill(T_adc,T_tdc);
                    hDigiHit_tdcadcTimeDiff[arm]->Fill(T_tdc-T_adc);
                    hDigiHit_tdcadcTimeDiffVsModule[arm]->Fill(module,T_tdc-T_adc);
                    hDigiHit_tdcadcTimeDiffVsIntegral[arm]->Fill(pI,T_tdc-T_adc);
                }
            }
            hDigiHit_adctdcMatchesVsModule[arm]->Fill(module,matches);
        }
    }

    for (const auto& hit : tdcdigihits) {
        int arm = (hit->counter_id <= Nmods) ? 0 : 1;
        int module = (hit->counter_id <= Nmods) ? hit->counter_id : (hit->counter_id - Nmods);
        hDigiHit_tdcOccupancy[arm]->Fill(module);
        hDigiHit_tdcRawTime[arm]->Fill(hit->time);
        double T_tdc = ttabUtilities->Convert_DigiTimeToNs_F1TDC(hit);
        hDigiHit_tdcTime[arm]->Fill(T_tdc);
        hDigiHit_tdcTimeVsModule[arm]->Fill(module,T_tdc);
    }

    // Fill calibrated-hit hists
    int NHits[] = {0,0};
    for (const auto& hit : hits) {
        int arm = hit->arm;
        hHit_HasTDCvsHasADC[arm]->Fill(hit->has_fADC,hit->has_TDC);
        if (hit->has_fADC) {
            hHit_Arm->Fill(arm);
            NHits[arm]++;
            hHit_Occupancy[arm]->Fill(hit->module);
            hHit_Integral[arm]->Fill(hit->integral);
            hHit_IntegralVsModule[arm]->Fill(hit->module,hit->integral);
            //hHit_Npe[arm]->Fill(hit->npe_fadc);
            hHit_fadcTime[arm]->Fill(hit->time_fadc);
            hHit_fadcTimeVsModule[arm]->Fill(hit->module,hit->time_fadc);
            hHit_Time[arm]->Fill(hit->t);
            hHit_TimeVsModule[arm]->Fill(hit->module,hit->t);
            if (hit->has_TDC) {
                hHit_tdcTime[arm]->Fill(hit->time_tdc);
                hHit_tdcadcTimeDiffVsModule[arm]->Fill(hit->module,hit->time_tdc-hit->time_fadc);
            }
        }
    }
    hHit_NHits->Fill(NHits[0]+NHits[1]);
    hHit_NHitsVsArm->Fill(0.,NHits[0]); hHit_NHitsVsArm->Fill(1.,NHits[1]);

    lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

}


//----------------------------------------------------------------------------------


void JEventProcessor_PSC_online::EndRun() {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_PSC_online::Finish() {
    // Called before program exit after event processing is finished.
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

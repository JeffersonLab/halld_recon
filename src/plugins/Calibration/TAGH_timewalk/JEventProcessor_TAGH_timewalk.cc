// $Id$
//
//    File: JEventProcessor_TAGH_timewalk.cc
// Created: Fri Nov 13 10:13:02 EST 2015
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-229.20.1.el7.x86_64 x86_64)
// A.S. 1/13/2022 add reference time determined by the PSC

#include "JEventProcessor_TAGH_timewalk.h"
using namespace std;

#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHGeometry.h>

#include <RF/DRFTime.h>

#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>

// Routine used to create our JEventProcessor
#include <JANA/JFactoryT.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

const int Nslots = 274; // number of TAGH counter slots
static TH2I *hTAGH_tdcadcTimeDiffVsSlotID;
static TH2I *hTAGHRF_tdcTimeDiffVsSlotID;
static TH2I *hTAGHRF_tdcTimeDiffVsPulseHeight[1+Nslots];
static TH2I *hTAGHRF_correctedTdcTimeDiffVsPulseHeight[1+Nslots];
static TH2I *htagh_rf_psc;

// Define RFTime_factory
// DRFTime_factory* dRFTimeFactory;


extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_TAGH_timewalk());
    }
} // "C"


//------------------
// JEventProcessor_TAGH_timewalk (Constructor)
//------------------
JEventProcessor_TAGH_timewalk::JEventProcessor_TAGH_timewalk()
{
	SetTypeName("JEventProcessor_TAGH_timewalk");
}

//------------------
// ~JEventProcessor_TAGH_timewalk (Destructor)
//------------------
JEventProcessor_TAGH_timewalk::~JEventProcessor_TAGH_timewalk()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TAGH_timewalk::Init()
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
    TDirectory *taghDir = gDirectory->mkdir("TAGH_timewalk");
    taghDir->cd();
    gDirectory->mkdir("Offsets")->cd();


    htagh_rf_psc = new TH2I("tagh_time_rf", "tagh_time_rf",  300, -0.5, 299.5,1600,-50.,50.);

    hTAGH_tdcadcTimeDiffVsSlotID = new TH2I("TAGH_tdcadcTimeDiffVsSlotID","TAGH TDC-ADC time difference vs. counter ID;counter ID;time(TDC) - time(ADC) [ns]",Nslots,0.5,0.5+Nslots,NTb,Tl,Th);
    hTAGHRF_tdcTimeDiffVsSlotID = new TH2I("TAGHRF_tdcTimeDiffVsSlotID","TAGH-RF TDC time difference vs. counter ID;counter ID;time(TDC) - time(RF) [ns]",Nslots,0.5,0.5+Nslots,NTb,Tl,Th);
    taghDir->cd();
    gDirectory->mkdir("Timewalk")->cd();
    for (int i = 0; i < 1+Nslots; i++) {
        stringstream ss; ss << i;
        TString name = "TAGHRF_tdcTimeDiffVsPulseHeight_" + ss.str();
        TString title = "TAGH counter " + ss.str();
        hTAGHRF_tdcTimeDiffVsPulseHeight[i] = new TH2I(name,title+";pulse height [fADC counts];time(TDC) - time(RF) [ns]",41,0,4100,50,-2.5,2.5);
    }
    taghDir->cd();
    gDirectory->mkdir("Corrected_Timewalk")->cd();
    for (int i = 0; i < 1+Nslots; i++) {
        stringstream ss; ss << i;
        TString name = "TAGHRF_correctedTdcTimeDiffVsPulseHeight_" + ss.str();
        TString title = "TAGH counter " + ss.str();
        hTAGHRF_correctedTdcTimeDiffVsPulseHeight[i] = new TH2I(name,title+";pulse height [fADC counts];time(TDC) - time(RF) [ns]",41,0,4100,50,-2.5,2.5);
    }
    mainDir->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TAGH_timewalk::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
    dRFTimeFactory = static_cast<DRFTime_factory*>(event->GetFactory("DRFTime", ""));
    vector<const DRFTime*> locRFTimes;
    event->Get(locRFTimes);
}

//------------------
// Process
//------------------
void JEventProcessor_TAGH_timewalk::Process(const std::shared_ptr<const JEvent>& event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

  vector<const DTAGHGeometry*> taghGeomVect;

  event->Get( taghGeomVect );
  if (taghGeomVect.size() < 1){
    cout << "  taghGeomVect not found " << endl;
    return; // OBJECT_NOT_AVAILABLE;
  }
    
  const DTAGHGeometry&  taghGeom =  *(taghGeomVect[0]);
    

    vector<const DTAGHHit*> taghhits;
    event->Get(taghhits, "Calib");
    
    vector<const DPSCPair*>   psc_pairs;
    event->Get(psc_pairs);
    
    vector<const DPSPair*>   ps_pairs;
    event->Get(ps_pairs);

//    RFs for PS and PSC
//  Use two separate RFs

    const DRFTime* rfTime = nullptr;
    vector <const DRFTime*> rfTimes;

    const DRFTime* rfTimePSC = nullptr;
    vector <const DRFTime*> rfTimesPSC;


    event->Get(rfTimesPSC,"PSC");

    if (rfTimesPSC.size() > 0)
      rfTimePSC = rfTimesPSC[0];
    else
      return; // NOERROR;


    event->Get(rfTimes,"TAGH");

    if (rfTimes.size() > 0)
        rfTime = rfTimes[0];
    else
        return;

    


    double t_rf_psc     =  0.;
    int    psc_found    =  0.;

    if((ps_pairs.size() > 0) && (psc_pairs.size() > 0)){

      double psc_time  = (psc_pairs[0]->ee.first->t  +  psc_pairs[0]->ee.second->t)/2.;
      t_rf_psc = dRFTimeFactory->Step_TimeToNearInputTime(rfTimePSC->dTime, psc_time);      

      psc_found = 1;
    }





    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    double t_RF = rfTime->dTime;

    for (unsigned int i = 0; i < taghhits.size(); i++) {

        const DTAGHHit *hit = taghhits[i];

        if (!hit->has_TDC || !hit->has_fADC) continue;

        int id = hit->counter_id;
        double t = hit->t;
        double t_tdc = hit->time_tdc;
        double t_adc = hit->time_fadc;
        double pulse_height = hit->pulse_peak;

        hTAGH_tdcadcTimeDiffVsSlotID->Fill(id,t_tdc-t_adc);
        hTAGHRF_tdcTimeDiffVsSlotID->Fill(id,t_tdc-t_RF);

        t_tdc = dRFTimeFactory->Step_TimeToNearInputTime(t_tdc,t_RF);
        t = dRFTimeFactory->Step_TimeToNearInputTime(t,t_RF);

        hTAGHRF_tdcTimeDiffVsPulseHeight[0]->Fill(pulse_height,t_tdc-t_RF);
        hTAGHRF_tdcTimeDiffVsPulseHeight[id]->Fill(pulse_height,t_tdc-t_RF);
        hTAGHRF_correctedTdcTimeDiffVsPulseHeight[id]->Fill(pulse_height,t-t_RF);

	if(psc_found){

	  double Elow  = taghGeom.getElow(id);
	  double Ehigh = taghGeom.getEhigh(id);
	  double tagh_energy = (Elow + Ehigh)/2;

	  double pair_energy = ps_pairs[0]->ee.first->E + ps_pairs[0]->ee.second->E;

	  if(fabs(pair_energy - tagh_energy) < 0.2)

	  htagh_rf_psc->Fill(id, hit->t - t_rf_psc);

	}

    }

    lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_TAGH_timewalk::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TAGH_timewalk::Finish()
{
    // Called before program exit after event processing is finished.
}

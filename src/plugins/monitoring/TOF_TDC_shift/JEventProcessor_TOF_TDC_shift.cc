#include <stdint.h>
#include <iomanip>
#include <vector>
#include <TMath.h>
#include <TCutG.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include "JEventProcessor_TOF_TDC_shift.h"
#include <JANA/JApplication.h>
using namespace std;

#include "TOF/DTOFHit.h"
#include "TOF/DTOFDigiHit.h"
#include "TOF/DTOFTDCDigiHit.h"
#include "TTAB/DTranslationTable.h"

#include "DAQ/DCODAROCInfo.h"

// Define some constants
const float_t   ADC_BIN = 0.0625; // fADC250 pulse time resolution (ns)
const float_t   TDC_BIN = 0.0234375; // CAEN TDC time resolution (ns)

//----------------------------------------------------------------------------------
// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TOF_TDC_shift());
  }
}
//----------------------------------------------------------------------------------
JEventProcessor_TOF_TDC_shift::JEventProcessor_TOF_TDC_shift() {
	SetTypeName("JEventProcessor_TOF_TDC_shift");
}
//----------------------------------------------------------------------------------
JEventProcessor_TOF_TDC_shift::~JEventProcessor_TOF_TDC_shift() {
}
//----------------------------------------------------------------------------------

void JEventProcessor_TOF_TDC_shift::Init() {
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  // Create root folder for ST and cd to it, store main dir
  TDirectory *main = gDirectory;
  filedir = main;
  gDirectory->mkdir("TOF_TDC_shift")->cd();

  // TI remainder vs (ADC time - TDC time)
  hrocTimeRemainder_AdcTdcTimeDiff = new TH2I("hrocTimeRemainder_AdcTdcTimeDiff",";t_{ADC} - t_{TDC};TI % 6",4000,-1500,500,6,-0.5,5.5);
  hrocTimeRemainder_AdcTdcTimeDiff_corrected = new TH2I("hrocTimeRemainder_AdcTdcTimeDiff_corrected",";t_{ADC} - t_{TDC};TI % 6",600,-30,30,6,-0.5,5.5);

  // cd back to main directory
  main->cd();
}
//----------------------------------------------------------------------------------
void JEventProcessor_TOF_TDC_shift::BeginRun(const std::shared_ptr<const JEvent>& event) {
  auto runnumber = event->GetRunNumber();
  char filename[200];
  sprintf(filename,"TOF_TDC_shift_%6.6d.txt",runnumber);
  OUTPUT.open(filename);
  // OUTPUT << setw(6) << runnumber;

  // This is called whenever the run number changes
}
//----------------------------------------------------------------------------------
void JEventProcessor_TOF_TDC_shift::Process(const std::shared_ptr<const JEvent>& event) {

  // Get all data objects first so we minimize the time we hold the ROOT mutex lock

  // Each detector's hits
  vector<const DTOFHit*>            dtofhits;            // TOF Hits
  vector<const DTOFDigiHit*>        dtofdigihits;        // TOF DigiHits
  vector<const DTOFTDCDigiHit*>     dtoftdcdigihits;     // TOF TDC DigiHits
  vector<const DCODAROCInfo*>       dcodarocinfo;        // DCODAROCInfo

  // TOF
  event->Get(dtofhits);
  event->Get(dtofdigihits);
  event->Get(dtoftdcdigihits);
  event->Get(dcodarocinfo);

  Int_t TriggerBIT = -1;
  for(UInt_t i=0;i<dcodarocinfo.size();i++){
    Int_t rocid = dcodarocinfo[i]->rocid;
    // We can use any roc ID besides 1,
    // just use 11.
    if(rocid == 11){
      ULong64_t rocTime = dcodarocinfo[i]->timestamp;
      TriggerBIT = rocTime % 6;
      break;
    }
  }

	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

  // Fill histogram of TI % 6 vs (ADC time - TDC time)
  for(UInt_t tof = 0;tof<dtofdigihits.size();tof++){
    for(UInt_t tof_TDC = 0;tof_TDC<dtoftdcdigihits.size();tof_TDC++){
      // Don't bother with matching of ADC and TDC hits,
      // just fill for all combinations

      // Make sure TI remainder is there
      if(TriggerBIT != -1){
	Double_t adc_time = dtofdigihits[tof]->pulse_time * ADC_BIN;
	Double_t tdc_time = (Double_t)dtoftdcdigihits[tof_TDC]->time * TDC_BIN;
	Double_t diff     = adc_time - tdc_time;
	// cout << adc_time << "   " << tdc_time << "   " << diff << endl;
	hrocTimeRemainder_AdcTdcTimeDiff->Fill(diff, TriggerBIT);
      }
    }
  }

  for(UInt_t tof = 0;tof<dtofhits.size();tof++){
      double diff = dtofhits[tof]->t_fADC - dtofhits[tof]->t_TDC;
      hrocTimeRemainder_AdcTdcTimeDiff_corrected->Fill(diff, TriggerBIT);
  }

  lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
}
//----------------------------------------------------------------------------------
void JEventProcessor_TOF_TDC_shift::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}
//----------------------------------------------------------------------------------
void JEventProcessor_TOF_TDC_shift::Finish() {

  // calculate mean timing for 6 possible values of
  // TI remainder. The TI remainder bin with the highest
  // mean gives the shift value for this run.
  Double_t mean[6];
  TH1I *hproj;
  char hname[200];
  Double_t min = +99999;
  Int_t shift = -1;

  lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

  TDirectory *main = gDirectory;
  filedir->cd();
  gDirectory->cd("TOF_TDC_shift");

  /*
  // multiple ADC hits skew the rough mean, so make a broad cut around the main peak
  // use TCutG to cut +- 60 ns (arbitrary) around the peak in X
  const double twindow = 60.;  // (ns)
  TCutG time_cut("time_cut",5);
  //time_cut.SetVarX("x");
  //time_cut.SetVarY("y");
  time_cut.SetPoint(0,mean_time-twindow,0.);
  time_cut.SetPoint(1,mean_time-twindow,6.);
  time_cut.SetPoint(2,mean_time+twindow,6.);
  time_cut.SetPoint(3,mean_time+twindow,0.);
  time_cut.SetPoint(4,mean_time-twindow,0.);
  */

  const double fit_window = 20.;
  for(Int_t i=0;i<6;i++){
    // Get projection of (ADC time - TDC time) for each value of TI remainder
    sprintf(hname,"TOF_TDC_shift/h%d",i);
    hproj = (TH1I*)hrocTimeRemainder_AdcTdcTimeDiff->ProjectionX(hname,i+1,i+1);
    //hproj = (TH1I*)hrocTimeRemainder_AdcTdcTimeDiff->ProjectionX(hname,i+1,i+1,"[time_cut]");
    //mean[i] = hproj->GetMean();
    double maximum = hproj->GetBinCenter(hproj->GetMaximumBin());
    TFitResultPtr fr = hproj->Fit("gaus", "S", "", maximum - fit_window, maximum + fit_window);
    if(fr == 0) {   // fit succeeds 
        mean[i] = fr->Parameter(1);;
    } else {
        mean[i] = hproj->GetMean();
    }
    cout << "TI remainder = " << i << " mean = " << mean[i] << endl;
    if(fabs(mean[i]) < fabs(min)){
      min = mean[i];
      shift = i;
    }
  }

  // shift value can only be 1, 3, or 5.  (not true anymore due to TI updates? - sdobbs 3 Mar. 2016)
  //if(!(shift == 1 || shift == 3 || shift == 5)) shift = -1;

  OUTPUT << shift << endl;
  //filedir->cd();
  main->cd();

  lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

  // Called before program exit after event processing is finished.
}
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

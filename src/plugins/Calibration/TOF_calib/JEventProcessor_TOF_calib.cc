// $Id$
//
//    File: JEventProcessor_TOF_calib.cc
// Created: Fri Mar 13 10:37:27 EDT 2015
// Creator: zihlmann (on Linux gluon47.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//


#include "JEventProcessor_TOF_calib.h"
#include "TOF/DTOFGeometry.h"
#include "DANA/DEvent.h"

#include <thread>
#include <mutex>

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_TOF_calib());
  }
} // "C"

//------------------
// JEventProcessor_TOF_calib (Constructor)
//------------------
JEventProcessor_TOF_calib::JEventProcessor_TOF_calib()
{
	SetTypeName("JEventProcessor_TOF_calib");
}

//------------------
// ~JEventProcessor_TOF_calib (Destructor)
//------------------
JEventProcessor_TOF_calib::~JEventProcessor_TOF_calib()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TOF_calib::Init()
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
  // GetLockService(locEvent)->RootWriteLock();
  //  ... fill historgrams or trees ...
  // GetLockService(locEvent)->RootUnLock();
  //
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  cout<<"INITIALIZE VARIABLES "<<flush<<endl;

  pthread_mutex_init(&mutex, NULL);
  ThreadCounter = 0;


  //BINTDC_2_TIME = 0.025;
  BINTDC_2_TIME = 0.0234375;
  BINADC_2_TIME = 0.0625; // is 4ns/64

  //TDCTLOC = 420.;
  //ADCTLOC = 130.;

  TDCTLOC = 180.;
  ADCTLOC = 130.;

  ADCTimeCut = 70.;
  TDCTimeCut = 70.;

  app->SetDefaultParameter("TOFCALIB:TDCTPEAK",TDCTLOC,
                              "Define location of TOF TDC time-peak in Raw histogram");
  app->SetDefaultParameter("TOFCALIB:ADCTPEAK",ADCTLOC,
                              "Defin location of TOF ADC time-peak in Raw histogram");

  // move histogram creation to brun() for potential run dependence, since the tree is initialized there
  // probably we should separate the histogram and tree logic, and use a DTreeInterface class
  // but this is left for later work - sdobbs, 8/17/2020
  //MakeHistograms();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TOF_calib::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  auto calibration = GetJCalibration(event);

  RunNumber = event->GetRunNumber();
  lockService->RootWriteLock();
  ThreadCounter++;
  lockService->RootUnLock();

  MakeHistograms();

  // this should have already been done in init()
  // so just in case.....

  map<string,double> tdcshift;
  const DTOFGeometry *locTOFGeometry = nullptr;
  event->GetSingle(locTOFGeometry);
  string locTOFTDCShiftTable = locTOFGeometry->Get_CCDB_DirectoryName() + "/tdc_shift";
  if(!calibration->Get(locTOFTDCShiftTable.c_str(), tdcshift)) {
    TOF_TDC_SHIFT = tdcshift["TOF_TDC_SHIFT"];
  }
  
  const DTOFGeometry& tofGeom = *locTOFGeometry; 
  // load base time offset
  map<string,double> base_time_offset;
  string locTOFBaseTimeOffsetTable = tofGeom.Get_CCDB_DirectoryName() + "/base_time_offset";
  if (calibration->Get(locTOFBaseTimeOffsetTable.c_str(),base_time_offset))
    jout << "Error loading " << locTOFBaseTimeOffsetTable << " !" << endl;
  if (base_time_offset.find("TOF_BASE_TIME_OFFSET") != base_time_offset.end())
    ADCTLOC = TMath::Abs(base_time_offset["TOF_BASE_TIME_OFFSET"]);
  else
    jerr << "Unable to get TOF_BASE_TIME_OFFSET from "<<locTOFBaseTimeOffsetTable<<" !" << endl;      
  
  if (base_time_offset.find("TOF_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
    TDCTLOC = TMath::Abs(base_time_offset["TOF_TDC_BASE_TIME_OFFSET"]);
  else
    jerr << "Unable to get TOF_TDC_BASE_TIME_OFFSET from "<<locTOFBaseTimeOffsetTable<<" !" << endl;
  

  jout<<"TOF: Updated ADC and TDC offsets according to CCDB: "<<ADCTLOC<<" / "<<TDCTLOC<<endl;

}

//------------------
// Process
//------------------
void JEventProcessor_TOF_calib::Process(const std::shared_ptr<const JEvent>& event)
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

  //NOTE: we do not use WriteLock() to safe time.


  //cout<<"CALL EVENT ROUTINE!!!! "<<eventnumber<<endl;

  // Get First Trigger Type
  vector <const DL1Trigger*> trig_words;
  uint32_t trig_mask, fp_trig_mask;
  try {
    event->Get(trig_words);
  } catch(...) {};
  if (trig_words.size()) {
    trig_mask = trig_words[0]->trig_mask;
    fp_trig_mask = trig_words[0]->fp_trig_mask;
  }
  else {
    trig_mask = 0;
    fp_trig_mask = 0;
  }


  /* fp_trig_mask & 0x100 - upstream LED
     fp_trig_mask & 0x200 - downstream LED
     trig_mask & 0x1 - cosmic trigger*/

  if (fp_trig_mask){ // this is a front pannel trigger like LED
    return;
  }
  if (trig_mask>7){ // this is not a BCAL/FCAL trigger
    return;
  }

  vector< const DCAEN1290TDCHit*> CAENHits;
  event->Get(CAENHits);
  if (CAENHits.size()<=0){
    return;
  }
  uint32_t locROCID = CAENHits[0]->rocid;

  vector <const DCODAROCInfo*> ROCS;
  event->Get(ROCS);
  int indx = -1;
  for ( unsigned int n=0; n<ROCS.size(); n++) {
    if (locROCID == ROCS[n]->rocid){
      indx = n;
      break;
    }
  }

  if (indx<0){
    return;
  }
  

  const DTOFGeometry* locTOFGeometry;
  event->GetSingle(locTOFGeometry);

  uint64_t TriggerTime = ROCS[indx]->timestamp;
  int TriggerBIT = TriggerTime%6;
  float TimingShift = TOF_TDC_SHIFT - (float)TriggerBIT;
  if(TimingShift <= 0) {
    TimingShift += 6.;
  }

  TimingShift *= 4. ;

  vector<const DTOFDigiHit*> ADCHits, ADCHitsLeft[2],ADCHitsRight[2];
  vector<const DTOFTDCDigiHit*> TDCHits, TDCHitsLeft[2], TDCHitsRight[2];
  vector<int> ADCLeftOverFlow[2],ADCRightOverFlow[2];

  event->Get(ADCHits);
  event->Get(TDCHits);

  // loop over DTOFDigiHit: this are ADC hits
  // sort them into ADCLeft and ADCRight hits
  // only keep hits within the time-peak
  // also keep the hodoscope planes separate
  int th[locTOFGeometry->Get_NPlanes()][locTOFGeometry->Get_NBars()][locTOFGeometry->Get_NEnds()];
  memset(th,0,locTOFGeometry->Get_NPlanes()*locTOFGeometry->Get_NBars()*locTOFGeometry->Get_NEnds()*4);
  for (unsigned int k=0; k<ADCHits.size(); k++){
    const DTOFDigiHit *hit = ADCHits[k];

    // only use DigiHits from fADC250 if QF for pedestal is ok
    if (hit->QF & 0x40){  // pedestal determination failed! do not use this hit!
      continue;
    }

    int plane = hit->plane;
    int end = hit->end;

    float time = (float)hit->pulse_time * BINADC_2_TIME;
    int val = (hit->pulse_time & 0x3F);
    if (!val){
      continue;
    }

    int bar = hit->bar;
    float indx = bar-1 + plane*locTOFGeometry->Get_NBars()*2 + end*locTOFGeometry->Get_NBars();
    //cout<<plane<<"  "<<bar<<"  "<<end<<endl;

    if (hit->pedestal){
      TOFPedestal->Fill(indx, (float)hit->pedestal);
      TOFEnergy->Fill(indx, (float)hit->pulse_integral);
      TOFPeak->Fill(indx, (float)hit->pulse_peak);
    }

    if (th[plane][bar-1][end]){ // only take first hit
      continue;
    }

    th[plane][bar-1][end] = 1;
    TOFADCtime->Fill(time,indx);
    if (fabsf(time-ADCTLOC)<ADCTimeCut){
      // test for overflow if raw data available
      vector <const Df250PulseIntegral*> PulseIntegral;
      hit->Get(PulseIntegral);
      //vector <const Df250WindowRawData*> WRawData;
      //PulseIntegral[0]->Get(WRawData);
      int overflow = 0;
      /*
 if ( WRawData.size() > 0) {
	for (int n=0;n<100;n++){
	  if (WRawData[0]->samples[n] & 0x1000){
	    overflow++;
	  }
	}
      } else {
	//overflow = PulseIntegral[0]->quality_factor;
      }
*/
      if (end){
	ADCHitsRight[plane].push_back(hit);
	ADCRightOverFlow[plane].push_back(overflow);
      } else {
	ADCHitsLeft[plane].push_back(hit);
	ADCLeftOverFlow[plane].push_back(overflow);
      }
    }
  }

  if (ADCLeftOverFlow[0].size() != ADCHitsLeft[0].size()){
    cout<<"Error vector size missmatch!"<<endl;
  }


  // loop over DTOFTDCDigiHits : these are the TDC hits
  // sort them into left and right hits
  // only keep hits within the time peak
  // also keep the hodoscope planes separate
  for (unsigned int k=0; k<TDCHits.size(); k++){
    const DTOFTDCDigiHit *hit = TDCHits[k];
    int plane = hit->plane;
    int end = hit->end;
    float time = (float)hit->time * BINTDC_2_TIME;
    float indx = plane*locTOFGeometry->Get_NBars()*2 + end*locTOFGeometry->Get_NBars() + hit->bar-1;
    TOFTDCtime->Fill(time, indx);
    if (fabsf(time-TDCTLOC)<TDCTimeCut){
      if (end){
	TDCHitsRight[plane].push_back(hit);
      } else {
	TDCHitsLeft[plane].push_back(hit);
      }
    }
  }

  float Signum = -1.;
  vector <paddle> TOFADCPaddles[2];
  vector <SingleP> TOFADCSingles[2];
  //int firsttime = 1;

  // for each hodoscope plane find matches between
  // ADC left and right data or find single hits for
  // the single ended readout paddles.
  for (int plane=0; plane<2; plane++){

    // loop over right pmts to find single ended paddle hits
    for (unsigned int i = 0; i<ADCHitsRight[plane].size(); i++) {
      const DTOFDigiHit *hitR = ADCHitsRight[plane][i];
      int paddle = hitR->bar;
      if ( locTOFGeometry->Is_ShortBar(paddle) ){
	struct SingleP newsingle;
	newsingle.paddle = paddle;
	newsingle.LR = 1;
	newsingle.time = (float)hitR->pulse_time*BINADC_2_TIME ;
	newsingle.adc = (float)hitR->pulse_integral -
	  (float)hitR->pedestal/(float)hitR->nsamples_pedestal*(float)hitR->nsamples_integral;

	newsingle.Peak = hitR->pulse_peak - (float)hitR->pedestal/(float)hitR->nsamples_pedestal;

	newsingle.OverFlow = ADCRightOverFlow[plane][i];
	TOFADCSingles[plane].push_back(newsingle);
      }
    }

    // loop over left pmts to find single ended paddle hits
    // for the other paddle loop over the right ones and find match

    for (unsigned int j = 0; j<ADCHitsLeft[plane].size(); j++) {
      const DTOFDigiHit *hit = ADCHitsLeft[plane][j];
      int paddle = hit->bar;
      if (locTOFGeometry->Is_ShortBar(paddle)){ // save singles of left pmts
	struct SingleP newsingle;
	newsingle.paddle = paddle;
	newsingle.LR = 0;
	newsingle.adc = (float)hit->pulse_integral -
	  (float)hit->pedestal/(float)hit->nsamples_pedestal*(float)hit->nsamples_integral;

	newsingle.Peak = hit->pulse_peak - (float)hit->pedestal/(float)hit->nsamples_pedestal;

	newsingle.time = (float)hit->pulse_time*BINADC_2_TIME ;
	newsingle.OverFlow = ADCLeftOverFlow[plane][j];
	TOFADCSingles[plane].push_back(newsingle);
      } else {

	// loop over Right adc hits find match with the left and prepare paddle hits
	for (unsigned int i = 0; i<ADCHitsRight[plane].size(); i++) {
	  const DTOFDigiHit *hitR = ADCHitsRight[plane][i];
	  if (hitR->bar == paddle){
	    struct paddle newpaddle;
	    newpaddle.paddle = paddle;
	    newpaddle.timeL = (float)hit->pulse_time*BINADC_2_TIME ;
	    newpaddle.timeR = (float)hitR->pulse_time*BINADC_2_TIME ;
	    newpaddle.mt = (newpaddle.timeL + newpaddle.timeR)/2.;
	    newpaddle.td = Signum*(newpaddle.timeL - newpaddle.timeR)/2.;
	    newpaddle.adcL = (float)hit->pulse_integral -
	      (float)hit->pedestal/(float)hit->nsamples_pedestal*(float)hit->nsamples_integral;
	    newpaddle.adcR = (float)hitR->pulse_integral -
	      (float)hitR->pedestal/(float)hitR->nsamples_pedestal*(float)hitR->nsamples_integral;

	    newpaddle.PeakR = hitR->pulse_peak - (float)hitR->pedestal/(float)hitR->nsamples_pedestal;

	    newpaddle.PeakL = hit->pulse_peak - (float)hit->pedestal/(float)hit->nsamples_pedestal;

	    newpaddle.OverFlowL =  ADCLeftOverFlow[plane][j];
	    newpaddle.OverFlowR =  ADCRightOverFlow[plane][i];
	    if (!locTOFGeometry->Is_ShortBar(paddle)) {
	      //cout<<newpaddle.OverFlowL<<"   "<< newpaddle.OverFlowR <<endl;
	      TOFADCPaddles[plane].push_back(newpaddle);
	    }
	  }
	}
      }
    }
  }

  vector <paddle> TOFTDCPaddles[2];
  vector <SingleP> TOFTDCSingles[2];
  //firsttime = 1;

  // now do the same thing for TDC hits
  // find matches between left and right and treat the single ended paddles separately

  for (int plane=0; plane<2; plane++){

    for (unsigned int j = 0; j<TDCHitsRight[plane].size(); j++) {
      const DTOFTDCDigiHit *hit = TDCHitsRight[plane][j];
      int paddle = hit->bar;
      if (locTOFGeometry->Is_ShortBar(paddle)){
	struct SingleP newsingle;
	newsingle.paddle = paddle;
	newsingle.LR = 1;
	newsingle.time = (float)hit->time*BINTDC_2_TIME ;
	TOFTDCSingles[plane].push_back(newsingle);
      }
    }

    for (unsigned int j = 0; j<TDCHitsLeft[plane].size(); j++) {
      const DTOFTDCDigiHit *hit = TDCHitsLeft[plane][j];
      int paddle = hit->bar;
      if (locTOFGeometry->Is_ShortBar(paddle)){
	struct SingleP newsingle;
	newsingle.paddle = paddle;
	newsingle.LR = 0;
	newsingle.time = (float)hit->time*BINTDC_2_TIME ;
	TOFTDCSingles[plane].push_back(newsingle);
      } else {
	for (unsigned int i = 0; i<TDCHitsRight[plane].size(); i++) {
	  const DTOFTDCDigiHit *hitR = TDCHitsRight[plane][i];
	  if (hitR->bar == paddle){
	    struct paddle newpaddle;
	    newpaddle.paddle = paddle;
	    newpaddle.timeL = (float)hit->time*BINTDC_2_TIME ;
	    newpaddle.timeR = (float)hitR->time*BINTDC_2_TIME ;
	    newpaddle.mt = (newpaddle.timeL + newpaddle.timeR)/2.;
	    newpaddle.td = Signum*(newpaddle.timeL - newpaddle.timeR)/2.; // left side get positive x
	    if ((paddle != 22) && (paddle !=23)) {
	      TOFTDCPaddles[plane].push_back(newpaddle);
	    }
	  }
	}
      }
    }
  }

  // FILL HISTOGRAMS
  // Since we are filling histograms (and trees in a file) local to this plugin, 
  // it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock

  lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  
  Event = event->GetEventNumber();
  TShift = TimingShift;
  Nhits = TOFTDCPaddles[0].size() + TOFTDCPaddles[1].size();
  int cnt = 0;
  int AllHits[4]={0,0,0,0};
  
  if (Nhits<MaxHits){
    for (unsigned int k = 0; k<TOFTDCPaddles[0].size() ; k++){
      Plane[cnt] = 0;
      Paddle[cnt] = TOFTDCPaddles[0][k].paddle;
      MeanTime[cnt] = TOFTDCPaddles[0][k].mt;
      TimeDiff[cnt] = TOFTDCPaddles[0][k].td;
      cnt++;
      AllHits[0]++;
    }
    AllHits[0] = cnt;
    for (unsigned int k = 0; k<TOFTDCPaddles[1].size() ; k++){
      Plane[cnt] = 1;
      Paddle[cnt] = TOFTDCPaddles[1][k].paddle;
      MeanTime[cnt] = TOFTDCPaddles[1][k].mt;
      TimeDiff[cnt] = TOFTDCPaddles[1][k].td;
      cnt++;
      AllHits[1]++;
    }
  } else {
    Nhits = 0;
  }
  
  cnt = 0;
  NhitsA = TOFADCPaddles[0].size() + TOFADCPaddles[1].size();
  if (NhitsA<MaxHits){
    for (unsigned int k = 0; k<TOFADCPaddles[0].size() ; k++){
      PlaneA[cnt] = 0;
      PaddleA[cnt] = TOFADCPaddles[0][k].paddle;
      MeanTimeA[cnt] = TOFADCPaddles[0][k].mt;
      TimeDiffA[cnt] = TOFADCPaddles[0][k].td;
      ADCL[cnt] = TOFADCPaddles[0][k].adcL;
      ADCR[cnt] = TOFADCPaddles[0][k].adcR;
      OFL[cnt] = TOFADCPaddles[0][k].OverFlowL;
      OFR[cnt] = TOFADCPaddles[0][k].OverFlowR;
      PEAKL[cnt] = TOFADCPaddles[0][k].PeakL;
      PEAKR[cnt] = TOFADCPaddles[0][k].PeakR;
      
      cnt++;
      AllHits[2]++;
    }
    
    for (unsigned int k = 0; k<TOFADCPaddles[1].size() ; k++){
      PlaneA[cnt] = 1;
      PaddleA[cnt] = TOFADCPaddles[1][k].paddle;
      MeanTimeA[cnt] = TOFADCPaddles[1][k].mt;
      TimeDiffA[cnt] = TOFADCPaddles[1][k].td;
      ADCL[cnt] = TOFADCPaddles[1][k].adcL;
      ADCR[cnt] = TOFADCPaddles[1][k].adcR;
      OFL[cnt] = TOFADCPaddles[1][k].OverFlowL;
      OFR[cnt] = TOFADCPaddles[1][k].OverFlowR;
      PEAKL[cnt] = TOFADCPaddles[1][k].PeakL;
      PEAKR[cnt] = TOFADCPaddles[1][k].PeakR;
      
      cnt++;
      AllHits[3]++;
    }
  } else {
    NhitsA = 0;
  }
  
  NsinglesA = TOFADCSingles[0].size() + TOFADCSingles[1].size();
  NsinglesT = TOFTDCSingles[0].size() + TOFTDCSingles[1].size();

  unsigned int j=0;
  for (unsigned int k = 0; k<TOFADCSingles[0].size(); k++){
    PlaneSA[k] = 0;
    PaddleSA[k] = TOFADCSingles[0][k].paddle ;
    LRA[k] = TOFADCSingles[0][k].LR ;
    ADCS[k] = TOFADCSingles[0][k].adc;
    TADCS[k] = TOFADCSingles[0][k].time;
    OF[k] = TOFADCSingles[0][k].OverFlow;
    PEAK[k] = TOFADCSingles[0][k].Peak;

    j++;
  }
  for (unsigned int k = 0; k<TOFADCSingles[1].size(); k++){
    PlaneSA[k+j] = 1;
    PaddleSA[k+j] = TOFADCSingles[1][k].paddle ;
    LRA[k+j] = TOFADCSingles[1][k].LR ;
    ADCS[k+j] = TOFADCSingles[1][k].adc;
    TADCS[k+j] = TOFADCSingles[1][k].time; ;
    OF[k+j] = TOFADCSingles[1][k].OverFlow;
    PEAK[k+j] = TOFADCSingles[1][k].Peak;
 }
  j = 0;
  for (unsigned int k = 0; k<TOFTDCSingles[0].size(); k++){
    PlaneST[k] = 0;
    PaddleST[k] = TOFTDCSingles[0][k].paddle ;
    LRT[k] = TOFTDCSingles[0][k].LR ;
    TDCST[k] = TOFTDCSingles[0][k].time;
    j++;
  }
  for (unsigned int k = 0; k<TOFTDCSingles[1].size(); k++){
    PlaneST[k+j] = 1;
    PaddleST[k+j] = TOFTDCSingles[1][k].paddle ;
    LRT[k+j] = TOFTDCSingles[1][k].LR ;
    TDCST[k+j] = TOFTDCSingles[1][k].time; ;
  }

  if (((AllHits[0]>0) &&  (AllHits[1]>0)) ||
      ((AllHits[2]>0) &&  (AllHits[3]>0))){
    t3->Fill();
  }
  
  lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_TOF_calib::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TOF_calib::Finish()
{
  // Called before program exit after event processing is finished.

  lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

  ThreadCounter--;
  if (ThreadCounter == 0) {
    WriteRootFile();
  }

  lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}


jerror_t JEventProcessor_TOF_calib::WriteRootFile(void){

  TDirectory *top = gDirectory;

  ROOTFile->cd();
  ROOTFile->cd("TOFcalib");

  TOFTDCtime->Write();
  TOFADCtime->Write();
  TOFEnergy->Write();
  TOFPeak->Write();
  TOFPedestal->Write();

  t3->Write();

  ROOTFile->cd();
  ROOTFile->Close();
  
  top->cd();

  return NOERROR;

}

jerror_t JEventProcessor_TOF_calib::MakeHistograms(void){

	//NO LOCKS: CALLED IN init(): GUARANTEED TO BE SINGLE-THREADED
        // (not really currently true)
  /*
  cout<<endl;
  cout<<"CALL MakeHistograms for TOF_calib!!!! "<<endl;
  cout<<endl;
  */

    //cout<<"SETUP HISTOGRAMS AND TREE FOR RUN "<<RunNumber<<flush<<endl;

  std::once_flag flag;
  std::call_once(flag, [&](){
    lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    TDirectory *top = gDirectory;

    // create root file here so the tree does not show up in hd_root.root
    sprintf(ROOTFileName,"hd_root_tofcalib.root");
    ROOTFile = new TFile(ROOTFileName,"recreate");
    ROOTFile->cd();

    ROOTFile->mkdir("TOFcalib");
    ROOTFile->cd("TOFcalib");


    TOFTDCtime = new TH2F("TOFTDCtime","TOF CAEN TDC times", 8000, 0., 4000., 200, 0., 200.);
    TOFADCtime = new TH2F("TOFADCtime","TOF ADC times", 800, 0., 400., 200, 0., 200.);

    TOFEnergy = new TH2F("TOFEnergy","TOF Energy Integral (no ped subraction)",
			 200, 0., 200., 100, 0., 20000.);
    TOFPeak = new TH2F("TOFPeak","TOF Peak Amplitude",200, 0., 200., 100, 0., 4100.);
    TOFPedestal = new TH2F("TOFPedestal","TOF Pedestal",200, 0., 200., 300, 0., 600.);


    t3 = new TTree("t3","TOF Hits");
    t3->Branch("Event", &Event,"Event/I");

    t3->Branch("Nhits", &Nhits,"Nhits/I");
    t3->Branch("TShift",&TShift,"TShift/F");
    t3->Branch("Plane",Plane,"Plane[Nhits]/I");
    t3->Branch("Paddle",Paddle,"Paddle[Nhits]/I");
    t3->Branch("MeanTime",MeanTime,"MeanTime[Nhits]/F");
    t3->Branch("TimeDiff",TimeDiff,"TimeDiff[Nhits]/F");

    t3->Branch("NhitsA", &NhitsA,"NhitsA/I");
    t3->Branch("PlaneA",PlaneA,"PlaneA[NhitsA]/I");
    t3->Branch("PaddleA",PaddleA,"PaddleA[NhitsA]/I");
    t3->Branch("MeanTimeA",MeanTimeA,"MeanTimeA[NhitsA]/F");
    t3->Branch("TimeDiffA",TimeDiffA,"TimeDiffA[NhitsA]/F");
    t3->Branch("ADCL",ADCL,"ADCL[NhitsA]/F");
    t3->Branch("ADCR",ADCR,"ADCR[NhitsA]/F");
    t3->Branch("OFL",OFL,"OFL[NhitsA]/I");
    t3->Branch("OFR",OFR,"OFR[NhitsA]/I");
    t3->Branch("PEAKL",PEAKL,"PEAKL[NhitsA]/F");
    t3->Branch("PEAKR",PEAKR,"PEAKR[NhitsA]/F");

    t3->Branch("NsinglesA", &NsinglesA,"NsinglesA/I");
    t3->Branch("PlaneSA",PlaneSA,"PlaneSA[NsinglesA]/I");
    t3->Branch("PaddleSA",PaddleSA,"PaddleSA[NsinglesA]/I");
    t3->Branch("LRA",LRA,"LRA[NsinglesA]/I"); //LA=0,1 (left/right)
    t3->Branch("ADCS",ADCS,"ADCS[NsinglesA]/F");
    t3->Branch("OF",OF,"OF[NsinglesA]/I");
    t3->Branch("TADCS",TADCS,"TADCS[NsinglesA]/F");
    t3->Branch("PEAK",PEAK,"PEAK[NsinglesA]/F");

    t3->Branch("NsinglesT", &NsinglesT,"NsinglesT/I");
    t3->Branch("PlaneST",PlaneST,"PlaneSA[NsinglesT]/I");
    t3->Branch("PaddleST",PaddleST,"PaddleSA[NsinglesT]/I");
    t3->Branch("LRT",LRT,"LRT[NsinglesT]/I"); //LA=0,1 (left/right)
    t3->Branch("TDCST",TDCST,"TDCST[NsinglesT]/F");

    top->cd();

    lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
  });

  return NOERROR;
}

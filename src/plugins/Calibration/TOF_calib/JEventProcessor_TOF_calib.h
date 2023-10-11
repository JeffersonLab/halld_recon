// $Id$
//
//    File: JEventProcessor_TOF_calib.h
// Created: Fri Mar 13 10:37:27 EDT 2015
// Creator: zihlmann (on Linux gluon47.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TOF_calib_
#define _JEventProcessor_TOF_calib_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/jerror.h>
#include <JANA/Compatibility/JLockService.h>

using namespace std;
#include <stdint.h>
#include <DAQ/DCODAEventInfo.h>
#include <TOF/DTOFDigiHit.h>

#include <TOF/DTOFTDCDigiHit.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250WindowRawData.h>
#include <TRIGGER/DL1Trigger.h>
#include <DAQ/DCAEN1290TDCHit.h>
#include <DAQ/DCODAROCInfo.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <math.h>

#define MaxHits 100

class JEventProcessor_TOF_calib:public JEventProcessor{
 public:
  JEventProcessor_TOF_calib();
  ~JEventProcessor_TOF_calib();

  int RunNumber;
  int ThreadCounter;

  float TDCTLOC;
  float ADCTLOC;

  float TOF_TDC_SHIFT;
  
  TH2F *TOFTDCtime;
  TH2F *TOFADCtime;
  TH2F *TOFEnergy;
  TH2F *TOFPeak;
  TH2F *TOFPedestal;


  float ADCTimeCut;
  float TDCTimeCut;

  float BINTDC_2_TIME;
  float BINADC_2_TIME;
  struct paddle{
    int paddle;
    float timeL;
    float timeR;
    float mt;
    float td;
    float adcL;
    float adcR;
    int OverFlowL;
    int OverFlowR;
    float PeakL;
    float PeakR;
  };

  struct SingleP{
    int paddle;
    int LR;
    float time;
    float adc;
    int OverFlow;
    float Peak;
  };


  char ROOTFileName[128];
  TFile *ROOTFile;
  TTree *t3;
  int Event;
  int Nhits;
  int Plane[MaxHits];
  int Paddle[MaxHits];
  float MeanTime[MaxHits];
  float TimeDiff[MaxHits];
  float TShift;

  int NhitsA;
  int PlaneA[MaxHits];
  int PaddleA[MaxHits];
  float MeanTimeA[MaxHits];
  float TimeDiffA[MaxHits];
  float ADCL[MaxHits];
  float ADCR[MaxHits];
  int OFL[MaxHits];
  int OFR[MaxHits];
  float PEAKL[MaxHits];
  float PEAKR[MaxHits];

  int NsinglesA;
  int PlaneSA[MaxHits];
  int PaddleSA[MaxHits];
  int LRA[MaxHits];
  float ADCS[MaxHits];
  float TADCS[MaxHits];
  int OF[MaxHits];
  float PEAK[MaxHits];

  int NsinglesT;
  int PlaneST[MaxHits];
  int PaddleST[MaxHits];
  int LRT[MaxHits];
  float TDCST[MaxHits];

  pthread_mutex_t mutex;
 private:
  jerror_t WriteRootFile(void);
  jerror_t MakeHistograms(void);

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
  std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TOF_calib_




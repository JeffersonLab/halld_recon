// $Id$
//
//    File: JEventProcessor_BEAM_online.h
// Created: Thu Jan  9 11:26:05 EST 2020
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_BEAM_online_
#define _JEventProcessor_BEAM_online_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>

#include <TRIGGER/DL1Trigger.h>
#include <PID/DBeamPhoton.h>
#include <PID/DEventRFBunch.h>

#include <RF/DRFTime.h>

#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSPair.h>

#include <DAQ/DCODAROCInfo.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>


using namespace std;
using namespace jana;


class JEventProcessor_BEAM_online:public jana::JEventProcessor{
 public:
  JEventProcessor_BEAM_online();
  ~JEventProcessor_BEAM_online();
  const char* className(void){return "JEventProcessor_BEAM_online";}

  int BlockStart;
  uint64_t LastTime;
  double RFWidth;

  TH1D *TriggerTime;
  TH1D *PStagm;
  TH1D *PStagh;

  TH2D *PStagm2d;
  TH2D *PStagh2d;

  TH1D *PStagmEnergyInTime;
  TH1D *PStagmEnergyOutOfTime;
  TH1D *PStaghEnergyInTime;
  TH1D *PStaghEnergyOutOfTime;
  TH1D *outoftimeM;
  TH1D *outoftimeH;
  TH1D *outoftimeMIX;
  TH1D *outoftimeHij;

  
  TH2D *PStagmEIT;
  TH2D *PStagmEOOT;
  TH2D *PStaghEIT;
  TH2D *PStaghEOOT;
  
  TH2D *correlationC;
  TH1D *correlationE;
  
  TH2D *acc;
  TH1D *fencePS;
  TH1D *fenceRF;

  TH2D *deltaTall;
  TH2D *deltaT;
  TH2D *deltaTs;
  TH2D *deltaTsa;  
  TH2D *MICdeltaT;
  
  
  
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_BEAM_online_


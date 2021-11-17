// $Id$
//
//    File: DEventProcessor_2p1pi1pi0.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_2p1pi1pi0_
#define _DEventProcessor_2p1pi1pi0_

#include <JANA/JEventProcessor.h>
using namespace jana;

#include <PID/DBeamPhoton.h>
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>
#include "ANALYSIS/DTreeInterface.h"
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DParticleID.h>
#include <PID/DNeutralParticle.h>

#include "TApplication.h"
#include "TCanvas.h"

#include "TProfile.h"
#include <TTree.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>


class DEventProcessor_2p1pi1pi0:public JEventProcessor{
 public:
  DEventProcessor_2p1pi1pi0(){};
  ~DEventProcessor_2p1pi1pi0(){};
  const char* className(void){return "DEventProcessor_2p1pi1pi0";}
  
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
  
  DKinFitUtils_GlueX *dKinFitUtils;
  DKinFitter *dKinFitter;

  void GetHypotheses(vector<const DChargedTrack *> &tracks,
		     map<Particle_t, int> &particles,
		     map<Particle_t, vector<const DChargedTrackHypothesis*> > &assignmentHypothesis,
		     vector<map<Particle_t, vector<const DChargedTrackHypothesis*> > > &hypothesisList
		     ) const;

  Int_t fcal_ncl; 
  
  Float_t fcal_en_cl, fcal_x_cl, fcal_y_cl;

  //TREE
  DTreeInterface* dTreeInterface;
  //thread_local: Each thread has its own object: no lock needed
  //important: manages it's own data internally: don't want to call new/delete every event!
  static thread_local DTreeFillData dTreeFillData;

  // Histograms
  
  TH2D * h_BCAL;
  TH2D * h_FCAL;
  TH1D * h_m2gamma;
  TH1D * h_m2pi;

  const double c = 29.98;
  const double mN = 0.93892;
  const double mpip = 0.139570;
  
};

#endif // _DEventProcessor_2p1pi2gamma_


// $Id$
//
//    File: JEventProcessor_BEAM_online.cc
// Created: Thu Jan  9 11:26:05 EST 2020
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_BEAM_online.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_BEAM_online());
  }
} // "C"


//------------------
// JEventProcessor_BEAM_online (Constructor)
//------------------
JEventProcessor_BEAM_online::JEventProcessor_BEAM_online()
{
	SetTypeName("JEventProcessor_BEAM_online");
}

//------------------
// ~JEventProcessor_BEAM_online (Destructor)
//------------------
JEventProcessor_BEAM_online::~JEventProcessor_BEAM_online()
{
  
}

//------------------
// Init
//------------------
void JEventProcessor_BEAM_online::Init()
{
  // This is called once at program startup. 
  // create root folder for BEAM histograms
  TDirectory *main = gDirectory;
  gDirectory->mkdir("BEAM")->cd();
  
  TriggerTime = new TH1D("TriggerTime", "Time difference of consecuitive triggers [ns]", 10000, 0., 200e3);
  TriggerTime->SetTitle("Time difference between two consecutive physics triggers");
  TriggerTime->GetXaxis()->SetTitle("#Deltat [ns]");

  PStagm = new TH1D("PStagm", "PStime minus tagm time", 5000, -120., 100.);
  PStagh = new TH1D("PStagh", "PStime minus tagh time", 5000, -120., 100.);

  PStagm2d = new TH2D("PStagm2d", "Microscope Counter ID vs. tagm time - PStime", 1000, -120., 100., 120, 0., 120.);
  PStagh2d = new TH2D("PStagh2d", "Hodoscope Counter  ID vs. tagh time - PStime",  1000, -120., 100.,  350, 0., 350.);

  PStagmEnergyInTime = new TH1D("PStagmEnergyInTime", "PSEnergy - TaggerEnergy in time photons microscope", 2000, -4., 4.);
  PStagmEnergyOutOfTime = new TH1D("PStagmEnergyOutOfTime", "PSEnergy - TaggerEnergy out of time photons microscope", 2000, -4., 4.);
  PStaghEnergyInTime = new TH1D("PStaghEnergyInTime", "PSEnergy - TaggerEnergy in time photons hodoscope", 2000, -4., 4.);
  PStaghEnergyOutOfTime = new TH1D("PStaghEnergyOutOfTime", "PSEnergy - TaggerEnergy outo of time photons hodoscope", 2000, -4., 4.);

  PStagmEIT = new TH2D("PStagmEIT", "TAGM ID vs (PSEnergy - TaggerEnergy) in time photons", 2000, -4., 4., 120, 0, 120);
  PStagmEOOT = new TH2D("PStagmEOOT", "TAGM ID vs PSEnergy - (TaggerEnergy) out of time photons", 2000, -4., 4., 120, 0, 120);

  PStaghEIT = new TH2D("PStaghEIT", "TAGH ID vs (PSEnergy - TaggerEnergy) in time photons", 2000, -4., 4., 350, 0, 350);
  PStaghEOOT = new TH2D("PStaghEOOT", "TAGH ID vs PSEnergy - (TaggerEnergy) out of time photons", 2000, -4., 4., 350, 0, 350);

  outoftimeH   = new TH1D("outoftimeH",   "outoftime deltat tagger hits H", 10000, -100., 100.);
  outoftimeM   = new TH1D("outoftimeM",   "outoftime deltat tagger hits M ", 5000, -100., 100.);
  outoftimeMIX = new TH1D("outoftimeMIX", "outoftime deltat tagger hits MIX", 5000, -100., 100.);
  outoftimeHij  = new TH1D("outoftimeHij",  "outoftime deltat tagger hits Hij i=220 j=100", 5000, -100., 100.);

  correlationC = new TH2D("correlationC", "Tagger Counter correlations when in time", 350, 0, 350., 350, 0., 350.);
  correlationE = new TH1D("correlationE", "Tagger Counter Energy Sum when in time", 1000, 4., 13.);


  acc = new TH2D("acc", "Microscope Counter ID vs. acc time difference with bunch -11", 1000, -120., 100., 120, 0., 120.);
  fencePS = new TH1D("fencePS", "tagm time minus PS_time", 6000, -120., 120.);
  fenceRF = new TH1D("fenceRF", "tagm time minus RF_time", 6000, -120., 120.);

  deltaT =new TH2D("deltaT", "Microscope counter time differences", 8000, -200., 200., 130, 0., 130);
  deltaTall =new TH2D("deltaTall", "ALL Microscope counter time differences", 8000, -200., 200., 130, 0., 130.);
  deltaTs =new TH2D("deltaTs", "Microscope counter time differences with itself", 80, -2., 2., 130, 0., 130.);
  deltaTsa =new TH2D("deltaTsa", "Microscope counter time differences with itself all", 80, -2., 2., 130, 0., 130.);

  MICdeltaT = new TH2D("MICdeltaT", "dT = Tmic-Tref vs Microscope counter for random triggers", 130, 0., 130., 8000, -200., 200. );
  
  main->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_BEAM_online::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  BlockStart = 1;
  RFWidth = 4.008; // 249.5MHz correspondes to 4.008ns
}

//------------------
// Process
//------------------
void JEventProcessor_BEAM_online::Process(const std::shared_ptr<const JEvent>& event)
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
  // lockService->RootWriteLock();
  //  ... fill historgrams or trees ...
  // lockService->RootUnLock();

  auto eventnumber = event->GetEventNumber();

  vector <const DL1Trigger*> trig;
  event->Get(trig);

  if (trig.size() == 0){
    //cout<<"no trigger found"<<endl;
    return;     
  }

  if (!eventnumber%40){
    // this is the first event in the block
    BlockStart = 1;
  }

  if (!BlockStart && (trig[0]->trig_mask & 0x3)){
    double dt = (double)(trig[0]->timestamp - LastTime)*4.;  // units are in ns
    TriggerTime->Fill(dt);
    LastTime = trig[0]->timestamp;
  }

  if (BlockStart && (trig[0]->trig_mask > 0)){
    // this is the first physics trigger in the block
    BlockStart = 0;
    LastTime = trig[0]->timestamp;
  }


  // This of for PS-Triggers Bit 4 
  if (trig[0]->trig_mask & 0x8){ 
   
    vector <const DBeamPhoton*> Beam;
    event->Get(Beam);
    int NBeamPhotons = Beam.size();
    vector <const DPSPair*> PSPairs;
    event->Get(PSPairs);
    
    if (PSPairs.size() < 1){
      return;
    }
    
    vector <const DPSPair*> OutOfTimePairsM;
    vector <const DPSPair*> OutOfTimePairsH;
    vector <const DBeamPhoton*> OutOfTimeBeamM;
    vector <const DBeamPhoton*> OutOfTimeBeamH;
    vector <const DBeamPhoton*> OutOfTimeBeamPhotons;
    
    double Width = RFWidth; // 249.5MHz correspondes to 4.008ns
    double tpair = (PSPairs[0]->ee.first->t + PSPairs[0]->ee.second->t) / 2.;
    double epair = (PSPairs[0]->ee.first->E + PSPairs[0]->ee.second->E);

    // loop over beam photons
    for (int k=0; k<NBeamPhotons; k++){
      float dt = Beam[k]->time() - tpair;
      
      // Beam photon is out of time with PS-event
      if (TMath::Abs(dt) >  Width*3./2.) {
	OutOfTimeBeamPhotons.push_back(Beam[k]);		
	if (Beam[k]->dSystem == SYS_TAGM){
	  OutOfTimeBeamM.push_back(Beam[k]);
	}else {
	  OutOfTimeBeamH.push_back(Beam[k]);
	}
      }
      
      if (Beam[k]->dSystem == SYS_TAGM){
	
	for (int n=0; n<(int)PSPairs.size(); n++){
	  const DPSPair *pair = PSPairs[n];
	  tpair = (PSPairs[n]->ee.first->t + PSPairs[n]->ee.second->t) / 2.;
	  epair = (PSPairs[n]->ee.first->E + PSPairs[n]->ee.second->E);
	  double delta = Beam[k]->time() - tpair;
	  PStagm->Fill(delta);
	  PStagm2d->Fill(delta, Beam[k]->dCounter);
	  
	  if (TMath::Abs(delta)<Width/2.){
	    double DeltaE = Beam[k]->momentum().Mag() - epair;
	    PStagmEnergyInTime->Fill(-DeltaE);
	    PStagmEIT->Fill(-DeltaE, Beam[k]->dCounter);
	    
	  } else if ( ( TMath::Abs(delta) >  Width*3./2.) && 
		      ( TMath::Abs(delta) <  Width*3./2.+ 10.*Width))  {  // +/- 10 beam bunches!
	    double DeltaE = Beam[k]->momentum().Mag() - epair;
	    PStagmEnergyOutOfTime->Fill(-DeltaE);
	    PStagmEOOT->Fill(-DeltaE, Beam[k]->dCounter);
	    OutOfTimePairsM.push_back(pair);
	  }
	}
	
      } else {
	
	for (int n=0; n<(int)PSPairs.size(); n++){
	  const DPSPair *pair = PSPairs[n];
	  tpair = (PSPairs[n]->ee.first->t + PSPairs[n]->ee.second->t) / 2.;
	  epair = (PSPairs[n]->ee.first->E + PSPairs[n]->ee.second->E);
	  double delta = Beam[k]->time() - tpair;
	  PStagh->Fill(delta);
	  PStagh2d->Fill(delta, Beam[k]->dCounter);
	  if (TMath::Abs(delta)<Width/2.){
	    double DeltaE = Beam[k]->momentum().Mag() - epair;
	    PStaghEnergyInTime->Fill(-DeltaE);
	    PStaghEIT->Fill(-DeltaE, Beam[k]->dCounter);
	    
	  } else if( ( TMath::Abs(delta) >  Width*3./2.) && 
		     ( TMath::Abs(delta) <  Width*3./2.+10.*Width))  {  // +/- 20 beam bunches!
	    double DeltaE = Beam[k]->momentum().Mag() - epair;
	    PStaghEnergyOutOfTime->Fill(-DeltaE);
	    PStaghEOOT->Fill(-DeltaE, Beam[k]->dCounter);
	    OutOfTimePairsH.push_back(pair);
	  }
	}   
      }
    }

    tpair = (PSPairs[0]->ee.first->t + PSPairs[0]->ee.second->t) / 2.;
    epair = (PSPairs[0]->ee.first->E + PSPairs[0]->ee.second->E);

    //loop over hoddoscope out of time hits
    for ( int n=0; n < ((int)OutOfTimeBeamH.size()-1); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamH[n];
      
      if (TMath::Abs( b1->time() - tpair - Width*12.) > Width*4.+Width/2.){
	continue;
      }
      
      // match with other hodoscope hits
      for ( int j=0; j<(int)OutOfTimeBeamH.size(); j++){
	if (j==n){
	  continue;
	}
	
	const DBeamPhoton* b2 = OutOfTimeBeamH[j];
	
	if (TMath::Abs(b1->momentum().Mag() + b2->momentum().Mag() - 11.6)<0.2){
	  continue; // get rid of mollers
	}
	if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter)<8){
	  continue; // get rid of neibouring hits.
	}
	
	double deltat = b1->time() - b2->time();
	outoftimeH->Fill(deltat);
      }
    }
    
    //loop over hoddoscope out of time hits
    for (unsigned int n=0; n<OutOfTimeBeamH.size(); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamH[n];
      if (TMath::Abs( b1->time() - tpair - Width*12.) > Width*4+Width/2.){
	continue;
      }
      
      // match with microscope hits
      for (unsigned int j=0; j<OutOfTimeBeamM.size(); j++){
	const DBeamPhoton* b2 = OutOfTimeBeamM[j];
	if (TMath::Abs(b1->momentum().Mag() + b2->momentum().Mag() - 11.6)>0.2){
	  double deltat = b1->time() - b2->time();
	  outoftimeMIX->Fill(deltat);
	}
      }
    }
    
    // loop over microscope out of time hits
    for (int n=0; n<((int)OutOfTimeBeamM.size()-1); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamM[n];
      if (TMath::Abs( b1->time() - tpair - Width*12.) > Width*4+Width/2.){
	continue;
      }
      
      //match with other microscope out of time hits
      for (int j=0; j<(int)OutOfTimeBeamM.size(); j++){
	if (j==n){
	  continue;
	}
	const DBeamPhoton* b2 = OutOfTimeBeamM[j];
	
	if  (TMath::Abs(b1->momentum().Mag() + b2->momentum().Mag() - 11.6) < 0.2){
	  continue; // get rid of potential mollers
	}
	if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter)<8){
	  continue; // get rid of neibouring hits.
	}
	
	double deltat = b1->time() - b2->time();
	outoftimeM->Fill(deltat);
	
      }
    }

    // look for potential Moller pairs
    for (int n=0; n<((int)OutOfTimeBeamPhotons.size()-1); n++){
      const DBeamPhoton* b1 = OutOfTimeBeamPhotons[n];
      for (int j=n+1; j<(int)OutOfTimeBeamPhotons.size(); j++){
	const DBeamPhoton* b2 = OutOfTimeBeamPhotons[j];
	if (TMath::Abs(b1->time()-b2->time()) < Width/2.){
	  int idx1 = b1->dCounter;
	  int idx2 = b2->dCounter;
	  if (b1->dSystem == SYS_TAGM){
	    idx1+=130;
	  } else {
	    if (idx1>130){
	      idx1 += 70;
	    }
	  }
	  if (b2->dSystem == SYS_TAGM){
	    idx2+=130;
	  } else {
	    if (idx2>130){
	      idx2 += 70;
	    }
	  }
	  
	  correlationC->Fill(idx1, idx2);
	  double E = (11.6-b1->momentum().Mag()) + (11.6-b2->momentum().Mag());
	  correlationE->Fill(E);
	}
      }
    }

    for (int k=0; k<NBeamPhotons; k++){
      const DBeamPhoton* b1 = Beam[k];
      
      if (b1->dSystem != SYS_TAGM){ // look only at microscope
	continue;
      }
      if (PSPairs.size()>0){ 
	float dt1 = b1->time() - tpair;
	fencePS->Fill(dt1);
      }
      float dt1 = b1->time() - tpair;
      fenceRF->Fill(b1->time() - tpair);
      
      if (TMath::Abs(dt1 - (15.*Width))> 6*Width+Width/2.){  // look only at accidentals in bunch +12 pm 7 bunches.
	continue;
      }
      
      for (int n=0; n<NBeamPhotons; n++){
	if (n == k){
	  continue;
	}
	
	const DBeamPhoton* b2 = Beam[n];
	
	float dt2 = b2->time() - tpair;
	if (b2->dSystem != SYS_TAGM){ // look only at microscope
	  continue;
	}
	if (TMath::Abs(dt2)<Width/2.){  // look only at accidentals 
	  continue;
	}
	if (TMath::Abs((int)b1->dCounter - (int)b2->dCounter) < 10 ){
	  continue;
	}
	if (TMath::Abs((int)b1->momentum().Mag() + (int)b2->momentum().Mag() - 11.6) < 0.2 ){
	  continue;
	}
	
	float DT = dt1-dt2;
	acc->Fill(DT, b1->dCounter);
	
      }
    }
  }


  // Any front pannel trigger is random like LED, clock, etc.
  if (trig[0]->fp_trig_mask > 1){ 
    vector <const DBeamPhoton*> Beam;
    event->Get(Beam);
    int NBeamPhotons = Beam.size();

    for (int j=0; j<50; j++) {
      
      double TIME = -16.*RFWidth + j*RFWidth;
      int REF_IDg = -1;
      //cout<<NBeamPhotons<<endl;
      for (int k=0 ;k<NBeamPhotons; k++){
	const DBeamPhoton* g = Beam[k];
	if (g->dSystem == SYS_TAGH){
	  //cout<<g->time()<<endl;
	  if ((TMath::Abs(g->time()-TIME)<2.) && (g->dCounter<120)){
	    REF_IDg = k;
	    break;
	  }
	}
      }
      if (REF_IDg>-1){
	
	double REFTIME = Beam[REF_IDg]->time();
	
	double Ncnts = 0;
	double NcntsA = 0;
	
	for (int k=0 ;k<NBeamPhotons; k++){
	  const DBeamPhoton* g = Beam[k];
	  if (g->dSystem == SYS_TAGM){
	    double dt = g->time()-REFTIME;
	    deltaTall->Fill(dt, j);
	    NcntsA += 1.;
	    int dc = (int)Beam[REF_IDg]->dCounter;
	    //cout<<dc<<endl;
	    if ((dc<100) || (dc>190)){
	      deltaT->Fill(dt, j);
	      Ncnts += 1.;
	    }
	    MICdeltaT->Fill(g->dCounter, dt);
	  }
	}
	
	if (NcntsA>0){
	  deltaTsa->Fill(0., j, 1./(double)NcntsA);
	}
	if (Ncnts>0){
	  deltaTs->Fill(0., j, 1./(double)Ncnts);
	}
      }	
    }
  }
}

//------------------
// EndRun
//------------------
void JEventProcessor_BEAM_online::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_BEAM_online::Finish()
{
	// Called before program exit after event processing is finished.
}


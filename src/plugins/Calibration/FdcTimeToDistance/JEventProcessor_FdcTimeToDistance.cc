// $Id$
//
//    File: JEventProcessor_FdcTimeToDistance.cc
// Created: Tue Mar 10 11:22:46 AM EDT 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.30.1.el9_7.x86_64 x86_64)
//
//  Determines time-to-distance relationship for each FDC plane

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_FdcTimeToDistance.h"
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackFitter.h>
#include <TDirectory.h>
#include <TF1.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FdcTimeToDistance());
}
} // "C"
#include <JANA/Calibrations/JCalibrationManager.h>

//------------------
// JEventProcessor_FdcTimeToDistance (Constructor)
//------------------
JEventProcessor_FdcTimeToDistance::JEventProcessor_FdcTimeToDistance()
{
    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_FdcTimeToDistance (Destructor)
//------------------
JEventProcessor_FdcTimeToDistance::~JEventProcessor_FdcTimeToDistance()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
}

//------------------
// Init
//------------------
void JEventProcessor_FdcTimeToDistance::Init()
{
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();
  lockService->RootWriteLock();
  
  gDirectory->mkdir("FdcDrift")->cd();
  for (unsigned int i=0;i<24;i++){
    char histname[10];
    char histtitle[50];
    sprintf(histname,"HDVsT%d",i);
    sprintf(histtitle,"Time-to-distance for plane %d;t [ns];d [cm]",i+1);
    HDVsT[i]=new TH2F(histname,histtitle,500,-250,250,120,-0.6,0.6);
  }
  gDirectory->cd("../");
  
  lockService->RootUnLock();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FdcTimeToDistance::BeginRun(const std::shared_ptr<const JEvent> &event)
{
  auto run_number = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  bfield = geo_manager->GetBfield(run_number);

  JCalibration *jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
  map<string, double> fdc_drift_parms;
  jcalib->Get("FDC/fdc_drift_parms", fdc_drift_parms);
  FDC_DRIFT_BSCALE_PAR1 = fdc_drift_parms["bscale_par1"];
  FDC_DRIFT_BSCALE_PAR2 = fdc_drift_parms["bscale_par2"];
}

//------------------
// Process
//------------------
void JEventProcessor_FdcTimeToDistance::Process(const std::shared_ptr<const JEvent> &event)
{
  auto tracks=event->Get<DChargedTrack>();
  if (tracks.size()==0) return;
  
  lockService->RootFillLock(this);
  for (auto &track:tracks){
    auto hyp=track->Get_BestTrackingFOM();
    auto trackTB=hyp->Get_TrackTimeBased();
    if (trackTB->FOM>0.01){
      auto pulls=trackTB->pulls;
      if (pulls.size()>10){
	for (auto &pull:pulls){
	  auto fdcptr=pull.fdc_hit;
	  if (fdcptr!=NULL){
	    int layer=fdcptr->wire->layer-1;
	    double t=pull.tdrift;
	    // Apply magnetic field-dependent correction
	    double Bz=bfield->GetBz(fdcptr->xy.X(),fdcptr->xy.Y(),
				    fdcptr->wire->origin.Z());
	    t/=1.+FDC_DRIFT_BSCALE_PAR1+FDC_DRIFT_BSCALE_PAR2*Bz*Bz;
	    double d=pull.d;
	    HDVsT[layer]->Fill((d>0)?t:-t,d);
	  }
	}
      }
    }
  }
  lockService->RootFillUnLock(this);
}

//------------------
// EndRun
//------------------
void JEventProcessor_FdcTimeToDistance::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FdcTimeToDistance::Finish()
{
  TF1 *f1=new TF1("f1","[0]*sqrt(x)+[1]*x+[2]*x*x+[3]*x*x*x",0,200);
  f1->SetParameters(0.026,0.0001,0.000001,0.00000001);

  ofstream out("fparms.dat");
  for (unsigned int m=0;m<24;m++){
    char histname[10];
    sprintf(histname,"HDVsT%d",m);
    TH2F *h=(TH2F*)gDirectory->FindObjectAny(histname);
    h->FitSlicesY();
    sprintf(histname,"HDVsT%d_1",m);
    TH1F *h1=(TH1F*)gDirectory->FindObjectAny(histname);
    h1->GetXaxis()->SetRangeUser(0.,165.);
    h1->Fit(f1,"q");
    out << f1->GetParameter(0) << " " << f1->GetParameter(1)
	<< " " << f1->GetParameter(2) << " " << f1->GetParameter(3) << endl;
  }
}

// $Id$
//
//    File: JEventProcessor_fa125_temp.cc
// Created: Tue Apr 29 02:00:21 PM EDT 2025
// Creator: njarvis (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_fa125_temp.h"

#include <TDirectory.h>
#include <TProfile2D.h>


#include "DAQ/Df125BORConfig.h"
#include "DAQ/bor_roc.h"


static TH2D *htemp;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_fa125_temp());
}
} // "C"


//------------------
// JEventProcessor_fa125_temp (Constructor)
//------------------
JEventProcessor_fa125_temp::JEventProcessor_fa125_temp()
{
    // Parameters and Services should be accessed from Init() instead of here!
	SetTypeName("JEventProcessor_fa125_temp"); // Provide JANA with this class's name
}

//------------------
// ~JEventProcessor_fa125_temp (Destructor)
//------------------
JEventProcessor_fa125_temp::~JEventProcessor_fa125_temp()
{

}

//------------------
// Init
//------------------
void JEventProcessor_fa125_temp::Init()
{
    // This is called once at program startup. 
    
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  histo_filled = 0;
  
  TDirectory *main = gDirectory;
  gDirectory->mkdir("fa125_temp")->cd();

  htemp = new TProfile2D("temp","fa125 temperature (F) (danger point: 185F); roc ; slot", 15, 1, 16, 17, 3, 20);

  for (int i=0; i<70; i++) rocmap[i] = 0;  // rocmap[rocid] = bin number for roc rocid in histogram

  int xlabels[70] = {0};
  int nbins;

  for (int i=25; i<29; i++) {
    int x = i-24;           // CDC, bins 1 to 4
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
  }

  for (int i=52; i<54; i++) {
    int x = i-46;           // FDC, bins 6-7
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
  }

  for (int i=55; i<63; i++) {
    int x = i-47;           // FDC, bins 8-15
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
    nbins = x;
  }

  // ROCs 51,54,63 and 64 are TDCs.

  for (int i=1;i<=nbins;i++) {
    if (xlabels[i]>0) {
         htemp->GetXaxis()->SetBinLabel(i,Form("%i",xlabels[i]));
    } else {
         htemp->GetXaxis()->SetBinLabel(i," ");
    }
  }

  
  main->cd();



    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_fa125_temp::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_fa125_temp::Process(const std::shared_ptr<const JEvent> &event)
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
    // If you have lockService initialized in Init() then you can acquire locks like this
    // lockService->RootFillLock(this);
    //  ... fill histograms or trees ...
    // lockService->RootFillUnLock(this);

  if (histo_filled) return;
  
  auto BORvector = event->Get<Df125BORConfig>();
  uint32_t nb = (uint32_t)BORvector.size();

  if (nb > 0) {
 
    uint32_t rocid, slot;
    Float_t temp;
    
    for (uint32_t i=0; i<nb; i++) {
      const Df125BORConfig *bc = BORvector[i];
    
      temp = 0.1*(Float_t)bc->temperature[0];
      rocid = bc->rocid;
      slot = bc->slot;

      lockService->RootFillLock(this);
      htemp->Fill(rocmap[rocid],(Int_t)slot,temp);
      lockService->RootFillUnLock(this);
    }

    histo_filled = 1;
  }



}

//------------------
// EndRun
//------------------
void JEventProcessor_fa125_temp::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_fa125_temp::Finish()
{
    // Called before program exit after event processing is finished.
}


// $Id$
//
//    File: JEventProcessor_production_check.cc
// Created: Thu May 22 12:12:27 PM EDT 2025
// Creator: ilarin (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_production_check.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_production_check());
}
} // "C"


//------------------
// JEventProcessor_production_check (Constructor)
//------------------
JEventProcessor_production_check::JEventProcessor_production_check()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_production_check (Destructor)
//------------------
JEventProcessor_production_check::~JEventProcessor_production_check()
{
}

//------------------
// Init
//------------------
void JEventProcessor_production_check::Init()
{
    // This is called once at program startup. 
    
//    auto app = GetApplication();
    // lockService should be initialized here like this
    // lockService = app->GetService<JLockService>();
    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_production_check::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_production_check::Process(const std::shared_ptr<const JEvent> &event)
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
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

  ++event_count;

  vector<const DL1Trigger*> l1trig;
  event->Get(l1trig);
  control_word += l1trig.size();

  if(event_count%250!=1) return;

  vector<const DPSCPair*> cpairs;
  event->Get(cpairs);
  vector<const DPSPair*> fpairs;
  event->Get(fpairs);
  control_word2 += cpairs.size() + fpairs.size();

//
	vector<const DBeamPhoton*> beam_photons;
	event->Get(beam_photons);
  control_word += beam_photons.size();

  vector<const DFCALHit*> fcal_hits;
  event->Get(fcal_hits);
  control_word += fcal_hits.size();

  vector<const DBCALHit*> bcal_hits;
  event->Get(bcal_hits);
  control_word += bcal_hits.size();

  vector<const DTOFHit*> tof_hits;
  event->Get(tof_hits);
  control_word += tof_hits.size();

//
	vector<const DChargedTrack*> locChargedTracks;
	event->Get(locChargedTracks, "PreSelect");
  control_word += locChargedTracks.size();

}

//------------------
// EndRun
//------------------
void JEventProcessor_production_check::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_production_check::Finish()
{
// Called before program exit after event processing is finished.

  ofstream  ofr("pcheck.dat");
  ofr << "CRC1 " << control_word << endl;
  ofr << "CRC2 " << control_word2 << endl;
  ofr.close();
}

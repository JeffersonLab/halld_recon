// $Id$
//
//    File: JEventProcessor_bigevents_skim.cc
// Created: Thu May 12 08:01:59 EDT 2016
// Creator: zihlmann (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_bigevents_skim.h"
#include "DANA/DEvent.h"


// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_bigevents_skim());
  }
} // "C"


//------------------
// JEventProcessor_bigevents_skim (Constructor)
//------------------
JEventProcessor_bigevents_skim::JEventProcessor_bigevents_skim()
{
	SetTypeName("JEventProcessor_bigevents_skim");
}

//------------------
// ~JEventProcessor_bigevents_skim (Destructor)
//------------------
JEventProcessor_bigevents_skim::~JEventProcessor_bigevents_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_bigevents_skim::Init()
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
  // lockService->RootWriteLock();
  //  ... fill historgrams or trees ...
  // lockService->RootUnLock();
  //
}

//------------------
// BeginRun
//------------------
void JEventProcessor_bigevents_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_bigevents_skim::Process(const std::shared_ptr<const JEvent>& event)
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

  const DEventWriterEVIO* locEventWriterEVIO = NULL;
  event->GetSingle(locEventWriterEVIO);
  // write out BOR events
  if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
    locEventWriterEVIO->Write_EVIOEvent(event, "bigevents");
    return;
  }
  // write out EPICS events
  if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) {
    locEventWriterEVIO->Write_EVIOEvent(event, "bigevents");
    return;
  }

  // get trigger types
  const DL1Trigger *trig_words = NULL;
  uint32_t trig_mask;
  //uint32_t fp_trig_mask;
  try {
    event->GetSingle(trig_words);
  } catch(...) {};
  if (trig_words) {
    trig_mask = trig_words->trig_mask;
    //fp_trig_mask = trig_words->fp_trig_mask;
  }
  else {
    trig_mask = 0;
    //fp_trig_mask = 0;
  }

  if (!(trig_mask & 0x4)){
    return;
  }

  vector <const DCDCDigiHit*> CDCHits;
  event->Get(CDCHits);

  if (CDCHits.size() > 2000) { // huge event lets keep it!

    locEventWriterEVIO->Write_EVIOEvent(event, "bigevents");

  }

}

//------------------
// EndRun
//------------------
void JEventProcessor_bigevents_skim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_bigevents_skim::Finish()
{
  // Called before program exit after event processing is finished.
}

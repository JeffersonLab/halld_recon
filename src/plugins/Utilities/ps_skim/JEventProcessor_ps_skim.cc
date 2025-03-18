// $Id$
//
//    File: JEventProcessor_ps_skim.cc
// Created: Mon May 18 09:52:08 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_ps_skim.h"

#include "evio_writer/DEventWriterEVIO.h"
#include <TRIGGER/DL1Trigger.h>
#include <DANA/DEvent.h>

// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_ps_skim());
    }
} // "C"


//------------------
// JEventProcessor_ps_skim (Constructor)
//------------------
JEventProcessor_ps_skim::JEventProcessor_ps_skim()
{
	SetTypeName("JEventProcessor_ps_skim");
}

//------------------
// ~JEventProcessor_ps_skim (Destructor)
//------------------
JEventProcessor_ps_skim::~JEventProcessor_ps_skim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_ps_skim::Init()
{
}

//------------------
// BeginRun
//------------------
void JEventProcessor_ps_skim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_ps_skim::Process(const std::shared_ptr<const JEvent>& event)
{
    const DEventWriterEVIO* locEventWriterEVIO = NULL;
    event->GetSingle(locEventWriterEVIO);
    // write out BOR events
    if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {
        locEventWriterEVIO->Write_EVIOEvent(event, "ps");
        return;
    }
    // write out EPICS events
    if(GetStatusBit(event, kSTATUS_EPICS_EVENT)) {
        locEventWriterEVIO->Write_EVIOEvent(event, "ps");
        return;
    }
    // get trigger types
    const DL1Trigger *trig_words = NULL;
    uint32_t trig_mask, fp_trig_mask;
    try {
        event->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    
    
    // skim PS triggers
    int trig_bit = (trig_mask & (1 << 3)) ? 1 : 0;

    if (fp_trig_mask == 0 && trig_bit){
      locEventWriterEVIO->Write_EVIOEvent(event, "ps");
      return;
    }
}

//------------------
// EndRun
//------------------
void JEventProcessor_ps_skim::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_ps_skim::Finish()
{
    // Called before program exit after event processing is finished.
}


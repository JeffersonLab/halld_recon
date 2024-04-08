//
// JEventProcessor_trigger_skims.cc
//
// JANA event processor plugin to skim various trigger types to EVIO files
//
// Author: Sean Dobbs
// Adapted from Ahmed Foda, 13-May-2016 copied from Paul Mattione's 2trackskim

#include "JEventProcessor_trigger_skims.h"
#include "TRIGGER/DL1Trigger.h"
#include "BCAL/DBCALHit.h"
#include "DAQ/DL1Info.h"
#include "DANA/DEvent.h"

// for initializing plugins
extern "C" {
   void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_trigger_skims());
   }
} // "extern C"


// variables to control which triggers get read out
static bool write_out_bcal_led = true;
static bool write_out_fcal_led = true;
static bool write_out_ccal_led = true;
static bool write_out_dirc_led = true;
static bool write_out_random = true;
static bool write_out_sync = true;
static bool write_out_ctof = true;


//-------------------------------
// Init
//-------------------------------
void JEventProcessor_trigger_skims::Init()
{
    auto app = GetApplication();
    int bcal_led_writeout_toggle = 1;
    int fcal_led_writeout_toggle = 1;
    int ccal_led_writeout_toggle = 1;
    int dirc_led_writeout_toggle = 1;
    int random_writeout_toggle = 1;
    int sync_writeout_toggle = 1;
    int ctof_writeout_toggle = 1;

    app->SetDefaultParameter("TRIGSKIM:WRITEBCALLED", bcal_led_writeout_toggle, "Write out BCAL LED events");
    app->SetDefaultParameter("TRIGSKIM:WRITEFCALLED", fcal_led_writeout_toggle, "Write out FCAL LED events");
    app->SetDefaultParameter("TRIGSKIM:WRITECCALLED", fcal_led_writeout_toggle, "Write out CCAL LED events");
    app->SetDefaultParameter("TRIGSKIM:WRITEDIRCLED", fcal_led_writeout_toggle, "Write out DIRC LED events");
    app->SetDefaultParameter("TRIGSKIM:WRITERANDOM", random_writeout_toggle, "Write out random pulser events");
    app->SetDefaultParameter("TRIGSKIM:WRITESYNC", sync_writeout_toggle, "Write out TS sync events");
    app->SetDefaultParameter("TRIGSKIM:WRITECTOF", ctof_writeout_toggle, "Write out CTOF events");

    if(bcal_led_writeout_toggle == 0)
        write_out_bcal_led = false;
    if(fcal_led_writeout_toggle == 0)
        write_out_fcal_led = false;
    if(ccal_led_writeout_toggle == 0)
        write_out_ccal_led = false;
    if(dirc_led_writeout_toggle == 0)
        write_out_dirc_led = false;
    if(random_writeout_toggle == 0)
        write_out_random = false;
    if(sync_writeout_toggle == 0)
        write_out_sync = false;
    if(ctof_writeout_toggle == 0)
        write_out_ctof = false;

    return;
}

//-------------------------------
// BeginRun
//-------------------------------
void JEventProcessor_trigger_skims::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//-------------------------------
// Process
//-------------------------------
void JEventProcessor_trigger_skims::Process(const std::shared_ptr<const JEvent>& locEvent)
{
    // Get EVIO writer 
	const DEventWriterEVIO* locEventWriterEVIO = NULL;
	locEvent->GetSingle(locEventWriterEVIO);

    // Save BOR events
    if(locEvent->GetSingle<DStatusBits>()->GetStatusBit(kSTATUS_BOR_EVENT)) {
        if (write_out_bcal_led)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "BCAL-LED" );
        if (write_out_ccal_led)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "CCAL-LED" );
        if (write_out_fcal_led)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "FCAL-LED" );
        if (write_out_dirc_led)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "DIRC-LED" );
        if (write_out_random)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "random" );
        if (write_out_sync)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "sync" );
        if (write_out_ctof)
            locEventWriterEVIO->Write_EVIOEvent( locEvent, "ctof" );
        return;
    }

	// Save EPICS events
	vector<const DEPICSvalue*> locEPICSValues;
	locEvent->Get(locEPICSValues);
	if(!locEPICSValues.empty()) {
        if (write_out_bcal_led)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "BCAL-LED");
        if (write_out_ccal_led)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "CCAL-LED");
        if (write_out_fcal_led)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "FCAL-LED");
        if (write_out_dirc_led)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "DIRC-LED");
        if (write_out_random)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "random");
        if (write_out_ctof)
            locEventWriterEVIO->Write_EVIOEvent(locEvent, "ctof");
		return;
	}

	//bool is_cosmic_trigger = false;
	bool is_BCAL_LED_US_trigger = false;
	bool is_BCAL_LED_DS_trigger = false;
	bool is_CCAL_LED_trigger = false;
	bool is_FCAL_LED_trigger = false;
	bool is_DIRC_LED_trigger = false;
	bool is_random_trigger = false;
	bool is_sync_event = false;
	bool is_ctof_event = false;
    
	const DL1Trigger *trig = NULL;
	try {
		locEvent->GetSingle(trig);
	} catch (...) {}

    // parse the triggers we want to save
	if (trig) {
		//printf("%5i  %5i | %5i  %5i  %5i | %i\n",
		//	   trig->trig_mask,trig->trig_mask & 0x1,
		//	   trig->fp_trig_mask, trig->fp_trig_mask & 0x100,trig->fp_trig_mask & 0x200,
		//	   trig->trig_mask && trig->fp_trig_mask);

        /* -- commented out to remove warning
		if (trig->trig_mask & 0x1) {
			// Cosmic trigger fired
			is_cosmic_trigger = true;
		}
        */
        
        // Select triggers based on front panel inputs
        // Trigger bits start counting from 0
		if (trig->fp_trig_mask & 0x100) {   // Trigger front-panel bit 8
			// Upstream BCAL LED trigger fired
			is_BCAL_LED_US_trigger = true;
		}
		if (trig->fp_trig_mask & 0x200) {   // Trigger front-panel bit 9
			// Downstream BCAL LED trigger fired
			is_BCAL_LED_DS_trigger = true;
		}
		if (trig->fp_trig_mask & 0x800) {  // Trigger front-panel bit 11
			// Periodic pulser trigger fired
			is_random_trigger = true;
		}
		if (trig->fp_trig_mask & 0x004) {   // Trigger front-panel bit 2
			// FCAL LED trigger fired
			is_FCAL_LED_trigger = true;
		}
		if (trig->fp_trig_mask & 0x010) {   // Trigger front-panel bit 5
			// CCAL LED trigger fired
			//is_CCAL_LED_trigger = true;

		}
		if (trig->fp_trig_mask & 0x020) {   // Trigger front-panel bit 6
			// CCAL LED trigger fired
			//is_CCAL_LED_trigger = true;

            // CPP run - this is now the CTOF trigger
            is_ctof_event = true;
		}
		if (trig->fp_trig_mask & 0x4000 ) {   // Trigger front-panel bit 15
			// DIRC LED trigger fired
			is_DIRC_LED_trigger = true;
		}
	} 

    // Do some backup calculations for runs in which the BCAL LED trigger did not latch correctly
    vector<const DBCALHit *> bcal_hits;
    locEvent->Get(bcal_hits);
    double total_bcal_energy = 0.;
    if(write_out_bcal_led) {
        for(unsigned int i=0; i<bcal_hits.size(); i++) {
            total_bcal_energy += bcal_hits[i]->E;
        }
    }

    // if there's a DL1Info object, this extra L1 info means that it's a "sync event"
    vector<const DL1Info*> l1_info;
    locEvent->Get(l1_info);
    if(l1_info.size() == 1) {
        is_sync_event = true;
    }

    // Save events to skim file

    // Save BCAL trigger if:
    // 1. Trigger front-panel bits 8 or 9
    // 2. Total energy in BCAL > 12 GeV (diabled to avoid GTP events leaking into BCAL LED skims)
    // 3. Number of hits in BCAL > 1200 (raised to 1200 to avoid GTP events leaking into BCAL LED skims)
    bool save_BCAL_LED_event = is_BCAL_LED_US_trigger || is_BCAL_LED_DS_trigger
      || (bcal_hits.size() >= 1200);// || (total_bcal_energy > 12.); Diabling energy trigger
    if ( write_out_bcal_led && save_BCAL_LED_event ) {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "BCAL-LED");
    }
    if ( write_out_ccal_led && is_CCAL_LED_trigger ) {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "CCAL-LED");
    }
    if ( write_out_fcal_led && is_FCAL_LED_trigger ) {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "FCAL-LED");
    }
    if ( write_out_dirc_led && is_DIRC_LED_trigger ) {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "DIRC-LED");
    }
    if ( write_out_random && is_random_trigger ) {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "random");
    }
    if ( write_out_sync && is_sync_event )  {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "sync");
    }
    if ( write_out_ctof && is_ctof_event )  {
      locEventWriterEVIO->Write_EVIOEvent(locEvent, "ctof");
    }
    
    return;
}

//-------------------------------
// EndRun
//-------------------------------
void JEventProcessor_trigger_skims::EndRun()
{
}

//-------------------------------
// Finish
//-------------------------------
void JEventProcessor_trigger_skims::Finish()
{
}


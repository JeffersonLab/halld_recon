#include "DTrigger_factory.h"
#include <bitset>
#include "BCAL/DBCALHit.h"
#include "FCAL/DFCALHit.h"

#include <JANA/JEvent.h>


//------------------
// Init
//------------------
void DTrigger_factory::Init()
{
	auto app = GetApplication();

	EMULATE_FCAL_LED_TRIGGER = false;
	EMULATE_BCAL_LED_TRIGGER = false;

    EMULATE_CAL_ENERGY_SUMS = true;
	
	BCAL_LED_NHITS_THRESHOLD = 200;
	FCAL_LED_NHITS_THRESHOLD = 200;

	string locUsageString = "Set BCAL LED front panel trigger bits if such events leak into other triggers (1/0, off by default)";
	app->SetDefaultParameter("TRIGGER:EMULATE_BCAL_LED_TRIGGER", EMULATE_BCAL_LED_TRIGGER, locUsageString);
	locUsageString = "Set FCAL LED front panel trigger bits if such events leak into other triggers (1/0, off by default)";
	app->SetDefaultParameter("TRIGGER:EMULATE_FCAL_LED_TRIGGER", EMULATE_FCAL_LED_TRIGGER, locUsageString);
	locUsageString = "Calculate calorimter energy sums using emulated triggers (1/0, off by default)";
	app->SetDefaultParameter("TRIGGER:EMULATE_CAL_ENERGY_SUMS", EMULATE_CAL_ENERGY_SUMS, locUsageString);

}




//------------------
// Process
//------------------
void DTrigger_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DL1Trigger*> locL1Triggers;
	event->Get(locL1Triggers);
	const DL1Trigger* locL1Trigger = locL1Triggers.empty() ? NULL : locL1Triggers[0];

	DTrigger *locTrigger = new DTrigger;
	
	//SET LEVEL-1 TRIGGER INFO
	if(locL1Trigger != NULL)
	{
	        locTrigger->Set_L1TriggerBits(locL1Trigger->trig_mask);
		uint32_t locFpTrigMask = locL1Trigger->fp_trig_mask;
	
		// Sometimes the BCAL/FCAL LED trigger also trip the main physics trigger,
		// due to hardware problems, afterglow, etc...
		// These events are generally easy to identify due to their extremely large
		// occupancies in the calorimeters.  So, we allow the option to exclude
		// these events by setting the trigger bits when it looks like they are firing
		
		if(EMULATE_BCAL_LED_TRIGGER) {
		    vector<const DBCALHit *> bcal_hits;
			event->Get(bcal_hits);
			
			if( bcal_hits.size() > BCAL_LED_NHITS_THRESHOLD) {
				// naively, not sure if this due to the up- or down-stream LEDs, so pick the upstream one
				locFpTrigMask |= 0x100;
			}
		}
		
		if(EMULATE_FCAL_LED_TRIGGER) {
		    vector<const DFCALHit *> fcal_hits;
			event->Get(fcal_hits);
			
			if( fcal_hits.size() > FCAL_LED_NHITS_THRESHOLD) {
				locFpTrigMask |= 0x004;
			}
		}
		
		locTrigger->Set_L1FrontPanelTriggerBits(locFpTrigMask);

        // use realistic trigger simulation to calculate what the BCAL & FCAL energies
        // used in the trigger decision were - hopefully this is good enough
        // eventually we'll get the values directly from the firmware
        if(EMULATE_CAL_ENERGY_SUMS) {
            vector<const DL1MCTrigger*> locMCTriggers;
            event->Get(locMCTriggers, "DATA");
            const DL1MCTrigger* locMCTrigger = locMCTriggers.empty() ? NULL : locMCTriggers[0];

            if(locMCTrigger != NULL)
            {
                locTrigger->Set_GTP_BCALEnergy(locMCTrigger->bcal_gtp_en);
                locTrigger->Set_GTP_FCALEnergy(locMCTrigger->fcal_gtp_en);
		locTrigger->Set_GTP_ECALEnergy(locMCTrigger->ecal_gtp_en);
		locTrigger->Set_GTP_FCAL2Energy(locMCTrigger->fcal2_gtp_en);
            }
        }

	}
	else 
    {
        // IF TRIGGER INFO IS NOT IN THE DATA STREAM, LOAD TRIGGER SIMULATIONS LAZILY
        
        locTrigger->Set_L1TriggerBits(0);
        locTrigger->Set_L1FrontPanelTriggerBits(0); 

        // cerr << " event status = " << std::bitset<32>(locEventLoop->GetJEvent().GetStatus()) << endl;

        // don't bother simulating the trigger for non-physics events
        // for now, just don't run this for EVIO (raw data) events
        auto status_bits = event->GetSingle<DStatusBits>();
        if( status_bits->GetStatusBit(kSTATUS_EVIO) ){
            Insert(locTrigger);
            return;
        }

        // realistic trigger simulation
        vector<const DL1MCTrigger*> locMCTriggers;
        event->Get(locMCTriggers);
        const DL1MCTrigger* locMCTrigger = locMCTriggers.empty() ? NULL : locMCTriggers[0];
	
        if(locMCTrigger != NULL)
        {        
            //IS MC DATA: USE SIMULATED TRIGGER INFORMATION IF AVAILABLE
            locTrigger->Set_L1TriggerBits(locMCTrigger->trig_mask);
            locTrigger->Set_L1FrontPanelTriggerBits(0);
            locTrigger->Set_GTP_BCALEnergy(locMCTrigger->bcal_gtp_en);
            locTrigger->Set_GTP_FCALEnergy(locMCTrigger->fcal_gtp_en);
	    locTrigger->Set_GTP_ECALEnergy(locMCTrigger->ecal_gtp_en);
	    locTrigger->Set_GTP_FCAL2Energy(locMCTrigger->fcal2_gtp_en);

        }
    }

	//SET LEVEL-3 TRIGGER INFO HERE

	Insert(locTrigger);
}

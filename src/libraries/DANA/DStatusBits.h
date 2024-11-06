// $Id$
//
//    File: DStatusBits.h
// Created: Fri Nov 20 09:58:15 EST 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

// This class is used to set descriptions for the basic set of
// event status bits used in Hall-D. It is here rather than 
// being embedded as part of DApplication so that it can be used 
// with JEventSource_EVIO without having to tie in the whole
// DApplication class. This is so the DAQ plugin can be used 
// with generic JANA executables that don't have DApplication
// linked in.
//
// The status bits are contained in a uint64_t so there are
// 64 bits available. The first 16 should be reserved for
// things defined inside this file. The remaning 48 are
// available for other things like flagging certain topologies.

#ifndef _DStatusBits_
#define _DStatusBits_

#include <JANA/Compatibility/JStatusBits.h>

// Used for status flags set for each event
enum StatusBitType{
	kSTATUS_HDDM,
	kSTATUS_REST,
	kSTATUS_EVIO,
	kSTATUS_CODA,
	kSTATUS_CDAQ,
	kSTATUS_FROM_FILE,
	kSTATUS_FROM_ET,
	kSTATUS_CONTROL_EVENT,
	kSTATUS_PHYSICS_EVENT,
	kSTATUS_EPICS_EVENT,
	kSTATUS_SYNC_EVENT,
	kSTATUS_BOR_EVENT,
	
	kSTATUS_L3PASS = 12,
	kSTATUS_L3FAIL,
	
	kSTATUS_FCAL_PI0 = 16,
	kSTATUS_FBCAL_PI0,
	kSTATUS_PI0,
	kSTATUS_1TRACK,
	kSTATUS_2TRACK,
	kSTATUS_3TRACK,
	
	kSTATUS_NONE = 31
};



class DStatusBits : public JStatusBits<StatusBitType> {
	public:

		static void SetStatusBitDescriptions() {
			SetStatusBitDescription( kSTATUS_HDDM,          "HDDM file" );
			SetStatusBitDescription( kSTATUS_REST,          "REST file" );
			SetStatusBitDescription( kSTATUS_EVIO,          "EVIO" );
			SetStatusBitDescription( kSTATUS_CODA,          "CODA event recorder" );
			SetStatusBitDescription( kSTATUS_CDAQ,          "CDAQ event recorder" );
			SetStatusBitDescription( kSTATUS_FROM_FILE,     "Event read from file" );
			SetStatusBitDescription( kSTATUS_FROM_ET,       "Event read from ET system" );
			SetStatusBitDescription( kSTATUS_CONTROL_EVENT, "Control event" );
			SetStatusBitDescription( kSTATUS_PHYSICS_EVENT, "Physics event" );
			SetStatusBitDescription( kSTATUS_EPICS_EVENT,   "EPICS event" );
			SetStatusBitDescription( kSTATUS_SYNC_EVENT,    "SYNC event" );
			SetStatusBitDescription( kSTATUS_BOR_EVENT,     "Beginning Of Run (BOR) event") ;

			SetStatusBitDescription( kSTATUS_L3PASS,        "L3 Pass") ;
			SetStatusBitDescription( kSTATUS_L3FAIL,        "L3 Fail") ;
		}
};

#endif // _DStatusBits_


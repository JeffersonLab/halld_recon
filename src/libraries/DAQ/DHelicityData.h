// $Id$
// $HeadURL$
//
//    File: DHelicityData.h
//
// This data type corresponds to the "helicity decoder" board
// that provides real-time information about the electron beam helicity
// The data format is described in https://halldweb.jlab.org/DocDB/0065/006593/001/Programming%20the%20Helicity%20Decoder%20Module%20V9.pdf
//


#ifndef _DHelicityData_
#define _DHelicityData_

#include <DAQ/DDAQAddress.h>

class DHelicityData:public DDAQAddress{
	
	
	public:
		JOBJECT_PUBLIC(DHelicityData);

		DHelicityData(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0 
			, uint32_t expected_helicity_state=0
			, uint32_t recovered_helicity_seed=0
			, uint32_t falling_edge_tstable_count=0
			, uint32_t rising_edge_tstable_count=0
			, uint32_t pattern_sync_count=0
			, uint32_t pair_sync_count=0
			, uint32_t time_from_start_tstable=0
			, uint32_t time_from_end_tstable=0
			, uint32_t duration_last_tstable=0
			, uint32_t duration_last_tsettle=0
			, bool     trigger_tstable=false
			, bool     trigger_pattern_sync=false
			, bool     trigger_pair_sync=false
			, bool     trigger_helicity_state=false
			, bool     trigger_helicity_state_pattern_start=false
			, bool     trigger_event_polarity=false
			, uint32_t trigger_pattern_phase_count=0
			, uint32_t last_pattern_sync=0
			, uint32_t last_pair_sync=0
			, uint32_t last_helicity_state=0
			, uint32_t last_helicity_state_pattern_sync=0)
			:DDAQAddress(rocid, slot, channel, itrigger)
			, expected_helicity_state(expected_helicity_state)
			, recovered_helicity_seed(recovered_helicity_seed)
			, falling_edge_tstable_count(falling_edge_tstable_count)
			, rising_edge_tstable_count(rising_edge_tstable_count)
			, pattern_sync_count(pattern_sync_count)
			, pair_sync_count(pair_sync_count)
			, time_from_start_tstable(time_from_start_tstable)
			, time_from_end_tstable(time_from_end_tstable)
			, duration_last_tstable(duration_last_tstable)
			, duration_last_tsettle(duration_last_tsettle)
			, trigger_tstable(trigger_tstable)
			, trigger_pattern_sync(trigger_pattern_sync)
			, trigger_pair_sync(trigger_pair_sync)
			, trigger_helicity_state(trigger_helicity_state)
			, trigger_helicity_state_pattern_start(trigger_helicity_state_pattern_start)
			, trigger_event_polarity(trigger_event_polarity)
			, trigger_pattern_phase_count(trigger_pattern_phase_count)
			, last_pattern_sync(last_pattern_sync)
			, last_pair_sync(last_pair_sync)
			, last_helicity_state(last_helicity_state)
			, last_helicity_state_pattern_sync(last_helicity_state_pattern_sync) {}

		// data
		uint32_t expected_helicity_state;
		uint32_t recovered_helicity_seed;
		uint32_t falling_edge_tstable_count;
		uint32_t rising_edge_tstable_count;
		uint32_t pattern_sync_count;
		uint32_t pair_sync_count;
		uint32_t time_from_start_tstable;
		uint32_t time_from_end_tstable;
		uint32_t duration_last_tstable;
		uint32_t duration_last_tsettle;
		bool     trigger_tstable;
		bool     trigger_pattern_sync;
		bool     trigger_pair_sync;
		bool     trigger_helicity_state;
		bool     trigger_helicity_state_pattern_start;
		bool     trigger_event_polarity;
		uint32_t trigger_pattern_phase_count;
		uint32_t last_pattern_sync;
		uint32_t last_pair_sync;
		uint32_t last_helicity_state;
		uint32_t last_helicity_state_pattern_sync;


		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
			summary.add(expected_helicity_state, NAME_OF(expected_helicity_state), "%d");
			summary.add(pattern_sync_count, NAME_OF(pattern_sync_count), "%d");
			summary.add(pair_sync_count, NAME_OF(pair_sync_count), "%d");
			summary.add(time_from_start_tstable, NAME_OF(time_from_start_tstable), "%d");
		}

};

#endif // _DHelicityData_


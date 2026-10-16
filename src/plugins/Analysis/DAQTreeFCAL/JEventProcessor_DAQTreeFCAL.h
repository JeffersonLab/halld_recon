// $Id$
//
//    File: JEventProcessor_DAQTreeFCAL.h
// Created: Thu Jan  9 12:29:31 PM EST 2025
// Creator: dalton (on Linux ifarm2402.jlab.org 5.14.0-427.42.1.el9_4.x86_64 x86_64)
//

#ifndef _JEventProcessor_DAQTreeFCAL_
#define _JEventProcessor_DAQTreeFCAL_

#include <JANA/JEventProcessor.h>

#include <TTree.h>

class JEventProcessor_DAQTreeFCAL:public JEventProcessor{
	public:
		JEventProcessor_DAQTreeFCAL();
		~JEventProcessor_DAQTreeFCAL();
		const char* className(void){return "JEventProcessor_DAQTreeFCAL";}

                TTree *FCALdigi;
		uint32_t channelnum;         ///< Arbitrary global channel number (sorted by crate, slot, channel)
		uint32_t eventnum;	     ///< Event number	
		uint32_t rocid;              ///< (from DDAQAddress) Crate number
		uint32_t slot;               ///< (from DDAQAddress) Slot number in crate
		uint32_t channel;            ///< (from DDAQAddress) Channel number in slot
		uint32_t itrigger;           ///< (from DDAQAddress) Trigger number for cases when this hit was read in a multi-event block (from DDAQAddress)
		uint32_t peak;               ///< pulse_peak
		uint32_t peak_raw;               ///< pulse_peak
		uint32_t integral;           ///< pulse_integral
                uint32_t pedestal;
		uint32_t time;               ///< pulse_time
                uint32_t nsamples;           ///< nsamples_integral
		uint32_t QF;                 ///< Quality Factor from FPGA algorithms
		int row;
		int column;

	private:
		void Init();						///< Called once at program start.
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;	///< Called everytime a new run number is detected.
		void Process(const std::shared_ptr<const JEvent>& event) override;	///< Called every event.
		void EndRun();						///< Called everytime run number changes, provided brun has been called.
		void Finish();						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_DAQTreeFCAL_


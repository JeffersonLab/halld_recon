// $Id$
//
//    File: JEventProcessor_DAQTreeBCAL.h
// Created: Mon May  5 15:20:49 EDT 2014
// Creator: davidl (on Darwin harriet.jlab.org 13.1.0 i386)
//

#ifndef _JEventProcessor_DAQTreeBCAL_
#define _JEventProcessor_DAQTreeBCAL_

#include <stdint.h>

#include <JANA/JEventProcessor.h>

#include <TTree.h>


class JEventProcessor_DAQTreeBCAL:public JEventProcessor{
	public:
		JEventProcessor_DAQTreeBCAL();
		~JEventProcessor_DAQTreeBCAL();

		TTree *BCALdigi;
		TTree *BCALTDCdigi;
		uint32_t channelnum;         ///< Arbitrary global channel number (sorted by crate, slot, channel)
		uint32_t eventnum;	         ///< Event number	
		uint32_t rocid;              ///< (from DDAQAddress) Crate number
		uint32_t slot;               ///< (from DDAQAddress) Slot number in crate
		uint32_t channel;            ///< (from DDAQAddress) Channel number in slot
		uint32_t itrigger;           ///< (from DDAQAddress) Trigger number for cases when this hit was read in a multi-event block (from DDAQAddress)
		vector<uint32_t> waveform;   ///< STL vector of samples in the waveform for the event
		uint32_t nsamples;           ///< Number of samples in the waveform
		uint32_t w_integral;         ///< Sum of all samples in the waveform
		uint32_t w_min;              ///< Minimum sample in the waveform
		uint32_t w_max;              ///< Maximum sample in the waveform
		uint32_t w_samp1;            ///< First sample in the waveform  (for simple analysis in case the STL vector is difficult to access)

		uint32_t time;

		uint32_t module;
		uint32_t layer;
		uint32_t sector;
		uint32_t end;  // 0=upstream, 1=downstream

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_DAQTreeBCAL_


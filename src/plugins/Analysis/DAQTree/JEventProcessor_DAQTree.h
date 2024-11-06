// $Id$
//
//    File: JEventProcessor_DAQTree.h
// Created: Tue Oct 22 14:55:40 EDT 2013
// Creator: dalton (on Linux gluon45.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_DAQTree_
#define _JEventProcessor_DAQTree_

#include <JANA/JEventProcessor.h>
#include "TH1D.h"
#include "TTree.h"

#include <stdint.h>

// Doxygen documentation
/** 
 This plugin is designed to make root trees of low level data from the DAQ.
 Each low level data type will have its own tree in the file.
 Each event and each channel is a new entry in the tree.

 Trees will be named after the low level data type used to fill them.
 Currently the data types that are supported are: \n
   Df125WindowRawData \n
   Df125PulseRawData \n
   Df125PulseIntegral \n
   Df125PulseTime \n
   Df125PulsePedestal \n
   Df125TriggerTime \n
   DF1TDCHit \n
   DF1TDCTriggerTime \n
   Df250WindowRawData \n
   Df250PulseRawData \n
   Df250PulseIntegral \n
   Df250PulseTime \n
   Df250PulsePedestal \n
   Df250TriggerTime \n

 The data types that might be added in future: \n
   Df250StreamingRawData \n
   Df250WindowSum \n

 Wiki documentation can be found here: https://halldweb1.jlab.org/wiki/index.php/DAQTree_plugin
*/

class JEventProcessor_DAQTree:public JEventProcessor{
	public:
		JEventProcessor_DAQTree();
		~JEventProcessor_DAQTree();

		TTree *Df125WindowRawData_tree;  ///< f125 readout 
		TTree *Df125PulseRawData_tree;   ///< f125 readout mode 2
		TTree *Df125PulseIntegral_tree;  ///< f125 readout modes 3 and 7
		TTree *Df125PulseTime_tree;      ///< f125 readout modes 3,4,7 and 8
		TTree *Df125PulsePedestal_tree;  ///< f125 readout modes 4,7 and 8
		TTree *Df125TriggerTime_tree;    ///< all f125 readout modes 
		TTree *DF1TDCHit_tree;
		TTree *DF1TDCTriggerTime_tree;
		TTree *Df250WindowRawData_tree;  ///< f250 readout modes 1 and 8
		TTree *Df250PulseRawData_tree;   ///< f250 readout mode 2
		TTree *Df250PulseIntegral_tree;  ///< f250 readout modes 3 and 7
		TTree *Df250PulseTime_tree;      ///< f250 readout modes 3,4,7 and 8
		TTree *Df250PulsePedestal_tree;  ///< f250 readout modes 4,7 and 8
		TTree *Df250TriggerTime_tree;    ///< all f250 readout modes 

		uint32_t channelnum;         ///< Arbitrary global channel number (sorted by crate, slot, channel).  Note that when data is sparsified then this value will not have a constant relationship with any particular physical channel.
		uint32_t eventnum;	         ///< Event number	
		uint32_t rocid;              ///< (from DDAQAddress) Crate number
		uint32_t slot;               ///< (from DDAQAddress) Slot number in crate
		uint32_t channel;            ///< (from DDAQAddress) Channel number in slot
		uint32_t itrigger;           ///< (from DDAQAddress) Trigger number for cases when this hit was read in a multi-event block (from DDAQAddress)
		std::vector<uint32_t> waveform;   ///< STL vector of samples of the waveform for the event\n for f125WRD, f250WRD, f250PRD
		uint32_t nsamples;           ///< Number of samples extracted from the waveform\n for f125WRD, f250WRD, f250PRD
		uint32_t w_integral;         ///< Sum of all samples extracted from the waveform\n for f125WRD, f250WRD, f250PRD
		uint32_t w_min;              ///< Minimum sample extracted from the waveform\n for f125WRD, f250WRD, f250PRD
		uint32_t w_max;              ///< Maximum sample extracted from the waveform\n for f125WRD, f250WRD, f250PRD
		uint32_t w_samp1;            ///< First sample extracted from the waveform  (for simple analysis in case the STL vector is difficult to access)\n for f250WRD, f250PRD
		uint32_t w_ped;              ///< the sum of the first 10 samples extracted from the waveform for use as a pedestal\n for f125WRD, f250WRD, Always = 0 for f250PRD
		Float_t w_time;              ///< the time (in samples) of the pulse calculated similar to the IU method\n for f125WRD, f250WRD, f250PRD
		uint32_t pulse_number;         /// \n for f250PRD, f250PI, f250PT
		uint32_t quality_factor;       /// \n for f250PI, f250PT
		int32_t integral;              /// \n for f250PI 
		int32_t pedestal;              /// \n for f250PI and f250PP
		uint32_t nsamples_integral;              /// \n for f250PI 
		uint32_t nsamples_pedestal;              /// \n for f250PI and f250PP
		uint32_t pulse_peak;           /// \n for f250PP
		uint32_t time;                 /// \n for f250PT, f250TT, F1TDCH
		uint32_t first_sample_number;  /// \n for f250PRD
		bool invalid_samples;          /// \n for f250WRD, f250PRD
		bool overflow;                 /// \n for f250WRD, f250PRD
		uint32_t trig_time;            /// \n for F1TDCH
		uint32_t data_word;            /// \n for F1TDCH

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		bool f125WRDtree_exists;
		bool f125PRDtree_exists;
		bool f125PItree_exists;
		bool f125PTtree_exists;
		bool f125PPtree_exists;
		bool f125TTtree_exists;
		bool F1TDCHtree_exists;
		bool F1TDCTTtree_exists;
		bool f250WRDtree_exists;
		bool f250PRDtree_exists;
		bool f250PItree_exists;
		bool f250PTtree_exists;
		bool f250PPtree_exists;
		bool f250TTtree_exists;

};

#endif // _JEventProcessor_DAQTree_

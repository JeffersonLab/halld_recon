// $Id$
//
//    File: DL3Trigger_factory.h
// Created: Wed Jul 31 14:34:24 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DL3Trigger_factory_
#define _DL3Trigger_factory_

#include <mutex>
using std::mutex;

#include <JANA/JFactoryT.h>
#include "DL3Trigger.h"

#include <RtypesCore.h>

#ifdef HAVE_TMVA
#include <TMVA/Reader.h>
#endif

class DL3Trigger_factory:public JFactoryT<DL3Trigger>{
	public:
		DL3Trigger_factory(){};
		~DL3Trigger_factory(){};

		double FRACTION_TO_KEEP;
		bool DO_WIRE_BASED_TRACKING;
		bool DO_BCAL_CLUSTER;
		uint32_t L1_TRIG_MASK;
		uint32_t L1_FP_TRIG_MASK;
		string MVA_WEIGHTS;
		double MVA_CUT;
		
		
#ifdef HAVE_TMVA
		TMVA::Reader *mvareader=nullptr;
#endif
		Float_t Nstart_counter;
		Float_t Ntof;
		Float_t Nbcal_points;
		Float_t Nbcal_clusters;
		Float_t Ebcal_points;
		Float_t Ebcal_clusters;
		Float_t Nfcal_clusters;
		Float_t Efcal_clusters;
		Float_t Ntrack_candidates;
		Float_t Ptot_candidates;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DL3Trigger_factory_


// $Id: DEventProcessor_acceptance_hists.h 1816 2006-06-06 14:38:18Z davidl $
//
//    File: DEventProcessor_acceptance_hists.h
// Created: Sun Apr 24 06:45:21 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DEventProcessor_acceptance_hists_
#define _DEventProcessor_acceptance_hists_

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

class DEventProcessor_acceptance_hists:public JEventProcessor{

	public:
		DEventProcessor_acceptance_hists();
		~DEventProcessor_acceptance_hists();
		
		TH2F *CDC, *FDC, *CDC_FDC;
		TH2F *BCAL, *FCAL, *TOF;
		TH2F *thrown_charged, *thrown_photon;
		
		TH1D *FDC_anode_hits_per_event;
		TH1D *FDC_anode_hits_per_layer;
		TH1D *FDC_anode_hits_per_wire;
		
		TH1D *CDC_nhits_vs_pthrown;
		TH1D *FDC_nhits_vs_pthrown;
		TH1D *pthrown;

	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

};

#endif // _DEventProcessor_acceptance_hists_


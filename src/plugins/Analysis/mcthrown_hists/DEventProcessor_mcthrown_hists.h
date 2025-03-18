// $Id: DEventProcessor_mcthrown_hists.h 1816 2006-06-06 14:38:18Z davidl $
//
//    File: DEventProcessor_mcthrown_hists.h
// Created: Sun Apr 24 06:45:21 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DEventProcessor_mcthrown_hists_
#define _DEventProcessor_mcthrown_hists_

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

class DEventProcessor_mcthrown_hists:public JEventProcessor{

	public:
		DEventProcessor_mcthrown_hists();
		~DEventProcessor_mcthrown_hists();
		
		TH1F *pmom, *theta, *phi, *energy;
		TH2F *pmom_vs_theta;
		TH2F *pmom_vs_theta_pip;
		TH2F *pmom_vs_theta_pim;
		TH2F *pmom_vs_theta_Kp;
		TH2F *pmom_vs_theta_Km;
		TH2F *pmom_vs_theta_proton;
		TH2F *pmom_vs_theta_neutron;
		TH2F *pmom_vs_theta_gamma;
		TH1F *Nparticles_per_event, *particle_type;
		TH3F *vertex;
		
	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

};

#endif // _DEventProcessor_mcthrown_hists_


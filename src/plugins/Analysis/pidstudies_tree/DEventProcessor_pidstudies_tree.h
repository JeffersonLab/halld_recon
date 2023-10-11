// $Id$
//
//    File: DEventProcessor_pidstudies_tree.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_pidstudies_tree_
#define _DEventProcessor_pidstudies_tree_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TTree.h>
#include <DVector3.h>
#include <particleType.h>
#include <map>

#include <TRACKING/DMCThrown.h>
#include <PID/DChargedTrack.h>
#include <MCReconstructionStatuses.h>

class DEventProcessor_pidstudies_tree:public JEventProcessor{
	public:
		DEventProcessor_pidstudies_tree(){};
		~DEventProcessor_pidstudies_tree(){};
		SetTypeName("DEventProcessor_pidstudies_tree");

		double Calc_MatchFOM(const DVector3& locMomentum_Track1, const DVector3& locMomentum_Track2) const;

		class plugin_trackmatch_t {
			public:
				plugin_trackmatch_t(){}

				double dFOM;
				const DMCThrown* dMCThrown;
				const DChargedTrack* dChargedTrack;
		};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		vector<plugin_trackmatch_t*> dTrackMatches;
		MCReconstructionStatuses *dMCReconstructionStatuses;

		TTree* dPluginTree_MCReconstructionStatuses;
};

#endif // _DEventProcessor_pidstudies_tree_


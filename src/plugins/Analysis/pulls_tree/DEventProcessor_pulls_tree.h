// $Id$
//
//    File: DEventProcessor_pulls_tree.h
// Created: Fri Feb 19 13:04:29 EST 2010
// Creator: davidl (on Darwin harriet.jlab.org 9.8.0 i386)
//

#ifndef _DEventProcessor_pulls_tree_
#define _DEventProcessor_pulls_tree_

#include <TTree.h>

#include <JANA/JEventProcessor.h>
#include <TRACKING/DTrackFitter.h>

#include <pull_t.h>

class DEventProcessor_pulls_tree:public JEventProcessor{
	public:
		DEventProcessor_pulls_tree();
		~DEventProcessor_pulls_tree();

		void RecalculateChisq(DTrackFitter::fit_type_t fit_type, const DKinematicData *kd, double &chisq, int &Ndof, vector<DTrackFitter::pull_t> &pulls);

		TTree *pullsWB, *pullsTB;
		pull_t pullWB, pullTB;
		pull_t *pullWB_ptr, *pullTB_ptr;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		pthread_mutex_t mutex;
		
		bool RECALCULATE_CHISQ;
		const DTrackFitter *fitter;
};

#endif // _DEventProcessor_pulls_tree_


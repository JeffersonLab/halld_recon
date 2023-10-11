// $Id$
//
//    File: JEventProcessor_dedx_tree.h
// Created: Wed Aug 28 14:48:25 EDT 2019
// Creator: njarvis (on Linux albert.phys.cmu.edu 3.10.0-693.5.2.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_dedx_tree_
#define _JEventProcessor_dedx_tree_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DVertex.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"

#include "ANALYSIS/DTreeInterface.h"

#include <vector>


class JEventProcessor_dedx_tree:public JEventProcessor{
	public:
		JEventProcessor_dedx_tree();
		~JEventProcessor_dedx_tree();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

                //TREE
                DTreeInterface* dTreeInterface;

                //thread_local: Each thread has its own object: no lock needed
                //important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;


};

#endif // _JEventProcessor_dedx_tree_


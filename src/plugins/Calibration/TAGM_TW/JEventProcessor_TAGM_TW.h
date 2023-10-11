// $Id$
//
//    File: JEventProcessor_TAGM_TW.h
// Created: Thu Aug  6 07:33:06 EDT 2015
// Creator: pooser (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_TW_
#define _JEventProcessor_TAGM_TW_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TAGGER/DTAGMHit.h"
#include "TAGGER/DTAGMGeometry.h"

class JEventProcessor_TAGM_TW:public JEventProcessor{
	public:
		JEventProcessor_TAGM_TW();
		~JEventProcessor_TAGM_TW();

	private:
		// For the timewalk
		double tw_c0[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];
		double tw_c1[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];
		double tw_c2[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];
		double thresh[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];
		double P_0[TAGM_MAX_ROW+1][TAGM_MAX_COLUMN+1];

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGM_TW_


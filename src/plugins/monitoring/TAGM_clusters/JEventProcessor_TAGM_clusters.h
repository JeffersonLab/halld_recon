// $Id$
//
//    File: JEventProcessor_TAGM_clusters.h
// Created: Tue Jul  5 21:19:22 EDT 2016
// Creator: barnes (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_clusters_
#define _JEventProcessor_TAGM_clusters_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <TAGGER/DTAGMHit.h>

class JEventProcessor_TAGM_clusters:public JEventProcessor{
	public:
		JEventProcessor_TAGM_clusters();
		~JEventProcessor_TAGM_clusters();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGM_clusters_


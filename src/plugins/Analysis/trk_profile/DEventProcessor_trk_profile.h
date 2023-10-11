// $Id$
//
//    File: DEventProcessor_trk_profile.h
// Created: Wed Jan 12 08:02:32 EST 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.6.0 i386)
//

#ifndef _DEventProcessor_trk_profile_
#define _DEventProcessor_trk_profile_

#include <JANA/JEventProcessor.h>

class DEventProcessor_trk_profile:public JEventProcessor{
	public:
		DEventProcessor_trk_profile();
		~DEventProcessor_trk_profile();
		SetTypeName("DEventProcessor_trk_profile");

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DEventProcessor_trk_profile_


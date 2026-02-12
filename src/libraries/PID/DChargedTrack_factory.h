// $Id$
//
//    File: DChargedTrack_factory.h
// Created: Mon Dec  7 14:29:24 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DChargedTrack_factory_
#define _DChargedTrack_factory_

#include <JANA/JFactoryT.h>
#include <PID/DChargedTrack.h>
#include <TRACKING/DTrackTimeBased.h>

class DChargedTrack_factory:public JFactoryT<DChargedTrack>{
	public:
		DChargedTrack_factory(){};
		~DChargedTrack_factory(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DChargedTrack_factory_


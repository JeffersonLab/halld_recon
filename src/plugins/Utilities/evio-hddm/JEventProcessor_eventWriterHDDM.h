// $Id$
//
//    File: JEventProcessor_eventWriterHDDM.h
// Created: Mon Mar  6 14:11:50 EST 2017
// Creator: tbritton (on Linux halld03.jlab.org 3.10.0-514.6.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_eventWriterHDDM_
#define _JEventProcessor_eventWriterHDDM_

#include <JANA/JEventProcessor.h>
#include <HDDM/DEventWriterHDDM.h>

class JEventProcessor_eventWriterHDDM:public JEventProcessor{
	public:
		JEventProcessor_eventWriterHDDM();
		~JEventProcessor_eventWriterHDDM();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_eventWriterHDDM_


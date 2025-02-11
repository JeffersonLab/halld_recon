// $Id$
//
//    File: JEventProcessor_PhiSkim.h
// Created: Wed May 24 13:46:12 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#ifndef _JEventProcessor_PhiSkim_
#define _JEventProcessor_PhiSkim_

#include <JANA/JEventProcessor.h>

class JEventProcessor_PhiSkim:public JEventProcessor{
public:
		JEventProcessor_PhiSkim();
		~JEventProcessor_PhiSkim();
		const char* className(void){return "JEventProcessor_PhiSkim";}
  
private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		int WRITE_EVIO_FILE;
		int WRITE_ROOT_TREE;
};

#endif // _JEventProcessor_PhiSkim_


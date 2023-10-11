// $Id$
//
//    File: JEventProcessor_OmegaSkim.h
// Created: Wed May 24 13:46:12 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#ifndef _JEventProcessor_OmegaSkim_
#define _JEventProcessor_OmegaSkim_

#include <JANA/JEventProcessor.h>

class JEventProcessor_OmegaSkim:public JEventProcessor{
public:
		JEventProcessor_OmegaSkim();
		~JEventProcessor_OmegaSkim();

private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
  
  int WRITE_EVIO_FILE;
  int WRITE_ROOT_TREE;
};

#endif // _JEventProcessor_OmegaSkim_


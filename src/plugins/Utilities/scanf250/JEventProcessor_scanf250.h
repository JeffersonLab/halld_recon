// $Id$
//
//    File: JEventProcessor_scanf250.h
// Created: Tue Oct  9 21:26:13 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_scanf250_
#define _JEventProcessor_scanf250_

#include <JANA/JEventProcessor.h>

#include <TTree.h>
#include "ANALYSIS/DTreeInterface.h"

class JEventProcessor_scanf250:public JEventProcessor{
	public:
		JEventProcessor_scanf250();
		~JEventProcessor_scanf250();

	private:


                //TREE
                DTreeInterface* dTreeInterface;

                //thread_local: Each thread has its own object: no lock needed
                //important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;





		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_scanf250_


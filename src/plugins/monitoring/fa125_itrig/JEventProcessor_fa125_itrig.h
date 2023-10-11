// $Id$
//
//    File: JEventProcessor_fa125_itrig.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_fa125_itrig_
#define _JEventProcessor_fa125_itrig_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>



class JEventProcessor_fa125_itrig:public JEventProcessor{
	public:
		JEventProcessor_fa125_itrig();
		~JEventProcessor_fa125_itrig();

		//                TTree *cdctree;

	private:

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;


                bool MAKE_TREE;

                int rocmap[70];   // which bin to use for each roc in the histogram, to make the histogram compact, not gappy
};

#endif // _JEventProcessor_fa125_itrig_


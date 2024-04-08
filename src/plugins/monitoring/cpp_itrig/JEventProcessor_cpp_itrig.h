// $Id$
//
//    File: JEventProcessor_cpp_itrig.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cpp_itrig_
#define _JEventProcessor_cpp_itrig_

#include <JANA/JEventProcessor.h>



class JEventProcessor_cpp_itrig:public JEventProcessor{
	public:
		JEventProcessor_cpp_itrig();
		~JEventProcessor_cpp_itrig();
		const char* className(void){return "JEventProcessor_cpp_itrig";}

		//                TTree *cdctree;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
        
		bool MAKE_TREE;

        int rocmap[81];   // which bin to use for each roc in the histogram, to make the histogram compact, not gappy
};

#endif // _JEventProcessor_cpp_itrig_


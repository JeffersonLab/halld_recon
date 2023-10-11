// $Id$
//
//    File: DMCThrown_factory_FinalState.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DMCThrown_factory_FinalState_
#define _DMCThrown_factory_FinalState_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>

#include <TRACKING/DMCThrown.h>

#include "ANALYSIS/DAnalysisUtilities.h"

using namespace std;


class DMCThrown_factory_FinalState : public JFactoryT<DMCThrown>
{
	public:
		DMCThrown_factory_FinalState(){
			SetTag("FinalState");
		}; //prevents JANA from searching the input file for these objects
		~DMCThrown_factory_FinalState(){};

	private:

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		const DAnalysisUtilities* dAnalysisUtilities;
};

#endif // _DMCThrown_factory_FinalState_


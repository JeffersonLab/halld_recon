// $Id$
//
//    File: DEventRFBunch_factory_Thrown.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DEventRFBunch_factory_Thrown_
#define _DEventRFBunch_factory_Thrown_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <RF/DRFTime.h>
#include <TRACKING/DMCThrown.h>

using namespace std;


class DEventRFBunch_factory_Thrown : public JFactoryT<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_Thrown(){
			SetTag("Thrown");
		};
		~DEventRFBunch_factory_Thrown(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DEventRFBunch_factory_Thrown_


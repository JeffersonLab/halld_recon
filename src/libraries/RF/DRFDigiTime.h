// $Id$
//
//    File: DRFDigiTime.h
// Created: Mon Mar 30 10:51:39 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//

#ifndef _DRFDigiTime_
#define _DRFDigiTime_

#include <JANA/JObject.h>

#include "DAQ/DModuleType.h"
#include "GlueX.h"

using namespace std;

class DRFDigiTime : public JObject
{
	public:
		JOBJECT_PUBLIC(DRFDigiTime);

		DetectorSystem_t dSystem;
		uint32_t time;


		void Summarize(JObjectSummary& summary) const override {
			summary.add(SystemName(dSystem), "System", "%s");
			summary.add(time, NAME_OF(time), "%u");
		}
};

#endif // _DRFDigiTime_


// $Id$
//
//    File: DF1TDCBORConfig.h
// Created: Tue Jan 26 13:04:46 EST 2016
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DF1TDCBORConfig_
#define _DF1TDCBORConfig_

#include <JANA/JObject.h>

#include <DAQ/bor_roc.h>

// This class inherits both from JObject and F1TDCconfig. The former
// so that it can be incorporated easily into the JANA framework.
// The latter so we can use the data struct defined in bor_roc.h.
// The file bor_roc.h exists in 2 places:
//
//  1. in the DAQ library of sim-recon
//  2. in the vme/src/rcm/monitor directory in the online
//


class DF1TDCBORConfig:public JObject, public F1TDCconfig{
	public:
		JOBJECT_PUBLIC(DF1TDCBORConfig);

		DF1TDCBORConfig(){}
		virtual ~DF1TDCBORConfig(){}


		void Summarize(JObjectSummary& summary) const override {
			summary.add(rocid, NAME_OF(rocid), "%d");
			summary.add(slot, NAME_OF(slot), "%d");
			summary.add(version, NAME_OF(version), "0x%x");
			summary.add(ctrl, NAME_OF(ctrl), "0x%x");
			summary.add(blocklevel, NAME_OF(blocklevel), "%d");
			summary.add(nchips, NAME_OF(nchips), "%d");
		}

};

#endif // _DF1TDCBORConfig_


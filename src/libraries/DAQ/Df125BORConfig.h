// $Id$
//
//    File: Df125BORConfig.h
// Created: Tue Jan 26 13:04:46 EST 2016
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _Df125BORConfig_
#define _Df125BORConfig_

#include <JANA/JObject.h>

#include <DAQ/bor_roc.h>

// This class inherits both from JObject and f125config. The former
// so that it can be incorporated easily into the JANA framework.
// The latter so we can use the data struct defined in bor_roc.h.
// The file bor_roc.h exists in 2 places:
//
//  1. in the DAQ library of sim-recon
//  2. in the vme/src/rcm/monitor directory in the online
//


class Df125BORConfig:public JObject, public f125config{
	public:
		JOBJECT_PUBLIC(Df125BORConfig);

		Df125BORConfig(){}
		virtual ~Df125BORConfig(){}

		void Summarize(JObjectSummary& summary) const override {
			summary.add(rocid, NAME_OF(rocid), "%d");
			summary.add(slot, NAME_OF(slot), "%d");
			summary.add(board_id, "id", "0x%x");
			summary.add(version, NAME_OF(version), "0x%x");
			summary.add(proc_version, NAME_OF(proc_version), "0x%x");
			summary.add(ctrl1, NAME_OF(ctrl1), "0x%x");
			summary.add(proc_blocklevel, NAME_OF(proc_blocklevel), "%d");
		}
};

#endif // _Df125BORConfig_


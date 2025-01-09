// $Id$
//
//    File: DCAEN1290TDCBORConfig.h
// Created: Tue Jan 26 13:04:46 EST 2016
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DCAEN1290TDCBORConfig_
#define _DCAEN1290TDCBORConfig_

#include <JANA/JObject.h>

#include <DAQ/bor_roc.h>

// This class inherits both from JObject and caen1190config. The former
// so that it can be incorporated easily into the JANA framework.
// The latter so we can use the data struct defined in bor_roc.h.
// The file bor_roc.h exists in 2 places:
//
//  1. in the DAQ library of sim-recon
//  2. in the vme/src/rcm/monitor directory in the online
//


class DCAEN1290TDCBORConfig : public JObject, public caen1190config{
	public:
		JOBJECT_PUBLIC(DCAEN1290TDCBORConfig);

		DCAEN1290TDCBORConfig(){}
		virtual ~DCAEN1290TDCBORConfig(){}
		
        void Summarize(JObjectSummary& summary) const override {
            summary.add(rocid, NAME_OF(rocid), "%d");
            summary.add(slot, NAME_OF(slot), "%d");
			summary.add(firmwareRev, NAME_OF(firmwareRev), "0x%x");
			summary.add(edge_resolution, NAME_OF(edge_resolution), "%d");
			summary.add(almostFullLevel, NAME_OF(almostFullLevel), "%d");
		}

};

#endif // _DCAEN1290TDCBORConfig_


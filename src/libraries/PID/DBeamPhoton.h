// $Id$
//
//    File: DBeamPhoton.h
// Created: Thu Feb 14 10:11:52 EST 2008
// Creator: davidl (on Darwin fwing-dhcp13.jlab.org 8.10.1 i386)
//

#ifndef _DBeamPhoton_
#define _DBeamPhoton_

#include <PID/DKinematicData.h>

class DBeamPhoton: public DKinematicData
{
	public:
		JOBJECT_PUBLIC(DBeamPhoton);

		oid_t id = reinterpret_cast<oid_t>(this);
		unsigned int dCounter = 0;
		DetectorSystem_t dSystem = SYS_NULL; //SYS_TAGM or SYS_TAGH (SYS_NULL if tag MCGEN and misses the tagger!)

		void Summarize(JObjectSummary& summary) const override {
			summary.add(momentum().Mag(), "E(GeV)", "%f");
			summary.add(SystemName(dSystem), "System", "%s");
			summary.add(dCounter, "Counter", "%d");
			summary.add(time(), "t(ns)", "%3.1f");
		}
};



#endif // _DBeamPhoton_


// $Id$
//
//    File: DMCThrown.h
// Created: Sun Apr  3 12:22:09 EDT 2005
// Creator: davidl (on Darwin Harriet.local 7.8.0 powerpc)
//

#ifndef _DMCThrown_
#define _DMCThrown_

#include <JANA/JObject.h>
#include <DANA/DObjectID.h>
#include "PID/DKinematicData.h"

class DMCThrown:public DKinematicData{
	public:
		JOBJECT_PUBLIC(DMCThrown);

		oid_t id = reinterpret_cast<oid_t>(this);
		int type;			///< GEANT particle ID
		int pdgtype;		///< PDG particle type (not used by GEANT)
		int myid;			///< id of this particle from original generator
		int parentid;		///< id of parent of this particle from original generator
		int mech;			///< production mechanism of this partcle (generator specific)

		void Summarize(JObjectSummary& summary) const override {
			DKinematicData::Summarize(summary);
			summary.add(pdgtype, NAME_OF(pdgtype), "%d");
			summary.add(myid, NAME_OF(myid), "%d");
			summary.add(parentid, NAME_OF(parentid), "%d");
			summary.add(mech, NAME_OF(mech), "%d");
		}
};

#endif // _DMCThrown_


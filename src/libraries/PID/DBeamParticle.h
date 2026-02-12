// $Id$
//

#ifndef _DBeamParticle_
#define _DBeamParticle_

#include <PID/DKinematicData.h>
#include <PID/DBeamPhoton.h>
#include <PID/DBeamKLong.h>

class DBeamParticle : public DKinematicData
{
	public:
		JOBJECT_PUBLIC(DBeamParticle);

		oid_t id = reinterpret_cast<oid_t>(this);

		DBeamParticle() : DKinematicData() {}
		DBeamParticle(const DKinematicData& locSourceData) : DKinematicData(locSourceData) {}

		const DBeamPhoton *dBeamPhoton = nullptr;
		const DBeamKLong  *dBeamKLong = nullptr;

		void Summarize(JObjectSummary& summary) const override {
			if(dBeamPhoton != nullptr)  dBeamPhoton->Summarize(summary);
			if(dBeamKLong != nullptr)   dBeamKLong->Summarize(summary);
		}
};



#endif // _DBeamParticle_


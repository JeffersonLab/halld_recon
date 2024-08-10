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

		DBeamParticle() : DKinematicData() {}
		DBeamParticle(const DKinematicData& locSourceData) : DKinematicData(locSourceData) {}

		const DBeamPhoton *dBeamPhoton = nullptr;
		const DBeamKLong  *dBeamKLong = nullptr;

		void toStrings(vector<pair<string,string> > &items)const{
			if(dBeamPhoton != nullptr)  dBeamPhoton->toStrings(items);
			if(dBeamKLong != nullptr)   dBeamKLong->toStrings(items);
		}
};



#endif // _DBeamParticle_


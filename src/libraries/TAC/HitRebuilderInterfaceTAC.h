/*
 * HitBuilderInterfaceTAC.h
 *
 *  Created on: Jun 23, 2017
 *      Author: Hovanes Egiyan
 */

#ifndef LIBRARIES_TAC_HITREBUILDERINTERFACETAC_H_
#define LIBRARIES_TAC_HITREBUILDERINTERFACETAC_H_

#include <vector>
#include <set>

#include <JANA/JEvent.h>
#include <JANA/Compatibility/jerror.h>
#include <DAQ/Df250WindowRawData.h>
#include <TAC/DTACHit.h>

//class DTACHit;
//class Df250WindowRawData;
//class JEventLoop;

class HitRebuilderInterfaceTAC {
protected:
	virtual double getTimeFromRawData(const std::vector<uint16_t>& samples) = 0;
	virtual const Df250WindowRawData* getRawData(const DTACHit* baseHit) = 0;
	jerror_t readCCDB(const std::shared_ptr<const JEvent>& event) = delete;

public:
	HitRebuilderInterfaceTAC() {
	}
	virtual ~HitRebuilderInterfaceTAC() {
	}
	virtual std::vector<DTACHit*> operator()(
			std::vector<const DTACHit*>& baseHitVector) = 0;
};

#endif /* LIBRARIES_TAC_HITREBUILDERINTERFACETAC_H_ */

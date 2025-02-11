/*
 * HitRebuilderTAC.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Hovanes Egiyan
 */

#ifndef LIBRARIES_TAC_HITREBUILDERTAC_H_
#define LIBRARIES_TAC_HITREBUILDERTAC_H_

#include <vector>
#include <set>
#include <stdexcept>

#include <JANA/Compatibility/jerror.h>
#include <JANA/JFactoryT.h>
#include <DAQ/Df250WindowRawData.h>

#include "DTACDigiHit.h"
#include "DTACTDCDigiHit.h"
#include "DTACHit.h"

#include "HitRebuilderInterfaceTAC.h"

class HitRebuilderTAC : virtual public HitRebuilderInterfaceTAC {
protected:
	double timeScaleADC = 4.0;
	double adcTimeOffset = 0;
	double timeBaseADC = -130;

	double adcTimeRescaleFactor = 64.0;

	std::set<const Df250WindowRawData*> rawDataPtrSet;

//	static std::string tagString;

	double getTimeFromRawData(const std::vector<uint16_t>& samples) override;
	const Df250WindowRawData* getRawData(const DTACHit* baseHit) override;

	jerror_t readCCDB( const std::shared_ptr<const JEvent>& event );

//	static std::string& setTagString( std::string tag ) { return (tagString = tag ); }
public:
//	HitRebuilderTAC(double timeScale, double timeOffset, double timeBase  ) :
//			timeScaleADC(timeScale), adcTimeOffset(timeOffset), timeBaseADC(
//					timeBase) {
//	}
	HitRebuilderTAC( const std::shared_ptr<const JEvent>& event ) : HitRebuilderInterfaceTAC() {
		HitRebuilderTAC::readCCDB( event );
	}

	HitRebuilderTAC(const HitRebuilderTAC& f) :HitRebuilderInterfaceTAC(),
			timeScaleADC(f.timeScaleADC), adcTimeOffset(f.adcTimeOffset), timeBaseADC(
					f.timeBaseADC) {
	}
	HitRebuilderTAC& operator=(const HitRebuilderTAC& f) {
		if (this != &f) {
			timeScaleADC = f.timeScaleADC;
			adcTimeOffset = f.adcTimeOffset;
			timeBaseADC = f.timeBaseADC;
		}
		return *this;
	}
	virtual ~HitRebuilderTAC() {
		// TODO Auto-generated destructor stub
	}

	virtual std::vector<DTACHit*> operator()(
			std::vector<const DTACHit*>& baseHitVector) override ;

	static std::string getTagString() {
		return "REBUILD";
	}

	double getADCTimeOffset() const {
		return adcTimeOffset;
	}

	void setADCTimeOffset(double adcTimeOffset = 0) {
		this->adcTimeOffset = adcTimeOffset;
	}

	const std::set<const Df250WindowRawData*>& getRawDataPtrSet() const {
		return rawDataPtrSet;
	}

	void setRawDataPtrSet(
			const std::set<const Df250WindowRawData*>& rawDataPtrSet) {
		this->rawDataPtrSet = rawDataPtrSet;
	}

	double getTimeBaseADC() const {
		return timeBaseADC;
	}

	void setTimeBaseADC(double timeBaseADC = -130) {
		this->timeBaseADC = timeBaseADC;
	}

	double getTimeScaleADC() const {
		return timeScaleADC;
	}

	void setTimeScaleADC(double timeScaleADC = 4.0) {
		this->timeScaleADC = timeScaleADC;
	}
};

#endif /* LIBRARIES_TAC_HITREBUILDERTAC_H_ */

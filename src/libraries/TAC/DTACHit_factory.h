/*
 * DTACHit_factory.h
 *
 *  Created on: Mar 24, 2017
 *      Author: hovanes
 */

#ifndef LIBRARIES_TAC_DTACHIT_FACTORY_H_
#define LIBRARIES_TAC_DTACHIT_FACTORY_H_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include <JANA/Compatibility/jerror.h>

#include <DANA/ReadWriteLock.h>
#include <TTAB/DTTabUtilities.h>
#include <DAQ/Df250PulseRawData.h>

#include "DTACDigiHit.h"
#include "DTACTDCDigiHit.h"
#include "DTACHit.h"

class DTACHit_factory: public JFactoryT<DTACHit> {
protected:
	double energyScale = 1.0;
	double timeScaleADC = 0.0625;          // ns
	double timeBaseADC = -130.0;          // ns
	double timeBaseTDC = -130;          // ns

	// calibration constants stored by channel
	double energyGain = 1.0;
	double adcPedestal = 100;
	double adcTimeOffset = 0;
	double tdcTimeOffsets = 0;
	vector<double> timeWalkParameters { 9.27503, -0.548155, 20.48000, 328.0000 };

	static bool checkErrorsOnFADC;
	static bool useTimeWalkCorrections;
	static double timeWindowTDC;
	static double timeDifferencInADCandTDC;

	static set<int> announcedRuns;

public:
	DTACHit_factory() {
		// TODO Auto-generated constructor stub
	}
	virtual ~DTACHit_factory() {
		// TODO Auto-generated destructor stub
	}

	void Init() override;
	void BeginRun(const std::shared_ptr<const JEvent> &aEvent) override;
	void Process(const std::shared_ptr<const JEvent>& aEvent) override;
	void EndRun() override;
	void Finish() override;

	std::string getTagString() { return ""; }

	virtual void Reset_Data(void);
	virtual void AppendData( DTACHit* hit ) { Insert(hit) ;};
//	virtual std::string SetTag( std::string tag ) ;

	virtual jerror_t readCCDB( const std::shared_ptr<const JEvent>& loop );

	virtual DTACHit* findMatch(double tdcTime);


	virtual void makeFADCHits(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber);
	virtual void makeTDCHits(const std::shared_ptr<const JEvent>& loop, uint64_t eventnumber);

	static bool errorCheckIsNeededForFADC() {
		return checkErrorsOnFADC;
	}
	static bool addRun(int runNumber) {
		static pthread_mutex_t runListMutex = PTHREAD_MUTEX_INITIALIZER;
		bool returnValue = false;
		PlainLock printLock(runListMutex);
		if (announcedRuns.count(runNumber) == 0) {
			announcedRuns.insert(runNumber);
			returnValue = true;
		}
		return returnValue;
	}

	double getADCPedestal() const {
		return adcPedestal;
	}

	void setADCPedestal(double adcPedestal = 100) {
		this->adcPedestal = adcPedestal;
	}

	double getADCTimeOffset() const {
		return adcTimeOffset;
	}

	void setADCTimeOffset(double adcTimeOffset = 0) {
		this->adcTimeOffset = adcTimeOffset;
	}

	double getEnergyGain() const {
		return energyGain;
	}

	void setEnergyGain(double energyGain = 1.0) {
		this->energyGain = energyGain;
	}

	double getEnergyScale() const {
		return energyScale;
	}

	void setEnergyScale(double energyScale = 1.0) {
		this->energyScale = energyScale;
	}

	double getTDCTimeOffsets() const {
		return tdcTimeOffsets;
	}

	void setTDCTimeOffsets(double tdcTimeOffsets = 0) {
		this->tdcTimeOffsets = tdcTimeOffsets;
	}

	double getTimeBaseADC() const {
		return timeBaseADC;
	}

	void setTimeBaseADC(double timeBaseAdc = -130.0) {
		timeBaseADC = timeBaseAdc;
	}

	double getTimeBaseTDC() const {
		return timeBaseTDC;
	}

	void setTimeBaseTDC(double timeBaseTdc = -130) {
		timeBaseTDC = timeBaseTdc;
	}

	double getTimeScaleADC() const {
		return timeScaleADC;
	}

	void setTimeScaleADC(double timeScaleAdc = 0.0625) {
		timeScaleADC = timeScaleAdc;
	}

	const std::vector<double>& getTimeWalkParameters() const {
		return timeWalkParameters;
	}

	void setTimeWalkParameters(const std::vector<double>& timeWalkParameters) {
		this->timeWalkParameters = timeWalkParameters;
	}
};

#endif /* LIBRARIES_TAC_DTACHIT_FACTORY_H_ */

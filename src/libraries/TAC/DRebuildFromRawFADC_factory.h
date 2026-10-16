/*
 * DRebuildFromRawFADC_factory.h
 *
 *  Created on: May 26, 2017
 *      Author: Hovanes Egiyan
 */

#ifndef LIBRARIES_DREBUILDFROMRAWFADC_FACTORY_H_
#define LIBRARIES_DREBUILDFROMRAWFADC_FACTORY_H_

#include <vector>
#include <set>
using namespace std;

#include <JANA/JFactoryT.h>

#include <DANA/ReadWriteLock.h>
#include <TTAB/DTTabUtilities.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>

#include "DTACDigiHit.h"
#include "DTACTDCDigiHit.h"
#include "DTACHit.h"

#include <TAC/HitRebuilderTAC.h>

#include <TAC/DTACHit_factory.h>

// We will use this factory to create TAC hits with "REBUILD" tag using the existing
// hits and the raw data. This method will skip the making a new DigiHit
// class and simply replace some of the hit info by reanalyzing the FADC250
// waveform. this class is not supposed to be considered as "another" way
// of getting a TACHit since it depends on the default TACHit building
// factory for the main part of the information contained on the TACHit objects
// produced.

template<typename BASE_FACTORY = DTACHit_factory,
		typename BUILDER = HitRebuilderTAC>
class DRebuildFromRawFADC_factory: public BASE_FACTORY {
protected:
	BUILDER* hitBuilder = nullptr;
	std::string fTag;
//	std::set<const Df250WindowRawData*> rawDataPtrSet;

//	virtual jerror_t rebuildHit(const std::shared_ptr<const JEvent>& event,
//			uint64_t eventNumber, const DTACHit* baseHit);
//	virtual const Df250WindowRawData* getRawData(const DTACHit* baseHit);
//	virtual double getTimeFromRawData(const vector<uint16_t>& samples);
public:
	DRebuildFromRawFADC_factory() : BASE_FACTORY() {
		this->SetTag(BASE_FACTORY::getTagString() + BUILDER::getTagString());
	}
	virtual ~DRebuildFromRawFADC_factory() {
	}

	void Init() override;
	void BeginRun(const std::shared_ptr<const JEvent>& event) override;
	void Process(const std::shared_ptr<const JEvent>& event) override;
	void EndRun() override;
	void Finish() override;
};

using namespace std;

template<typename BASE_FACTORY, typename BUILDER>
void DRebuildFromRawFADC_factory<BASE_FACTORY, BUILDER>::Init() {
	BASE_FACTORY::Init();
//	this->setTimeScaleADC(this->getTimeScaleADC() * 64);          // Time scale for a single FADC hit
}

template<typename BASE_FACTORY, typename BUILDER>
void DRebuildFromRawFADC_factory<BASE_FACTORY, BUILDER>::BeginRun(
		const std::shared_ptr<const JEvent>& event) {
	BASE_FACTORY::BeginRun(event);
//	hitBuilder = new BUILDER(this->getTimeScaleADC(), this->getADCTimeOffset(),
//			this->getTimeBaseADC());
	hitBuilder = new BUILDER( event );
}

// override the event processing method
template<typename BASE_FACTORY, typename BUILDER>
void DRebuildFromRawFADC_factory<BASE_FACTORY, BUILDER>::Process(
		const std::shared_ptr<const JEvent>& event) {

//	cout << "In DTACHit_Rebuild_factory::evnt" << endl;
//
//	cout << "Data structure has " << _data.size() << " elements" << endl;
	vector<const DTACHit*> baseHitVector;
	event->Get(baseHitVector, "");

//	// Declare comparison functor for this TAC hits basedon pulse peak
//	static auto compareHits =
//			[](const DTACHit* lhs, const DTACHit* rhs ) ->
//			bool {return( (lhs!=nullptr)&(rhs!=nullptr ) ? fabs(lhs->getPulsePeak()) > fabs(rhs->getPulsePeak() ) : false );};
//
//	std::sort(baseHitVector.begin(), baseHitVector.end(), compareHits);
//
////	rawDataPtrSet = new set<const Df250WindowRawData*>();
//	for (auto baseHit : baseHitVector) {
//		rebuildHit(event, eventNumber, baseHit);
//		vector<DTACHit*> newHitVec = BUILDER( baseHit );
//	}
//	cout << "Here are all the " << _data.size() << " new hits in the rebuild factory" << endl;
//	for( auto hit : _data ) {
//		vector<pair<string,string>> outputStrings;
//
//		hit->toStrings( outputStrings );
//		cout << endl << "Rebuild hit at " << hit  << endl;
//		for( auto& outputPair :  outputStrings ) {
//			cout << outputPair.first << "  = " << outputPair.second << endl;
//		}
//	}
//	if( rawDataPtrSet != 0 ) {
//		delete rawDataPtrSet;
//	}

//	rawDataPtrSet.clear();

	vector<DTACHit*> newHitVector = (*hitBuilder)(baseHitVector);
	for (auto newHit : newHitVector) {
		this->AppendData(newHit);
//		Insert(newHit);
	}

//	cout << "Done rebuilding TACHit " << endl;
}

//------------------
// EndRun
//------------------
template<typename BASE_FACTORY, typename BUILDER>
void DRebuildFromRawFADC_factory<BASE_FACTORY, BUILDER>::EndRun() {
	if (hitBuilder != nullptr) {
		delete hitBuilder;
		hitBuilder = nullptr;
	}
	BASE_FACTORY::EndRun();
}

//------------------
// Finish
//------------------
template<typename BASE_FACTORY, typename BUILDER>
void DRebuildFromRawFADC_factory<BASE_FACTORY, BUILDER>::Finish() {
	BASE_FACTORY::Finish();
}


//// Declare specializations
//
//class WaveformErfcFunctor;
//template <typename T> class HitRebuilderByFit;
//
//
//template<>
//jerror_t DRebuildFromRawFADC_factory<DTACHit_factory,
//		HitRebuilderByFit<WaveformErfcFunctor>>::init(void);
//template<>
//jerror_t DRebuildFromRawFADC_factory<DTACHit_factory,
//		HitRebuilderByFit<WaveformErfcFunctor>>::brun(
//		const std::shared_ptr<const JEvent>& event, int32_t runnumber);

#endif /* LIBRARIES_DREBUILDFROMRAWFADC_FACTORY_H_ */

// $Id$
//
//    File: DChargedTrackHypothesis_factory.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DChargedTrackHypothesis_factory_
#define _DChargedTrackHypothesis_factory_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DDetectorMatches.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DParticleID.h>
#include <TOF/DTOFPoint.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include "DResourcePool.h"

using namespace std;
using namespace jana;

class DChargedTrackHypothesis_factory:public jana::JFactory<DChargedTrackHypothesis>
{
	public:
		DChargedTrackHypothesis* Create_ChargedTrackHypothesis(JEventLoop* locEventLoop, const DTrackTimeBased* locTrackTimeBased, const DDetectorMatches* locDetectorMatches, const DEventRFBunch* locEventRFBunch);
		void Add_TimeToTrackingMatrix(DChargedTrackHypothesis* locChargedTrackHypothesis, TMatrixFSym* locCovarianceMatrix, double locFlightTimeVariance, double locHitTimeVariance, double locFlightTimePCorrelation) const;

		void Recycle_Hypotheses(vector<const DChargedTrackHypothesis*>& locHypos){dResourcePool_ChargedTrackHypothesis->Recycle(locHypos);}
		void Recycle_Hypotheses(vector<DChargedTrackHypothesis*>& locHypos){dResourcePool_ChargedTrackHypothesis->Recycle(locHypos);}
		void Recycle_Hypothesis(const DChargedTrackHypothesis* locHypo){dResourcePool_ChargedTrackHypothesis->Recycle(locHypo);}

		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_ChargedTrackHypothesis->Get_NumObjectsAllThreads();}
		DChargedTrackHypothesis* Get_Resource(void)
		{
			auto locHypo = dResourcePool_ChargedTrackHypothesis->Get_Resource();
			return locHypo;
		}

	private:
		const DParticleID* dPIDAlgorithm;

		//RESOURCE POOL
		//For some reason, JANA doesn't call factory destructor until AFTER the threads have been closed
		//This causes the pool destructor to crash.  Instead, delete in fini();
		vector<DChargedTrackHypothesis*> dCreated;
		DResourcePool<DChargedTrackHypothesis>* dResourcePool_ChargedTrackHypothesis = nullptr;
		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t fini(void)
		{
			for(auto locHypo : _data)
				Recycle_Hypothesis(locHypo);
			_data.clear();
			delete dResourcePool_ChargedTrackHypothesis;
			return NOERROR;
		}

		// CDC space charge correction for dEdx
		vector<vector<Float_t>>CDC_DEDX_AMP_CORRECTION;
		Float_t cdc_min_theta, cdc_max_theta;
		Float_t cdc_min_dedx, cdc_max_dedx;
		Float_t cdc_theta_step, cdc_dedx_step;
		Int_t cdc_npoints_theta, cdc_npoints_dedx;
		Float_t Correct_CDC_dEdx_amp(Float_t theta_deg, Float_t thisdedx);
};

#endif // _DChargedTrackHypothesis_factory_


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

#include <JANA/JFactoryT.h>
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DDetectorMatches.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DParticleID.h>
#include <TOF/DTOFPoint.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include "DResourcePool.h"

using namespace std;


class DChargedTrackHypothesis_factory:public JFactoryT<DChargedTrackHypothesis>
{
	public:
		DChargedTrackHypothesis* Create_ChargedTrackHypothesis(const std::shared_ptr<const JEvent>& event, const DTrackTimeBased* locTrackTimeBased, const DDetectorMatches* locDetectorMatches, const DEventRFBunch* locEventRFBunch);
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

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override
		{
			for(auto locHypo : mData)
				Recycle_Hypothesis(locHypo);
			mData.clear();
			delete dResourcePool_ChargedTrackHypothesis;
		}

                bool CDC_CORRECT_DEDX_THETA;   // use the correction for dE/dx with theta

		// CDC space charge correction for dEdx
		vector<vector<double>>CDC_DEDX_AMP_CORRECTION;
		double cdc_min_theta, cdc_max_theta;
		double cdc_min_dedx, cdc_max_dedx;
		double cdc_theta_step, cdc_dedx_step;
		int cdc_npoints_theta, cdc_npoints_dedx;
		double Correct_CDC_dEdx_amp(double theta_deg, double thisdedx);

                // Correction for dE/dx from integral
		vector<vector<double>>CDC_DEDX_INT_CORRECTION;
		double cdc_min_theta_int, cdc_max_theta_int;
		double cdc_min_dedx_int, cdc_max_dedx_int;
		double cdc_theta_step_int, cdc_dedx_step_int;
		int cdc_npoints_theta_int, cdc_npoints_dedx_int;
		double Correct_CDC_dEdx_int(double theta_deg, double thisdedx);
};

#endif // _DChargedTrackHypothesis_factory_


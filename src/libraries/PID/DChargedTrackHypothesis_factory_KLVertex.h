// $Id$
//
//    File: DChargedTrackHypothesis_factory_KLVertex.h
//

#ifndef _DChargedTrackHypothesis_factory_KLVertex_
#define _DChargedTrackHypothesis_factory_KLVertex_

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

class DChargedTrackHypothesis_factory_KLVertex:public jana::JFactory<DChargedTrackHypothesis>
{
	public:
		const char* Tag(void){return "KLVertex";}

		DChargedTrackHypothesis* Create_ChargedTrackHypothesis(JEventLoop* locEventLoop, const DTrackTimeBased* locTrackTimeBased, const DDetectorMatches* locDetectorMatches);
		void Add_TimeToTrackingMatrix(DChargedTrackHypothesis* locChargedTrackHypothesis, TMatrixFSym* locCovarianceMatrix, double locFlightTimeVariance, double locHitTimeVariance, double locFlightTimePCorrelation) const;

// 		void Recycle_Hypotheses(vector<const DChargedTrackHypothesis*>& locHypos){dResourcePool_ChargedTrackHypothesis->Recycle(locHypos);}
// 		void Recycle_Hypotheses(vector<DChargedTrackHypothesis*>& locHypos){dResourcePool_ChargedTrackHypothesis->Recycle(locHypos);}
// 		void Recycle_Hypothesis(const DChargedTrackHypothesis* locHypo){dResourcePool_ChargedTrackHypothesis->Recycle(locHypo);}

//		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_ChargedTrackHypothesis->Get_NumObjectsAllThreads();}
// 		DChargedTrackHypothesis* Get_Resource(void)
// 		{
// 			auto locHypo = dResourcePool_ChargedTrackHypothesis->Get_Resource();
// 			return locHypo;
// 		}

	private:
		const DParticleID* dPIDAlgorithm;

		//RESOURCE POOL
		//For some reason, JANA doesn't call factory destructor until AFTER the threads have been closed
		//This causes the pool destructor to crash.  Instead, delete in fini();
		vector<DChargedTrackHypothesis*> dCreated;
		//DResourcePool<DChargedTrackHypothesis>* dResourcePool_ChargedTrackHypothesis = nullptr;
		//shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t fini(void)
		{
// 			for(auto locHypo : _data)
// 				Recycle_Hypothesis(locHypo);
// 			_data.clear();
// 			delete dResourcePool_ChargedTrackHypothesis;
			return NOERROR;
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

#endif // _DChargedTrackHypothesis_factory_KLVertex_


// $Id$
//
//    File: DRFTime_factory_TAGH.h
// Created: Mon Mar 30 10:51:29 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//

#ifndef _DRFTime_factory_TAGH_
#define _DRFTime_factory_TAGH_

#include <limits>
#include <iostream>

#include <JANA/JFactoryT.h>
#include "TTAB/DTTabUtilities.h"

#include "DRFTime.h"
#include "DRFDigiTime.h"
#include "DRFTDCDigiTime.h"

using namespace std;


class DRFTime_factory_TAGH : public JFactoryT<DRFTime>
{
	public:
		DRFTime_factory_TAGH(){
			SetTag("TAGH");
		};
		~DRFTime_factory_TAGH(){};

		double Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo) const;
		double Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo, double locPeriod) const;

		double Convert_TDCToTime(const DRFTDCDigiTime* locRFTDCDigiTime, const DTTabUtilities* locTTabUtilities) const;

		void BeginRun(const std::shared_ptr<const JEvent>& event) override;	///< Called everytime a new run number is detected.
	
	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double Calc_WeightedAverageRFTime(vector<double>& locRFTimes, double& locRFTimeVariance) const;

		DetectorSystem_t dSourceSystem;
		double dBeamBunchPeriod;

		double dTimeOffset;
		double dTimeOffsetVariance;
		double dTimeResolutionSq;
};

#endif // _DRFTime_factory_TAGH_


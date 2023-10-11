// $Id$
//
//    File: DRFTime_factory.h
// Created: Mon Mar 30 10:51:29 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//

#ifndef _DRFTime_factory_
#define _DRFTime_factory_

#include <limits>
#include <iostream>

#include <JANA/JFactoryT.h>
#include "TTAB/DTTabUtilities.h"

#include "DRFTime.h"
#include "DRFDigiTime.h"
#include "DRFTDCDigiTime.h"

using namespace std;


class DRFTime_factory : public JFactoryT<DRFTime>
{
	public:
		DRFTime_factory(){};
		~DRFTime_factory(){};

		double Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo) const;
		double Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo, double locPeriod) const;

		double Convert_TDCToTime(const DRFTDCDigiTime* locRFTDCDigiTime, const DTTabUtilities* locTTabUtilities) const;
		double Convert_ADCToTime(const DRFDigiTime* locRFDigiTime) const;

		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
	
	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double Calc_WeightedAverageRFTime(map<DetectorSystem_t, vector<double> >& locRFTimesMap, double& locRFTimeVariance) const;

		DetectorSystem_t dOverrideRFSourceSystem; //Choose this system over the best-resolution system if data is present (default SYS_NULL (disabled))
		double dBeamBunchPeriod;

		map<DetectorSystem_t, double> dTimeOffsetMap;
		map<DetectorSystem_t, double> dTimeOffsetVarianceMap;
		map<DetectorSystem_t, double> dTimeResolutionSqMap;
};

#endif // _DRFTime_factory_

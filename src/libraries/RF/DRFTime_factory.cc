// $Id$
//
//    File: DRFTime_factory.cc
// Created: Mon Mar 30 10:51:29 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//

#include "DRFTime_factory.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

//------------------
// Init
//------------------
void DRFTime_factory::Init()
{
	dOverrideRFSourceSystem = SYS_NULL; 
}

//------------------
// BeginRun
//------------------
void DRFTime_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto run_number = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto calibration = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);

	//RF Period
	vector<double> locBeamPeriodVector;
	calibration->Get("PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];
	
	//Time Offsets
	map<string, double> locCCDBMap;
	map<string, double>::iterator locIterator;
	if(calibration->Get("/PHOTON_BEAM/RF/time_offset", locCCDBMap))
		jout << "Error loading /PHOTON_BEAM/RF/time_offset !" << jendl;
	for(locIterator = locCCDBMap.begin(); locIterator != locCCDBMap.end(); ++locIterator)
	{
		DetectorSystem_t locSystem = NameToSystem(locIterator->first.c_str());
		dTimeOffsetMap[locSystem] = locIterator->second;
	}

	//Time Offset Variances
	if(calibration->Get("/PHOTON_BEAM/RF/time_offset_var", locCCDBMap))
		jout << "Error loading /PHOTON_BEAM/RF/time_offset_var !" << jendl;
	for(locIterator = locCCDBMap.begin(); locIterator != locCCDBMap.end(); ++locIterator)
	{
		DetectorSystem_t locSystem = NameToSystem(locIterator->first.c_str());
		dTimeOffsetVarianceMap[locSystem] = locIterator->second;
	}

	//Time Resolution Squared
	if(calibration->Get("/PHOTON_BEAM/RF/time_resolution_sq", locCCDBMap))
		jout << "Error loading /PHOTON_BEAM/RF/time_resolution_sq !" << jendl;
	for(locIterator = locCCDBMap.begin(); locIterator != locCCDBMap.end(); ++locIterator)
	{
		DetectorSystem_t locSystem = NameToSystem(locIterator->first.c_str());
		dTimeResolutionSqMap[locSystem] = locIterator->second;
	}

	string locSystemName = "NULL";
	app->SetDefaultParameter("RF:SOURCE_SYSTEM", locSystemName);
	dOverrideRFSourceSystem = NameToSystem(locSystemName.c_str());
}

//------------------
// Process
//------------------
void DRFTime_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	//The RF Time is defined at the center of the target
	//The time offset needed to line it up to the center of the target is absorbed into the calibration constants.

	vector<const DRFTDCDigiTime*> locRFTDCDigiTimes;
	event->Get(locRFTDCDigiTimes);

	const DTTabUtilities* locTTabUtilities = NULL;
	event->GetSingle(locTTabUtilities);

	//Get All RF Times
	map<DetectorSystem_t, vector<double> > locRFTimesMap;

	//F1TDC's
	for(size_t loc_i = 0; loc_i < locRFTDCDigiTimes.size(); ++loc_i)
	{
		const DRFTDCDigiTime* locRFTDCDigiTime = locRFTDCDigiTimes[loc_i];
		DetectorSystem_t locSystem = locRFTDCDigiTime->dSystem;
		if(dTimeOffsetMap.find(locSystem) == dTimeOffsetMap.end())
			continue; //system not recognized
		double locRFTime = Convert_TDCToTime(locRFTDCDigiTime, locTTabUtilities);
		locRFTimesMap[locSystem].push_back(locRFTime);
	}

	if(locRFTimesMap.empty())
		return; //No RF signals, will try to emulate RF bunch time from timing from other systems

	//Prefer to use the system with the best timing resolution (TOF if present)
		//Seems to be an additional jitter when averaging times between systems

	//Find the best system present in the data
	DetectorSystem_t locBestSystem = SYS_NULL;
	if(locRFTimesMap.find(dOverrideRFSourceSystem) != locRFTimesMap.end())
		locBestSystem = dOverrideRFSourceSystem; //Overriden on command line: use this system
	else
	{
		//Not overriden or not present, search for the system with the best resolution
		map<DetectorSystem_t, double>::iterator locResolutionIterator = dTimeResolutionSqMap.begin();
		double locBestResolution = 9.9E9;
		for(; locResolutionIterator != dTimeResolutionSqMap.end(); ++locResolutionIterator)
		{
			DetectorSystem_t locSystem = locResolutionIterator->first;
			if(locRFTimesMap.find(locSystem) == locRFTimesMap.end())
				continue; // this system is not in the data stream for this event
			if(locResolutionIterator->second >= locBestResolution)
				continue;
			locBestResolution = locResolutionIterator->second;
			locBestSystem = locResolutionIterator->first;
		}
	}

	//Use it (remove the others)
	map<DetectorSystem_t, vector<double> >::iterator locTimeIterator = locRFTimesMap.begin();
	while(locTimeIterator != locRFTimesMap.end())
	{
		if(locTimeIterator->first != locBestSystem)
			locRFTimesMap.erase(locTimeIterator++);
		else
			++locTimeIterator;
	}

	if(locRFTimesMap.empty())
		return; //No RF signals, will try to emulate RF bunch time from timing from other systems

	//Calculate the average RF time
	double locRFTimeVariance = 0.0;
	double locAverageRFTime = Calc_WeightedAverageRFTime(locRFTimesMap, locRFTimeVariance);

	DRFTime* locRFTime = new DRFTime();
	locRFTime->dTime = locAverageRFTime; //This time is defined at the center of the target (offsets with other detectors center it)
	locRFTime->dTimeVariance = locRFTimeVariance;
	Insert(locRFTime);
}

double DRFTime_factory::Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo) const
{
	return Step_TimeToNearInputTime(locTimeToStep, locTimeToStepTo, dBeamBunchPeriod);
}

double DRFTime_factory::Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo, double locPeriod) const
{
	double locDeltaT = locTimeToStepTo - locTimeToStep;
	int locNumBucketsToShift = (locDeltaT > 0.0) ? int(locDeltaT/locPeriod + 0.5) : int(locDeltaT/locPeriod - 0.5);
	return (locTimeToStep + locPeriod*double(locNumBucketsToShift));
}

double DRFTime_factory::Convert_TDCToTime(const DRFTDCDigiTime* locRFTDCDigiTime, const DTTabUtilities* locTTabUtilities) const
{
	double locRFTime = 0.0;
	if(locRFTDCDigiTime->dIsCAENTDCFlag)
		locRFTime = locTTabUtilities->Convert_DigiTimeToNs_CAEN1290TDC(locRFTDCDigiTime);
	else
		locRFTime = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(locRFTDCDigiTime);

	map<DetectorSystem_t, double>::const_iterator locIterator = dTimeOffsetMap.find(locRFTDCDigiTime->dSystem);
	if(locIterator != dTimeOffsetMap.end())
		locRFTime -= locIterator->second;
	return locRFTime;
}

double DRFTime_factory::Calc_WeightedAverageRFTime(map<DetectorSystem_t, vector<double> >& locRFTimesMap, double& locRFTimeVariance) const
{
	//returns the average (and the variance by reference)

	//will line up the first time near zero, then line up subsequent times to the first time
		//cannot line up all times to zero: if first time is near +/- beam_period/2 (e.g. +/- 2.004 ns),
		//may end up with some times near 2.004 and some near -2.004!

	map<DetectorSystem_t, vector<double> >::iterator locTimeIterator = locRFTimesMap.begin();
	double locFirstTime = (locTimeIterator->second)[0];
	locFirstTime = Step_TimeToNearInputTime(locFirstTime, 0.0);

	//take weighted average of times
		//avg = Sum(w_i * x_i) / Sum (w_i)
			//w_i = 1/varx_i
		//avg = Sum(x_i / varx_i) / Sum (1 / varx_i)
			//var_avg = 1 / Sum (1 / varx_i)
		//avg = Sum(x_i / varx_i) * var_avg

	double locSumOneOverTimeVariance = 0.0;
	double locSumTimeOverTimeVariance = 0.0;
	for(; locTimeIterator != locRFTimesMap.end(); ++locTimeIterator)
	{
		DetectorSystem_t locSystem = locTimeIterator->first;
		vector<double>& locRFTimes = locTimeIterator->second;

		double locSingleTimeVariance = dTimeResolutionSqMap.find(locSystem)->second + dTimeOffsetVarianceMap.find(locSystem)->second;
		if(!(locSingleTimeVariance > 0.0))
			continue;
		locSumOneOverTimeVariance += double(locRFTimes.size()) / locSingleTimeVariance;

		for(size_t loc_i = 0; loc_i < locRFTimes.size(); ++loc_i)
		{
			double locShiftedRFTime = Step_TimeToNearInputTime(locRFTimes[loc_i], locFirstTime);
			locSumTimeOverTimeVariance += locShiftedRFTime / locSingleTimeVariance;
		}
	}

	if(!(locSumOneOverTimeVariance > 0.0))
	{
		locRFTimeVariance = numeric_limits<double>::quiet_NaN();
		return locRFTimesMap.begin()->second.front();
	}

	locRFTimeVariance = 1.0 / locSumOneOverTimeVariance;
	double locAverageRFTime = locSumTimeOverTimeVariance * locRFTimeVariance;
	return locAverageRFTime;
}

//------------------
// EndRun
//------------------
void DRFTime_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DRFTime_factory::Finish()
{
}

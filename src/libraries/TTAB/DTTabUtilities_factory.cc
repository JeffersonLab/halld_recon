// $Id$
//
//    File: DTTabUtilities_factory.cc
// Created: Mon Apr  6 09:41:29 EDT 2015
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-504.12.2.el6.x86_64 x86_64)
//
#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include "DTTabUtilities_factory.h"
#include "TOF/DTOFGeometry.h"

void DTTabUtilities_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	//Early Commissioning Data: Code & CCDB constants
	//BCAL, RF: none
	//SC, TAGH, TAGM: Uses F1TDC rollover
	//FDC: is bad (tdc_scale fixed at 0.115, no ref time or tframe). Use F1TDC rollover x 2
	//PSC: Uses F1TDC rollover; if run = 0 change run = 2012 (same as old hardcoded)

	auto event_number = event->GetEventNumber();
	auto run_number = event->GetRunNumber();
	auto calibration_manager = event->GetJApplication()->GetService<JCalibrationManager>();
	auto calibration = calibration_manager->GetJCalibration(run_number);

	// F1TDC tframe(ns) and rollover count
	map<string, int> tdc_parms;
	if(calibration->Get("/F1TDC/rollover", tdc_parms, event_number))
		jout << "Error loading /F1TDC/rollover !" << jendl;

	map<string, int>::const_iterator locMapIterator = tdc_parms.find("tframe");
	dRolloverTimeWindowLength = (locMapIterator != tdc_parms.end()) ? uint64_t(tdc_parms["tframe"]) : 0;

	locMapIterator = tdc_parms.find("count");
	dNumTDCTicksInRolloverTimeWindow = (locMapIterator != tdc_parms.end()) ? uint64_t(tdc_parms["count"]) : 0;

	if(event->GetRunNumber() == 0) //PSC data with bad run number. Use hard-coded values from run 2012
	{
		dRolloverTimeWindowLength = 3744;
		dNumTDCTicksInRolloverTimeWindow = 64466;
	}

	//CAEN1290/TI Phase Difference
	dCAENTIPhaseDifference = 1;
	map<string, double> tof_tdc_shift;
	const DTOFGeometry *locTOFGeometry = event->GetSingle<DTOFGeometry>();

	string locTOFTDCShiftTable = locTOFGeometry->Get_CCDB_DirectoryName() + "/tdc_shift";
	if(!calibration->Get(locTOFTDCShiftTable, tof_tdc_shift, event_number))
		dCAENTIPhaseDifference = tof_tdc_shift["TOF_TDC_SHIFT"];

}

void DTTabUtilities_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DMCThrown*> locMCThrowns = event->Get<DMCThrown>();

	DTTabUtilities* locTTabUtilities = new DTTabUtilities();
	locTTabUtilities->dRolloverTimeWindowLength = dRolloverTimeWindowLength;
	locTTabUtilities->dNumTDCTicksInRolloverTimeWindow = dNumTDCTicksInRolloverTimeWindow;
	locTTabUtilities->dHasBadOrNoF1TDCConfigInfoFlag = ((event->GetRunNumber() <= 2965) || (!locMCThrowns.empty()));
	locTTabUtilities->dCAENTIPhaseDifference = dCAENTIPhaseDifference;

	// Get DCODAROCInfo's, put into map
	vector<const DCODAROCInfo*> locCODAROCInfos = event->Get<DCODAROCInfo>();

	map<uint32_t, const DCODAROCInfo*> locCODAROCInfoMap;
	for(size_t loc_i = 0; loc_i < locCODAROCInfos.size(); ++loc_i)
		locCODAROCInfoMap[locCODAROCInfos[loc_i]->rocid] = locCODAROCInfos[loc_i];
	locTTabUtilities->dCODAROCInfoMap = locCODAROCInfoMap;

	//get the trigger reference signal ("Beni-cable")
		//hard-coded crate/slot/channel, but whatever. This isn't intended to be long-term-code anyway.

	vector<const DF1TDCHit*> locF1TDCHits = event->Get<DF1TDCHit>();

	bool locFoundFlag = false;
	for(size_t loc_i = 0; loc_i < locF1TDCHits.size(); ++loc_i)
	{
		if((locF1TDCHits[loc_i]->rocid != 51) || (locF1TDCHits[loc_i]->slot != 17) || (locF1TDCHits[loc_i]->channel != 8))
			continue;
		locTTabUtilities->dTriggerReferenceSignal = locF1TDCHits[loc_i]->time; //in TDC clicks
		locTTabUtilities->dTriggerReferenceSignalIsLowResTDC = (locF1TDCHits[loc_i]->modtype == DModuleType::F1TDC48);
		locFoundFlag = true;
		break;
	}
	if(!locFoundFlag)
		locTTabUtilities->dTriggerReferenceSignal = 0;

	Insert(locTTabUtilities);
}

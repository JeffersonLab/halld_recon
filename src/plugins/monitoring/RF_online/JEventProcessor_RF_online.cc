// $Id$
//
//    File: JEventProcessor_RF_online.cc
// Created: Wed Nov  8 11:58:09 EST 2015
// Creator: pmatt (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)

#include "JEventProcessor_RF_online.h"

// Routine used to create our JEventProcessor
extern "C"
{
	void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_RF_online());
	}
}

//DOCUMENTATION: https://halldweb.jlab.org/wiki/index.php/RF_Calibration

void JEventProcessor_RF_online::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();  // TODO: NWB: Why does the linter think this is dead code?

	//This constant should be fixed for the lifetime of GlueX.  If it ever changes, move it into the CCDB.
	dRFSignalPeriod = 1000.0/499.0; //2.004008016
	double locDeltaTRangeMax = 2.2;
	unsigned int locNumDeltaTBins = 1100;

	dRFSignalSystems.push_back(SYS_FDC);  dRFSignalSystems.push_back(SYS_PSC);
	dRFSignalSystems.push_back(SYS_TAGH);  dRFSignalSystems.push_back(SYS_TOF);

	dRFSamplingFactor[SYS_FDC] = 128;
	dRFSamplingFactor[SYS_TAGH] = 128;
	dRFSamplingFactor[SYS_PSC] = 128;
	dRFSamplingFactor[SYS_TOF] = 128;

	dMaxDeltaTHits[SYS_TOF] = 15;
	dMaxDeltaTHits[SYS_FDC] = 8;
	dMaxDeltaTHits[SYS_TAGH] = 8;
	dMaxDeltaTHits[SYS_PSC] = 8;

	string locHistName, locHistTitle;

	gDirectory->Cd("/");
	new TDirectoryFile("RF", "RF");
	gDirectory->cd("RF");

	//roc info consistency:
		//compare roc infos: delta-t of each to avg-t-exlcuding-itself //separate histograms for each delta
		//will create histograms in evnt() method
	dROCTIDirectory = new TDirectoryFile("ROCTIs", "ROCTIs");

	//# rf signals per event
	new TDirectoryFile("NumRFSignals", "NumRFSignals");
	gDirectory->cd("NumRFSignals");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = string("Num") + locSystemName + string("RFSignals");
		locHistTitle = string("RF_") + locSystemName + string(";Num RF Signals");
		dHistMap_NumSignals[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 20, -0.5, 19.5);
	}
	gDirectory->cd("..");

	//rf hits found
	new TDirectoryFile("RFHitsFound", "RFHitsFound");
	gDirectory->cd("RFHitsFound");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("RFHitsFound");
		locHistTitle = string("RF_") + locSystemName + string(";Which RF Hits Were Found");
		dHistMap_RFHitsFound[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 20, -0.5, 19.5);
	}
	gDirectory->cd("..");

	//num rf hits missing
	new TDirectoryFile("NumRFHitsMissing", "NumRFHitsMissing");
	gDirectory->cd("NumRFHitsMissing");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("NumRFHitsMissing");
		locHistTitle = string("RF_") + locSystemName + string(";# RF Hits Missing");
		dHistMap_NumRFHitsMissing[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 20, -0.5, 19.5);
	}
	gDirectory->cd("..");

	//rf times: //delta-t to first time
	new TDirectoryFile("DeltaT_RF_FirstTime", "DeltaT_RF_FirstTime");
	gDirectory->cd("DeltaT_RF_FirstTime");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("RF_FirstTimeDeltaT");
		locHistTitle = string("RF_") + locSystemName + string("_TDC;#Deltat (Subsequent Times - First Time) (ns)");
		dHistMap_RFFirstTimeDeltaT[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 1000000, 0.0, 5000.0);
	}
	gDirectory->cd("..");

	//all delta-t's:
	new TDirectoryFile("DeltaT_All", "DeltaT_All");
	gDirectory->cd("DeltaT_All");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		for(size_t loc_j = 0; loc_j < dMaxDeltaTHits[locSystem]; ++loc_j)
		{
			for(size_t loc_k = loc_j; loc_k < dMaxDeltaTHits[locSystem]; ++loc_k)
			{
				pair<size_t, size_t> locTimePair(loc_j, loc_k);
				ostringstream locHistNameStream, locHistTitleStream;
				locHistNameStream << locSystemName << "RF_DeltaT_" << loc_j << "_" << loc_k;
				double locHistRangeMin = -200.0 + (loc_k - loc_j)*dRFSignalPeriod*dRFSamplingFactor[locSystem];
				locHistTitleStream << "RF_" << locSystemName << "_TDC;" << "#Deltat: Hit " << loc_k << " - Hit " << loc_j << " (ns)";
				dHistMap_AdjacentRFDeltaTs[locSystem][locTimePair] = new TH1I(locHistNameStream.str().c_str(), locHistTitleStream.str().c_str(), 80000, locHistRangeMin, locHistRangeMin + 400.0);
			}
		}
	}
	gDirectory->cd("..");

	//rf signal period
	new TDirectoryFile("RF_SignalPeriod", "RF_SignalPeriod");
	gDirectory->cd("RF_SignalPeriod");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("RF_SignalPeriod");
		locHistTitle = string("RF_") + locSystemName + string(";RF Signal Period (ns)");
		dHistMap_RFSignalPeriod[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 1200, 2.001, 2.007);
	}
	gDirectory->cd("..");

	//RF beam bunch period
	new TDirectoryFile("RF_BeamBunchPeriod", "RF_BeamBunchPeriod");
	gDirectory->cd("RF_BeamBunchPeriod");
	locHistName = "RFBeamBunchPeriod";
	locHistTitle = ";TAGH #Deltat (Within Same Counter) (ns)";
	dHist_RFBeamBunchPeriod = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 1000, 0.0, 200.0);
	gDirectory->cd("..");

	//resolutions: compare each rf to itself
	new TDirectoryFile("DeltaT_RF_Itself", "DeltaT_RF_Itself");
	gDirectory->cd("DeltaT_RF_Itself");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("RF_SelfDeltaT");
		locHistTitle = string("RF_") + locSystemName + string(";#Deltat (First Pair) (ns)");
		dHistMap_SelfResolution[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), locNumDeltaTBins, -1.0*locDeltaTRangeMax, locDeltaTRangeMax);
	}
	rf_itself_num_events = new TH1I("rf_itself_num_events", "RF itself number of events", 1, 0.0, 1.0);
	gDirectory->cd("..");

	//absolute resolutions: compare each rf to each rf
	new TDirectoryFile("AbsoluteDeltaT_RF_OtherRFs", "AbsoluteDeltaT_RF_OtherRFs");
	gDirectory->cd("AbsoluteDeltaT_RF_OtherRFs");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		string locSystemName1 = SystemName(dRFSignalSystems[loc_i]);
		for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
		{
			string locSystemName2 = SystemName(dRFSignalSystems[loc_j]);
			locHistName = string("RFDeltaT_") + locSystemName1 + string("_") + locSystemName2;
			locHistTitle = string(";#Deltat (RF_") + locSystemName1 + string(" - RF_") + locSystemName2 + string(")");
			pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(dRFSignalSystems[loc_i], dRFSignalSystems[loc_j]);
			dHistMap_AbsoluteRFRFDeltaTs[locSystemPair] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 4000000, -4000.0, 4000.0);
		}
	}
	gDirectory->cd("..");

	//resolutions: compare each rf to each rf
	new TDirectoryFile("DeltaT_RF_OtherRFs", "DeltaT_RF_OtherRFs");
	gDirectory->cd("DeltaT_RF_OtherRFs");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		string locSystemName1 = SystemName(dRFSignalSystems[loc_i]);
		for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
		{
			string locSystemName2 = SystemName(dRFSignalSystems[loc_j]);
			locHistName = string("RFDeltaT_") + locSystemName1 + string("_") + locSystemName2;
			locHistTitle = string(";#Deltat (RF_") + locSystemName1 + string(" - RF_") + locSystemName2 + string(")");
			pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(dRFSignalSystems[loc_i], dRFSignalSystems[loc_j]);
			dHistMap_RFRFDeltaTs[locSystemPair] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), locNumDeltaTBins, -1.0*locDeltaTRangeMax, locDeltaTRangeMax);
		}
	}
	gDirectory->cd("..");

	//resolutions: compare avg of each rf to avg of each rf
		//calib time offsets
	new TDirectoryFile("AverageDeltaT_RF_OtherRFs", "AverageDeltaT_RF_OtherRFs");
	gDirectory->cd("AverageDeltaT_RF_OtherRFs");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		string locSystemName1 = SystemName(dRFSignalSystems[loc_i]);
		for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
		{
			string locSystemName2 = SystemName(dRFSignalSystems[loc_j]);
			locHistName = string("RFDeltaT_") + locSystemName1 + string("_") + locSystemName2;
			locHistTitle = string(";#Deltat_{#mu} (RF_") + locSystemName1 + string(" - RF_") + locSystemName2 + string(")");
			pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(dRFSignalSystems[loc_i], dRFSignalSystems[loc_j]);
			dHistMap_AverageRFRFDeltaTs[locSystemPair] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), locNumDeltaTBins, -1.0*locDeltaTRangeMax, locDeltaTRangeMax);
		}
	}
	gDirectory->cd("..");

	//compare each rf to tagger hodoscope times
	new TDirectoryFile("DeltaT_RF_TAGH", "DeltaT_RF_TAGH");
	gDirectory->cd("DeltaT_RF_TAGH");
	for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
		string locSystemName = SystemName(locSystem);

		locHistName = locSystemName + string("RF_TaggerDeltaT");
		locHistTitle = string(";#Deltat (RF_") + locSystemName + string(" - TAGH) (ns)");
		dHistMap_RFTaggerDeltaT[locSystem] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), locNumDeltaTBins, -1.0*locDeltaTRangeMax, locDeltaTRangeMax);
	}
	gDirectory->cd("..");

	gDirectory->cd(".."); //END (get back to base folder)

}

void JEventProcessor_RF_online::BeginRun(const std::shared_ptr<const JEvent> &locEvent, int32_t runnumber)
{
	// This is called whenever the run number changes

    auto dRFTimeFactory = static_cast<DRFTime_factory*>(locEvent->GetFactory("DRFTime", ""));
    // TODO: NWB: Excise all uses of brun_was_called and Set_brun_called from codebase
}

void JEventProcessor_RF_online::Process(const std::shared_ptr<const JEvent> &locEvent, uint64_t eventnumber)
{
	vector<const DCODAROCInfo*> locCODAROCInfos;
	locEvent->Get(locCODAROCInfos);

	const DTTabUtilities* locTTabUtilities = NULL;
	locEvent->GetSingle(locTTabUtilities);

	vector<const DRFTDCDigiTime*> locRFTDCDigiTimes;
	locEvent->Get(locRFTDCDigiTimes);

	vector<const DTAGHHit*> locTAGHHits;
	locEvent->Get(locTAGHHits);

	auto dRFTimeFactory = static_cast<DRFTime_factory*>(locEvent->GetFactory("DRFTime", ""));

	//Convert TDCs to Times
	//Use std::set: times are NOT necessarily in order (for high-resolution mode, times interleaved between different internal channels)
	map<DetectorSystem_t, set<double> > locRFTimes;
	map<DetectorSystem_t, set<double> >::iterator locTDCIterator;
	for(size_t loc_i = 0; loc_i < locRFTDCDigiTimes.size(); ++loc_i)
	{
		DetectorSystem_t locSystem = locRFTDCDigiTimes[loc_i]->dSystem;
		double locRFTime = dRFTimeFactory->Convert_TDCToTime(locRFTDCDigiTimes[loc_i], locTTabUtilities);
		locRFTimes[locSystem].insert(locRFTime);
	}

	//Get Total ROC Info Time
	uint64_t locTotalROCTimeStamp = 0;
	for(size_t loc_i = 0; loc_i < locCODAROCInfos.size(); ++loc_i)
		locTotalROCTimeStamp += locCODAROCInfos[loc_i]->timestamp;

	//Average RF Times
	map<DetectorSystem_t, double> locAverageRFTimes;
	for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
	{
		set<double>& locRFTimeSet = locTDCIterator->second;

		double locFirstTime = dRFTimeFactory->Step_TimeToNearInputTime(*(locRFTimeSet.begin()), 0.0);
		double locAverageTime = locFirstTime;
		set<double>::iterator locSetIterator = locRFTimeSet.begin();
		for(++locSetIterator; locSetIterator != locRFTimeSet.end(); ++locSetIterator)
			locAverageTime += dRFTimeFactory->Step_TimeToNearInputTime(*locSetIterator, locFirstTime);
		locAverageTime /= double(locRFTimeSet.size());
		locAverageRFTimes[locTDCIterator->first] = locAverageTime;
	}

	//organize TAGH hits
	map<int, set<double> > locTAGHHitMap; //double: TAGH tdc times (set allows time sorting)
	for(size_t loc_i = 0; loc_i < locTAGHHits.size(); ++loc_i)
	{
		if(locTAGHHits[loc_i]->has_TDC)
			locTAGHHitMap[locTAGHHits[loc_i]->counter_id].insert(locTAGHHits[loc_i]->time_tdc);
	}

	//collect TAGH delta-t's for RF beam bunch histogram
	map<int, set<double> >::iterator locTAGHIterator = locTAGHHitMap.begin();
	vector<double> locTAGHDeltaTs;
	for(; locTAGHIterator != locTAGHHitMap.end(); ++locTAGHIterator)
	{
		set<double>& locCounterHits = locTAGHIterator->second;
		if(locCounterHits.size() < 2)
			continue;
		set<double>::iterator locSetIterator = locCounterHits.begin();
		set<double>::iterator locPreviousSetIterator = locSetIterator;
		for(++locSetIterator; locSetIterator != locCounterHits.end(); ++locSetIterator, ++locPreviousSetIterator)
			locTAGHDeltaTs.push_back(*locSetIterator - *locPreviousSetIterator);
	}

	//CALC RF SIGNAL SAMPLING RATES
		//Set by EPICS guis, but can back-compute rather than carry around more data
		//Compute for each event, so don't need to acquire locks for accessing/modifying member variable (processor member variables are NOT thread-local)
	//Doesn't quite work!: Hits seem to be missing sometimes
/*
	map<DetectorSystem_t, double> locRFSamplingRateMap;
	for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
	{
		DetectorSystem_t locSystem = locTDCIterator->first;
		set<double>& locRFTimeSet = locTDCIterator->second;
		if(locRFTimeSet.size() < 2)
			continue;

		double locFirstDeltaT = *(++locRFTimeSet.begin()) - *(locRFTimeSet.begin());
		double locRFSamplingRate = locFirstDeltaT/dRFSignalPeriod; //is a number like "128.0" so is actually an inverse rate (rate = 1/128)

		//get the nearest even integer
		int locRFSamplingRateInt = int(locRFSamplingRate); //round down
		if(locRFSamplingRateInt % 2 == 1) //if true: rounded to odd: wrong direction
			locRFSamplingRateInt += (locRFSamplingRate > double(locRFSamplingRateInt)) ? 1 : -1; //if rounded down/up, increase/decrease (was wrong direction)
		//else rounded down to even number: OK (rounding up would have been to an odd number, so rounding down was closest int)

		locRFSamplingRateMap[locSystem] = 1.0/double(locRFSamplingRateInt); //convert from int -> double, then from inverse-rate to rate (i.e. is now something like 1/128.0)
		set<double>::iterator locSetIterator = locRFTimeSet.begin();
		for(++locSetIterator; locSetIterator != locRFTimeSet.end(); ++locSetIterator)
			dHistMap_RFFirstTimeDeltaT[locTDCIterator->first]->Fill(*locSetIterator - *(locRFTimeSet.begin()));
	}
*/

	//MAKE/FILL ROC HISTOGRAMS
	lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		//roc info consistency:
			//compare roc infos: delta-t of each to avg-t-exlcuding-itself //separate histograms for each delta
		for(size_t loc_i = 0; loc_i < locCODAROCInfos.size(); ++loc_i)
		{
			//double has precision of 14 digits: 4ns per tick, 14 digits = 111 hours, way longer than a run
			double locTotalROCTimeStamp_AllButThis = double(locTotalROCTimeStamp - locCODAROCInfos[loc_i]->timestamp);
			double locDeltaT = 4.0*(double(locCODAROCInfos[loc_i]->timestamp) - locTotalROCTimeStamp_AllButThis/(double(locCODAROCInfos.size() - 1)));

			//if histogram doesn't exist yet, create it
			uint32_t locROCID = locCODAROCInfos[loc_i]->rocid;
			if(dHistMap_ROCInfoDeltaT.find(locROCID) == dHistMap_ROCInfoDeltaT.end())
			{
				dROCTIDirectory->cd();
				ostringstream locROCIDStream;
				locROCIDStream << locROCID;
				string locHistName = string("ROCDeltaTs_ROC") + locROCIDStream.str();
				string locHistTitle = string(";#Deltat (ROC ") + locROCIDStream.str() + string(" - #mu_{Other ROCs}) (ns)");
				dHistMap_ROCInfoDeltaT[locROCID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 33, -16.5, 16.5);
				gDirectory->Cd("/");
			}

			//fill
			dHistMap_ROCInfoDeltaT[locROCID]->Fill(locDeltaT);
		}
	}
	lockService->RootUnLock(); //RELEASE ROOT LOCK!!

	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK
	{
		// Event count used by RootSpy->RSAI so it knows how many events have been seen.
		rf_itself_num_events->Fill(0.5);
	
		//num rf signals
		for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
		{
			DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
			locTDCIterator = locRFTimes.find(locSystem);
			if(locTDCIterator == locRFTimes.end())
				dHistMap_NumSignals[locSystem]->Fill(0);
			else
				dHistMap_NumSignals[locSystem]->Fill(locTDCIterator->second.size());
		}

		//rf hits found, # missing, adjacent delta-t's
		for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
		{
			set<double>& locRFTimeSet = locTDCIterator->second;
			DetectorSystem_t locSystem = locTDCIterator->first;
			double locHitPeriod = dRFSignalPeriod*dRFSamplingFactor[locSystem];

			set<double>::iterator locSetIterator = locRFTimeSet.begin();
			size_t locNumHitsMissing = 0;
			for(; locSetIterator != locRFTimeSet.end(); ++locSetIterator)
			{
				double locTimeFromStart1 = *locSetIterator - *(locRFTimeSet.begin());
				size_t locHitIndex1 = size_t(locTimeFromStart1 / locHitPeriod + 0.5);
				dHistMap_RFHitsFound[locSystem]->Fill(locHitIndex1);

				int locExpectedHitIndex = locHitIndex1 + 1;
				set<double>::iterator locOtherIterator = locSetIterator;
				for(++locOtherIterator; locOtherIterator != locRFTimeSet.end(); ++locOtherIterator)
				{
					double locTimeFromStart2 = *locOtherIterator - *(locRFTimeSet.begin());
					size_t locHitIndex2 = size_t(locTimeFromStart2 / locHitPeriod + 0.5);
					locNumHitsMissing += (int(locHitIndex2) > locExpectedHitIndex) ? locHitIndex2 - locExpectedHitIndex : 0;
					if(locHitIndex2 >= dMaxDeltaTHits[locSystem])
						break; //no histogram for this pair

					pair<size_t, size_t> locTimePair(locHitIndex1, locHitIndex2);
					double locDeltaT = *locOtherIterator - *locSetIterator;
					dHistMap_AdjacentRFDeltaTs[locSystem][locTimePair]->Fill(locDeltaT);

					locExpectedHitIndex = locHitIndex2 + 1;
				}
				dHistMap_NumRFHitsMissing[locSystem]->Fill(locNumHitsMissing);
			}
		}

		//RF signal frequency
		for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
		{
			set<double>& locRFTimeSet = locTDCIterator->second;
			if(locRFTimeSet.size() < 2)
				continue;
			DetectorSystem_t locSystem = locTDCIterator->first;
			double locDeltaT = *(locRFTimeSet.rbegin()) - *(locRFTimeSet.begin());
			double locRFSignalFrequency = locDeltaT/(double(dRFSamplingFactor[locSystem]*(locRFTimeSet.size() - 1)));
			dHistMap_RFSignalPeriod[locSystem]->Fill(locRFSignalFrequency);
		}

		//RF beam bunch period
		for(size_t loc_i = 0; loc_i < locTAGHDeltaTs.size(); ++loc_i)
			dHist_RFBeamBunchPeriod->Fill(locTAGHDeltaTs[loc_i]);

		//TDCs: RF / First RF
		for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
		{
			set<double>& locRFTimeSet = locTDCIterator->second;
			set<double>::iterator locSetIterator = locRFTimeSet.begin();
			for(++locSetIterator; locSetIterator != locRFTimeSet.end(); ++locSetIterator)
				dHistMap_RFFirstTimeDeltaT[locTDCIterator->first]->Fill(*locSetIterator - *(locRFTimeSet.begin()));
		}

		//TDCs / Self
		for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
		{
			set<double>& locRFTimeSet = locTDCIterator->second;
			if(locRFTimeSet.size() < 2)
				continue;
			set<double>::iterator locSetIterator = locRFTimeSet.begin();
			++locSetIterator;
			double locShiftedRFTime = dRFTimeFactory->Step_TimeToNearInputTime(*locSetIterator, *(locRFTimeSet.begin()));
			dHistMap_SelfResolution[locTDCIterator->first]->Fill(locShiftedRFTime - *(locRFTimeSet.begin()));
		}

		//RF / TAGH
		for(size_t loc_i = 0; loc_i < locTAGHHits.size(); ++loc_i)
		{
			if(!locTAGHHits[loc_i]->has_TDC)
				continue;

			//TDCs
			for(locTDCIterator = locRFTimes.begin(); locTDCIterator != locRFTimes.end(); ++locTDCIterator)
			{
				set<double>& locRFTimeSet = locTDCIterator->second;
				set<double>::iterator locSetIterator = locRFTimeSet.begin();
				for(; locSetIterator != locRFTimeSet.end(); ++locSetIterator)
				{
					double locShiftedRFTime = dRFTimeFactory->Step_TimeToNearInputTime(*locSetIterator, locTAGHHits[loc_i]->time_tdc);
					dHistMap_RFTaggerDeltaT[locTDCIterator->first]->Fill(locShiftedRFTime - locTAGHHits[loc_i]->time_tdc);
				}
			}
		}

		//TDCs / TDCs
		for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
		{
			DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
			locTDCIterator = locRFTimes.find(locSystem);
			if(locTDCIterator == locRFTimes.end())
				continue;
			set<double>& locRFTimeSet = locTDCIterator->second;

			for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
			{
				DetectorSystem_t locSystem2 = dRFSignalSystems[loc_j];
				map<DetectorSystem_t, set<double> >::iterator locTDCIterator2 = locRFTimes.find(locSystem2);
				if(locTDCIterator2 == locRFTimes.end())
					continue;

				pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(locSystem, locSystem2);
				if(dHistMap_RFRFDeltaTs.find(locSystemPair) == dHistMap_RFRFDeltaTs.end())
					continue;
				set<double>& locRFTimeSet2 = locTDCIterator2->second;

				double locShiftedRFTime = dRFTimeFactory->Step_TimeToNearInputTime(*(locRFTimeSet.begin()), *(locRFTimeSet2.begin()));
				double locDeltaT = locShiftedRFTime - *(locRFTimeSet2.begin());
				dHistMap_RFRFDeltaTs[locSystemPair]->Fill(locDeltaT);
			}
		}

		//Average TDCs / TDCs
		for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
		{
			DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
			locTDCIterator = locRFTimes.find(locSystem);
			if(locTDCIterator == locRFTimes.end())
				continue;

			for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
			{
				DetectorSystem_t locSystem2 = dRFSignalSystems[loc_j];
				map<DetectorSystem_t, set<double> >::iterator locTDCIterator2 = locRFTimes.find(locSystem2);
				if(locTDCIterator2 == locRFTimes.end())
					continue;

				pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(locSystem, locSystem2);
				if(dHistMap_AverageRFRFDeltaTs.find(locSystemPair) == dHistMap_AverageRFRFDeltaTs.end())
					continue;

				double locShiftedAverageRFTime = dRFTimeFactory->Step_TimeToNearInputTime(locAverageRFTimes[locTDCIterator->first], locAverageRFTimes[locTDCIterator2->first]);
				double locDeltaT = locShiftedAverageRFTime - locAverageRFTimes[locTDCIterator2->first];
				dHistMap_AverageRFRFDeltaTs[locSystemPair]->Fill(locDeltaT);
			}
		}

		//Absolute TDCs / TDCs
		for(size_t loc_i = 0; loc_i < dRFSignalSystems.size(); ++loc_i)
		{
			DetectorSystem_t locSystem = dRFSignalSystems[loc_i];
			locTDCIterator = locRFTimes.find(locSystem);
			if(locTDCIterator == locRFTimes.end())
				continue;
			set<double>& locRFTimeSet = locTDCIterator->second;

			for(size_t loc_j = loc_i + 1; loc_j < dRFSignalSystems.size(); ++loc_j)
			{
				DetectorSystem_t locSystem2 = dRFSignalSystems[loc_j];
				map<DetectorSystem_t, set<double> >::iterator locTDCIterator2 = locRFTimes.find(locSystem2);
				if(locTDCIterator2 == locRFTimes.end())
					continue;

				pair<DetectorSystem_t, DetectorSystem_t> locSystemPair(locSystem, locSystem2);
				if(dHistMap_AbsoluteRFRFDeltaTs.find(locSystemPair) == dHistMap_AbsoluteRFRFDeltaTs.end())
					continue;
				set<double>& locRFTimeSet2 = locTDCIterator2->second;

				double locDeltaT = *(locRFTimeSet.begin()) - *(locRFTimeSet2.begin());
				dHistMap_AbsoluteRFRFDeltaTs[locSystemPair]->Fill(locDeltaT);
			}
		}
	}
	lockService->RootUnLock(); //RELEASE ROOT FILL LOCK
}

void JEventProcessor_RF_online::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

void JEventProcessor_RF_online::Finish()
{
	// Called before program exit after event processing is finished.
}

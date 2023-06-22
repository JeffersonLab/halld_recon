// $Id$
//
// File: DEventProcessor_monitoring_hists.cc
// Created: Thu Sep 28 11:38:03 EDT 2011
// Creator: pmatt (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include "DEventProcessor_monitoring_hists.h"

// The executable should define the ROOTfile global variable. It will
// be automatically linked when dlopen is called.
extern TFile *ROOTfile;

// Routine used to create our DEventProcessor
extern "C"
{
	void InitPlugin(JApplication *app)
	{
		InitJANAPlugin(app);
		app->AddProcessor(new DEventProcessor_monitoring_hists());
	}
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_monitoring_hists::init(void)
{
	dNumMemoryMonitorEvents = 0;
	gPARMS->SetDefaultParameter("MONITOR:MEMORY_EVENTS", dNumMemoryMonitorEvents);

    MIN_TRACKING_FOM = 0.0027;
    gPARMS->SetDefaultParameter("MONITOR:MIN_TRACKING_FOM", MIN_TRACKING_FOM);

	string locOutputFileName = "hd_root.root";
	if(gPARMS->Exists("OUTPUT_FILENAME"))
		gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

	//go to file
	TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
	if(locFile != NULL)
		locFile->cd("");
	else
		gDirectory->Cd("/");

	//go to directory
	TDirectoryFile* locSubDirectory = static_cast<TDirectoryFile*>(gDirectory->Get("Independent"));
	if(locSubDirectory == NULL) //else folder already created
		locSubDirectory = new TDirectoryFile("Independent", "Independent");
	locSubDirectory->cd();

	dHist_IsEvent = new TH1D("IsEvent", "Is the event an event?", 2, -0.5, 1.5);
	dHist_IsEvent->GetXaxis()->SetBinLabel(1, "False");
	dHist_IsEvent->GetXaxis()->SetBinLabel(2, "True");

	gDirectory->cd("..");

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_monitoring_hists::brun(JEventLoop *locEventLoop, int32_t runnumber)
{
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);

	//Initialize Actions
	dHistogramAction_NumReconstructedObjects.Initialize(locEventLoop);
	dHistogramAction_Reconstruction.Initialize(locEventLoop);
	dHistogramAction_EventVertex.Initialize(locEventLoop);

    dHistogramAction_DetectorMatching.dMinTrackingFOM = MIN_TRACKING_FOM;
	dHistogramAction_DetectorMatching.Initialize(locEventLoop);
	dHistogramAction_DetectorMatchParams.Initialize(locEventLoop);
	dHistogramAction_Neutrals.Initialize(locEventLoop);
	dHistogramAction_DetectorPID.Initialize(locEventLoop);

    dHistogramAction_TrackMultiplicity.dMinTrackingFOM = MIN_TRACKING_FOM;
	dHistogramAction_TrackMultiplicity.Initialize(locEventLoop);
	dHistogramAction_DetectedParticleKinematics.Initialize(locEventLoop);
	dHistogramAction_TrackShowerErrors.Initialize(locEventLoop);
	dHistogramAction_TriggerStudies.Initialize(locEventLoop);

	if(dNumMemoryMonitorEvents > 0)
	{
		dHistogramAction_ObjectMemory.dMaxNumEvents = dNumMemoryMonitorEvents;
		dHistogramAction_ObjectMemory.Initialize(locEventLoop);
	}

	if(!locMCThrowns.empty())
	{
		dHistogramAction_ThrownParticleKinematics.Initialize(locEventLoop);
		dHistogramAction_ReconnedThrownKinematics.Initialize(locEventLoop);
		dHistogramAction_GenReconTrackComparison.Initialize(locEventLoop);
	}

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_monitoring_hists::evnt(JEventLoop *locEventLoop, uint64_t eventnumber)
{
	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock

        // first fill histograms which should always be filled, whether or not this is a real "physics" event
	vector<const DMCThrown*> locMCThrowns;
	locEventLoop->Get(locMCThrowns);
	if(!locMCThrowns.empty())
	{
		dHistogramAction_ThrownParticleKinematics(locEventLoop);
	}

	if(dNumMemoryMonitorEvents > 0)
		dHistogramAction_ObjectMemory(locEventLoop);

	//CHECK TRIGGER TYPE
	const DTrigger* locTrigger = NULL;
	locEventLoop->GetSingle(locTrigger);
	if(!locTrigger->Get_IsPhysicsEvent())
		return NOERROR;

	japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	{
		dHist_IsEvent->Fill(1);
	}
	japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

	//Fill reaction-independent histograms.
	dHistogramAction_NumReconstructedObjects(locEventLoop);
	dHistogramAction_Reconstruction(locEventLoop);
	dHistogramAction_EventVertex(locEventLoop);

	dHistogramAction_DetectorMatching(locEventLoop);
	dHistogramAction_DetectorMatchParams(locEventLoop);
	dHistogramAction_Neutrals(locEventLoop);
	dHistogramAction_DetectorPID(locEventLoop);

	dHistogramAction_TrackMultiplicity(locEventLoop);
	dHistogramAction_DetectedParticleKinematics(locEventLoop);
	dHistogramAction_TrackShowerErrors(locEventLoop);
	dHistogramAction_TriggerStudies(locEventLoop);

	if(!locMCThrowns.empty())
	{
		dHistogramAction_ReconnedThrownKinematics(locEventLoop);
		dHistogramAction_GenReconTrackComparison(locEventLoop);
	}

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_monitoring_hists::erun(void)
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_monitoring_hists::fini(void)
{
	return NOERROR;
}


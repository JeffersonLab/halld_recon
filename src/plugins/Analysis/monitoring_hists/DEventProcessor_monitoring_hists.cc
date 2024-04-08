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
		app->Add(new DEventProcessor_monitoring_hists());
	}
} // "C"

//------------------
// Init
//------------------
void DEventProcessor_monitoring_hists::Init()
{
	auto app = GetApplication();

	dNumMemoryMonitorEvents = 0;
	app->SetDefaultParameter("MONITOR:MEMORY_EVENTS", dNumMemoryMonitorEvents);

    MIN_TRACKING_FOM = 0.0027;
    app->SetDefaultParameter("MONITOR:MIN_TRACKING_FOM", MIN_TRACKING_FOM);

	string locOutputFileName = "hd_root.root";
	if(app->GetJParameterManager()->Exists("OUTPUT_FILENAME"))
		app->GetParameter("OUTPUT_FILENAME", locOutputFileName);

	// TODO: NWB: Rejigger Exists() to be less weird and ugly

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
}

//------------------
// BeginRun
//------------------
void DEventProcessor_monitoring_hists::BeginRun(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DMCThrown*> locMCThrowns;
	locEvent->Get(locMCThrowns);

	//Initialize Actions
	dHistogramAction_NumReconstructedObjects.Initialize(locEvent);
	dHistogramAction_Reconstruction.Initialize(locEvent);
	dHistogramAction_EventVertex.Initialize(locEvent);

    dHistogramAction_DetectorMatching.dMinTrackingFOM = MIN_TRACKING_FOM;
	dHistogramAction_DetectorMatching.Initialize(locEvent);
	dHistogramAction_DetectorMatchParams.Initialize(locEvent);
	dHistogramAction_Neutrals.Initialize(locEvent);
	dHistogramAction_DetectorPID.Initialize(locEvent);

    dHistogramAction_TrackMultiplicity.dMinTrackingFOM = MIN_TRACKING_FOM;
	dHistogramAction_TrackMultiplicity.Initialize(locEvent);
	dHistogramAction_DetectedParticleKinematics.Initialize(locEvent);
	dHistogramAction_TrackShowerErrors.Initialize(locEvent);
	dHistogramAction_TriggerStudies.Initialize(locEvent);

	if(dNumMemoryMonitorEvents > 0)
	{
		dHistogramAction_ObjectMemory.dMaxNumEvents = dNumMemoryMonitorEvents;
		dHistogramAction_ObjectMemory.Initialize(locEvent);
	}

	if(!locMCThrowns.empty())
	{
		dHistogramAction_ThrownParticleKinematics.Initialize(locEvent);
		dHistogramAction_ReconnedThrownKinematics.Initialize(locEvent);
		dHistogramAction_GenReconTrackComparison.Initialize(locEvent);
	}
}

//------------------
// Process
//------------------
void DEventProcessor_monitoring_hists::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock

        // first fill histograms which should always be filled, whether or not this is a real "physics" event
	vector<const DMCThrown*> locMCThrowns;
	locEvent->Get(locMCThrowns);
	if(!locMCThrowns.empty())
	{
		dHistogramAction_ThrownParticleKinematics(locEvent);
	}

	if(dNumMemoryMonitorEvents > 0)
		dHistogramAction_ObjectMemory(locEvent);

	//CHECK TRIGGER TYPE
	const DTrigger* locTrigger = NULL;
	locEvent->GetSingle(locTrigger);
	if(!locTrigger->Get_IsPhysicsEvent())
		return;

	GetLockService(locEvent)->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	{
		dHist_IsEvent->Fill(1);
	}
	GetLockService(locEvent)->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

	//Fill reaction-independent histograms.
	dHistogramAction_NumReconstructedObjects(locEvent);
	dHistogramAction_Reconstruction(locEvent);
	dHistogramAction_EventVertex(locEvent);

	dHistogramAction_DetectorMatching(locEvent);
	dHistogramAction_DetectorMatchParams(locEvent);
	dHistogramAction_Neutrals(locEvent);
	dHistogramAction_DetectorPID(locEvent);

	dHistogramAction_TrackMultiplicity(locEvent);
	dHistogramAction_DetectedParticleKinematics(locEvent);
	dHistogramAction_TrackShowerErrors(locEvent);
	dHistogramAction_TriggerStudies(locEvent);

	if(!locMCThrowns.empty())
	{
		dHistogramAction_ReconnedThrownKinematics(locEvent);
		dHistogramAction_GenReconTrackComparison(locEvent);
	}
}

//------------------
// EndRun
//------------------
void DEventProcessor_monitoring_hists::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_monitoring_hists::Finish()
{
}


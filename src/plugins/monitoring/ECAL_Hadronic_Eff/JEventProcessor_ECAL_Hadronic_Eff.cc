// $Id$
//
//    File: JEventProcessor_ECAL_Hadronic_Eff.cc
// Created: Mon Jul  7 11:40:05 AM EDT 2025
// Creator: aaustreg (on Linux ifarm2402.jlab.org 5.14.0-503.23.2.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_ECAL_Hadronic_Eff.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_ECAL_Hadronic_Eff());
}
} // "C"


//------------------
// JEventProcessor_ECAL_Hadronic_Eff (Constructor)
//------------------
JEventProcessor_ECAL_Hadronic_Eff::JEventProcessor_ECAL_Hadronic_Eff()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_ECAL_Hadronic_Eff (Destructor)
//------------------
JEventProcessor_ECAL_Hadronic_Eff::~JEventProcessor_ECAL_Hadronic_Eff()
{
}

//------------------
// Init
//------------------
void JEventProcessor_ECAL_Hadronic_Eff::Init()
{
    // This is called once at program startup. 
    
        auto app = GetApplication();
        // lockService should be initialized here like this
        lockService = app->GetService<JLockService>();

        //TRACK REQUIREMENTS
	dMaxTOFDeltaT = 1.0;
	dMinTrackingFOM = 5.73303E-7; // +/- 5 sigma
	dMinNumTrackHits = 16; //e.g. 4 in each FDC plane
	dMinHitRingsPerCDCSuperlayer = 0;
	dMinHitPlanesPerFDCPackage = 4;
	dMaxVertexR = 1.0;
	dCutAction_TrackHitPattern = new DCutAction_TrackHitPattern(NULL, dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage);
	//action initialize not necessary: is empty


	TDirectory* locOriginalDir = gDirectory;
	gDirectory->mkdir("ECAL_Hadronic_Eff")->cd();

	//Histograms
	string locHistName, locHistTitle;

	locHistName = "TrackECALYVsX_HasHit";
	locHistTitle = "ECAL Has Hit;Projected ECAL Hit X (cm);Projected ECAL Hit Y (cm)";
	dHist_TrackECALYVsX_HasHit = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 110, -50.0, 50.0, 110, -50.0, 50.0);

	locHistName = "TrackECALYVsX_NoHit";
	locHistTitle = "ECAL Total Hit;Projected ECAL Hit X (cm);Projected ECAL Hit Y (cm)";
	dHist_TrackECALYVsX_TotalHit = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 110, -50.0, 50.0, 110, -50.0, 50.0);

	locHistName = "TrackECALRowVsColumn_HasHit";
	locHistTitle = "ECAL Has Hit;Projected ECAL Hit Column;Projected ECAL Hit Row";
	dHist_TrackECALRowVsColumn_HasHit = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 40, -0.5, 39.5, 40, -0.5, 39.5);

	locHistName = "TrackECALRowVsColumn_NoHit";
	locHistTitle = "ECAL Total Hit;Projected ECAL Hit Column;Projected ECAL Hit Row";
	dHist_TrackECALRowVsColumn_TotalHit = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 40, -0.5, 39.5, 40, -0.5, 39.5);

	locHistName = "TrackECAL_ResX";
	locHistTitle = "ECAL Residual X;Projected ECAL Hit X - Shower X";
	dHist_TrackECAL_ResX = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 400, -10, 10);

	locHistName = "TrackECAL_ResY";
	locHistTitle = "ECAL Residual Y;Projected ECAL Hit Y - Shower Y";
	dHist_TrackECAL_ResY = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 400, -10, 10);

	locHistName = "TrackECAL_ResX_all";
	locHistTitle = "ECAL Residual X;Projected ECAL Hit X - Shower X";
	dHist_TrackECAL_ResX_all = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 400, -10, 10);

	locHistName = "TrackECAL_ResY_all";
	locHistTitle = "ECAL Residual Y;Projected ECAL Hit Y - Shower Y";
	dHist_TrackECAL_ResY_all = new TH1I(locHistName.c_str(), locHistTitle.c_str(), 400, -10, 10);

	locHistName = "TrackECALRowVsColumn_ResX";
	locHistTitle = "ECAL Residual X;Projected ECAL Hit Column;Projected ECAL Hit Row";
	dHist_TrackECALRowVsColumn_ResX = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 40, -0.5, 39.5, 40, -0.5, 39.5);

	locHistName = "TrackECALRowVsColumn_ResY";
	locHistTitle = "ECAL Residual Y;Projected ECAL Hit Column;Projected ECAL Hit Row";
	dHist_TrackECALRowVsColumn_ResY = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 40, -0.5, 39.5, 40, -0.5, 39.5);

	// back to original dir
	locOriginalDir->cd();


    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_ECAL_Hadronic_Eff::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_ECAL_Hadronic_Eff::Process(const std::shared_ptr<const JEvent> &event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    // Here's an example:
    //
    // vector<const MyDataClass*> mydataclasses;
    // event->Get(mydataclasses);
    //
    // If you have lockService initialized in Init() then you can acquire locks like this
    // lockService->RootFillLock(this);
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

    // This plugin is used to determine the reconstruction efficiency of hits in the ECAL
		// Note, this is hit-level, not shower-level.  Hits: By sector/layer/module/end

	//CUT ON TRIGGER TYPE
	const DTrigger* locTrigger = NULL;
	event->GetSingle(locTrigger);
	if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
		return;

	const DEventRFBunch* locEventRFBunch = NULL;
	event->GetSingle(locEventRFBunch);
	if(locEventRFBunch->dNumParticleVotes <= 1)
    	return; //don't trust PID: beta-dependence

	vector<const DChargedTrack*> locChargedTracks;
	event->Get(locChargedTracks);

	const DParticleID* locParticleID = NULL;
	event->GetSingle(locParticleID);

	vector<const DECALShower*> locECALShowers;
	event->Get(locECALShowers);

	const DDetectorMatches* locDetectorMatches = NULL;
	event->GetSingle(locDetectorMatches);

	//Try to select the most-pure sample of tracks possible
	set<const DChargedTrackHypothesis*> locBestTracks;
	for(auto& locChargedTrack : locChargedTracks)
	{
		//loop over PID hypotheses and find the best (if any good enough)
		const DChargedTrackHypothesis* locBestChargedTrackHypothesis = nullptr;
		double locBestTrackingFOM = -1.0;
		for(auto& locChargedTrackHypothesis : locChargedTrack->dChargedTrackHypotheses)
		{
			if((locChargedTrackHypothesis->PID() == Electron) || (locChargedTrackHypothesis->PID() == Positron))
				continue; //don't evaluate for these

			if(locChargedTrackHypothesis->position().Perp() > dMaxVertexR)
				continue; //don't trust reconstruction if not close to target

			//Need PID for beta-dependence, but cannot use ECAL info: would bias
			if(!Cut_TOFTiming(locChargedTrackHypothesis))
				continue;

			auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
			if(locTrackTimeBased->FOM < dMinTrackingFOM)
				continue; //don't trust tracking results: bad tracking FOM

			if(!dCutAction_TrackHitPattern->Cut_TrackHitPattern(locParticleID, locTrackTimeBased))
				continue; //don't trust tracking results: not many grouped hits

			unsigned int locNumTrackHits = locTrackTimeBased->Ndof + 5;
			if(locNumTrackHits < dMinNumTrackHits)
				continue; //don't trust tracking results: not many hits

			if(locTrackTimeBased->FOM < locBestTrackingFOM)
				continue; //not the best mass hypothesis

			locBestTrackingFOM = locTrackTimeBased->FOM;
			locBestChargedTrackHypothesis = locChargedTrackHypothesis;
		}

		//if passed all cuts, save the best
		if(locBestChargedTrackHypothesis != nullptr)
			locBestTracks.insert(locBestChargedTrackHypothesis);
	}

	// Loop over the good tracks, using the best DTrackTimeBased object for each
	for(auto& locChargedTrackHypothesis : locBestTracks)
	{
		auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();

		//Predict ECAL Surface Hit Location
		DVector3 locProjectedECALIntersection;
		unsigned int locProjectedECALRow = 999, locProjectedECALColumn = 999;
		vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased->extrapolations.at(SYS_ECAL);
		if(!locParticleID->PredictECALHit(extrapolations, locProjectedECALRow, locProjectedECALColumn, &locProjectedECALIntersection))
			continue; //not predicted to hit ECAL

		//Find closest ECAL Shower
		const DECALShower* locECALShower = nullptr;
		shared_ptr<const DECALShowerMatchParams> locClosestMatchParams;
		double locStartTime = locTrackTimeBased->t0();
		if(locParticleID->Get_ClosestToTrack(extrapolations, locECALShowers, false, locStartTime, locClosestMatchParams))
			locECALShower = locClosestMatchParams->dECALShower;

		//Is match to ECAL shower?
		auto locECALShowerMatchParams = locChargedTrackHypothesis->Get_ECALShowerMatchParams();
		bool locIsMatchedToTrack = (locECALShowerMatchParams != nullptr);

		// FILL HISTOGRAMS
		// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
		lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
		{
			dHist_TrackECALYVsX_TotalHit->Fill(locProjectedECALIntersection.X(), locProjectedECALIntersection.Y());
			dHist_TrackECALRowVsColumn_TotalHit->Fill(locProjectedECALColumn, locProjectedECALRow);

			if(locIsMatchedToTrack)
			{
				dHist_TrackECALYVsX_HasHit->Fill(locProjectedECALIntersection.X(), locProjectedECALIntersection.Y());
				dHist_TrackECALRowVsColumn_HasHit->Fill(locProjectedECALColumn, locProjectedECALRow);

				dHist_TrackECAL_ResX->Fill(locProjectedECALIntersection.X() - locECALShower->pos.X());
				dHist_TrackECAL_ResY->Fill(locProjectedECALIntersection.Y() - locECALShower->pos.Y());

				dHist_TrackECALRowVsColumn_ResX->Fill(locProjectedECALColumn, locProjectedECALRow, locProjectedECALIntersection.X() - locECALShower->pos.X());
				dHist_TrackECALRowVsColumn_ResY->Fill(locProjectedECALColumn, locProjectedECALRow, locProjectedECALIntersection.Y() - locECALShower->pos.Y());
			}

			// Loop over all showers for unbiased residuals
			for(auto& locECALShower_all : locECALShowers){
				dHist_TrackECAL_ResX_all->Fill(locProjectedECALIntersection.X() - locECALShower_all->pos.X());
				dHist_TrackECAL_ResY_all->Fill(locProjectedECALIntersection.Y() - locECALShower_all->pos.Y());
			}
		}
		lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK


    }
}

bool JEventProcessor_ECAL_Hadronic_Eff::Cut_TOFTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis)
{
	if(locChargedTrackHypothesis->t1_detector() != SYS_TOF)
		return true; //don't require TOF hit: limits reach of study

	double locDeltaT = locChargedTrackHypothesis->time() - locChargedTrackHypothesis->t0();
	return (fabs(locDeltaT) <= dMaxTOFDeltaT);
}

//------------------
// EndRun
//------------------
void JEventProcessor_ECAL_Hadronic_Eff::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_ECAL_Hadronic_Eff::Finish()
{
    // Called before program exit after event processing is finished.
}


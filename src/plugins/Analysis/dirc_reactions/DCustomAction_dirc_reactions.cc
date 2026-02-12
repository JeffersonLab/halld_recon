// $Id$
//
//    File: DCustomAction_dirc_reactions.cc
//

#include "DCustomAction_dirc_reactions.h"

void DCustomAction_dirc_reactions::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	// get PID algos
	const DParticleID* locParticleID = NULL;
	locEvent->GetSingle(locParticleID);
	dParticleID = locParticleID;

	locEvent->GetSingle(dDIRCLut);
	locEvent->GetSingle(dAnalysisUtilities);

	// get DIRC geometry
	vector<const DDIRCGeometry*> locDIRCGeometry;
	locEvent->Get(locDIRCGeometry);
	dDIRCGeometry = locDIRCGeometry[0];

}

void DCustomAction_dirc_reactions::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
	auto params = locEvent->GetJApplication()->GetJParameterManager();
	DIRC_TRUTH_BARHIT = false;
	if(params->Exists("DIRC:TRUTH_BARHIT"))
		params->GetParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

	DIRC_FILL_BAR_MAP = false;
	params->SetDefaultParameter("DIRC:FILL_BAR_MAP",DIRC_FILL_BAR_MAP);

	//DIRC_FILL_PIXEL_MAP = false;
	//params->SetDefaultParameter("DIRC:FILL_PIXEL_MAP",DIRC_FILL_PIXEL_MAP);
	
	Run_Update(locEvent);

	// set PID for different passes in debuging histograms
	dFinalStatePIDs.push_back(Positron);
	dFinalStatePIDs.push_back(PiPlus);
	dFinalStatePIDs.push_back(KPlus);
	dFinalStatePIDs.push_back(Proton);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		//Required: Create a folder in the ROOT output file that will contain all of the output ROOT objects (if any) for this action.
			//If another thread has already created the folder, it just changes to it. 
		CreateAndChangeTo_ActionDirectory();

		string locParticleName = ParticleType(dPID);
		string locParticleROOTName = ParticleName_ROOT(dPID);
		string locLikelihoodName = "ln L(#pi) - ln L(K)";
		if(dPID==Electron || dPID==Positron)
			locLikelihoodName = "ln L(e) - ln L(#pi)";
		else if(dPID==Proton || dPID==AntiProton)
			locLikelihoodName = "ln L(K) - ln L(p)";

		hExtrapolatedBarHitXY_PreCut = GetOrCreate_Histogram<TH2I>(Form("hExtrapolatedBarHitXY_PreCut_%s",locParticleName.data()), "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);
		hExtrapolatedBarHitXY = GetOrCreate_Histogram<TH2I>(Form("hExtrapolatedBarHitXY_%s",locParticleName.data()), "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);

		hDiff = GetOrCreate_Histogram<TH1I>(Form("hDiff_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleROOTName.data()), 400,-10,10);
		hDiffDirect = GetOrCreate_Histogram<TH1I>(Form("hDiffDirect_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleROOTName.data()), 400,-10,10);
		hDiffReflected = GetOrCreate_Histogram<TH1I>(Form("hDiffReflected_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleROOTName.data()), 400,-10,10);
		hNphC = GetOrCreate_Histogram<TH1I>(Form("hNphC_%s",locParticleName.data()), Form("# photons; %s # photons", locParticleROOTName.data()), 150, 0, 150);
		hThetaC = GetOrCreate_Histogram<TH1I>(Form("hThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #theta_{C} [rad]", locParticleROOTName.data()), 250, 0.6, 1.0);
		hDeltaThetaC = GetOrCreate_Histogram<TH1I>(Form("hDeltaThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		hDeltaThetaCDirect = GetOrCreate_Histogram<TH1I>(Form("hDeltaThetaCDirect_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		hDeltaThetaCReflected = GetOrCreate_Histogram<TH1I>(Form("hDeltaThetaCReflected_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		hLikelihood = GetOrCreate_Histogram<TH1I>(Form("hLikelihood_%s",locParticleName.data()), Form("; %s -lnL; entries [#]", locParticleROOTName.data()),1000,0.,1000.);
		hLikelihoodDiff = GetOrCreate_Histogram<TH1I>(Form("hLikelihoodDiff_%s",locParticleName.data()), Form("; %s;entries [#]", locLikelihoodName.data()),100,-200.,200.);
		
		hThetaCVsP = GetOrCreate_Histogram<TH2I>(Form("hThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 250, 0.6, 1.0);
		hDeltaThetaCVsP = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
		hDeltaThetaCVsPDirect = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsPDirect_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
		hDeltaThetaCVsPReflected = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsPReflected_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
		hDeltaThetaCVsChannel = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsChannel_%s",locParticleName.data()),  Form("cherenkov angle vs. channel; channel ID; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 6912, 0, 6912, 200,-0.2,0.2);

		hLikelihoodDiffVsP = GetOrCreate_Histogram<TH2I>(Form("hLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName.data()), 120, 0.0, 12.0, 100, -200, 200);
		hReactionLikelihoodDiffVsP = GetOrCreate_Histogram<TH2I>(Form("hReactionLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName.data()), 120, 0.0, 12.0, 100, -200, 200);

		// Map of histograms for every bar, binned in x position
		if(DIRC_FILL_BAR_MAP) {

			for(int locBar=0; locBar<DDIRCGeometry::kBars; locBar++) {
				
				CreateAndChangeTo_Directory(Form("Bar %d", locBar), Form("Bar %d", locBar));
				double bar_y = dDIRCGeometry->GetBarY(locBar);
				
				hDeltaThetaCVsChannelMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsChannel_%s_%d",locParticleName.data(),locBar),  Form("cherenkov angle vs. channel; channel ID; %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar), 6912, 0, 6912, 200,-0.2,0.2);
				hDeltaThetaCDirectVsChannelMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCDirectVsChannel_%s_%d",locParticleName.data(),locBar),  Form("direct cherenkov angle vs. channel; channel ID; %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar), 6912, 0, 6912, 200,-0.2,0.2);
				hDeltaThetaCReflectedVsChannelMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCReflectedVsChannel_%s_%d",locParticleName.data(),locBar),  Form("reflected cherenkov angle vs. channel; channel ID; %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar), 6912, 0, 6912, 200,-0.2,0.2);
				hDeltaThetaCDirectVsPMTMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCDirectVsPMT_%s_%d",locParticleName.data(),locBar),  Form("direct cherenkov angle vs. PMT; PMT ID; %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar),DDIRCGeometry::kPMTs, 0, DDIRCGeometry::kPMTs, 200,-0.2,0.2);
				hDeltaThetaCReflectedVsPMTMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCReflectedVsPMT_%s_%d",locParticleName.data(),locBar),  Form("reflected cherenkov angle vs. PMT; PMT ID; %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar),DDIRCGeometry::kPMTs, 0, DDIRCGeometry::kPMTs, 200,-0.2,0.2);

				hDeltaThetaCVsDeltaYMap[locBar] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsDeltaYBar_%s_%d",locParticleName.data(),locBar),  Form("cherenkov angle vs. #Delta Y; #Delta Y (cm); %s Bar %d #Delta#theta_{C} [rad]", locParticleROOTName.data(),locBar), 100,-2,2, 200,-0.2,0.2); 

				for(int locXbin=0; locXbin<40; locXbin++) {
					
					double xbin_min = -100.0 + locXbin*5.0; 
					double xbin_max = xbin_min + 5.0;
					
					// skip creation of histograms for bins outside active area
					TVector3 locBinVect((xbin_min+xbin_max)/2., bar_y, 586.5 - 65.);
					if(locBinVect.Theta()*180/TMath::Pi() > 12.) 
						continue;

					hDeltaTOF[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hDeltaTOF_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s TOF #Delta x [cm]; %s TOF #Delta y [cm]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data(),locParticleROOTName.data()), 40,-10,10,40,-10,10);
					hDiffMap[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hDiff_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{calc}-t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 100,-10,10);
					hDiffMapDirect[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hDiffDirect_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{calc}-t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 100,-10,10);
					hDiffMapReflected[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hDiffReflected_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{calc}-t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 100,-10,10);
					hHitTimeMap[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hHitTimeMap_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 100,0,150);
					hNphCMap[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hNphC_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f] # photons; %s # photons", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 80, 0, 80);
					
					hThetaCVsPMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hThetaCVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f] cherenkov angle vs. momentum; p (GeV/c); %s #theta_{C} [rad]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 60, 0.0, 12.0, 250,0.6,1.0);
					hDeltaThetaCVsPMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f] cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 60, 0.0, 12.0, 300,-0.15,0.15);
					hDeltaThetaCVsPMapDirect[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCDirectVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f] cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 60, 0.0, 12.0, 300,-0.15,0.15);
					hDeltaThetaCVsPMapReflected[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCReflectedVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f] cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 60, 0.0, 12.0, 300,-0.15,0.15);
					hReactionLikelihoodDiffVsPMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hReactionLikelihoodDiffVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f]; p (GeV/c); %s", locBar,xbin_min,xbin_max,locLikelihoodName.data()), 60, 0.0, 12.0, 100, -100, 100);
					//hPixelHitMap3D[locBar][locXbin] = GetOrCreate_Histogram<TH3S>(Form("hPixelHit3D_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns; hit time", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5, 50, 0, 100);
					hPixelHitMap[locBar][locXbin] = GetOrCreate_Histogram<TH2S>(Form("hPixelHit_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
					hPixelHitMapReflected[locBar][locXbin] = GetOrCreate_Histogram<TH2S>(Form("hPixelHitReflected_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);

					hHitTimeMapAll[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hHitTimeMapAll_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 100,0,150);
					//hPixelHitMapAll[locBar][locXbin] = GetOrCreate_Histogram<TH2S>(Form("hPixelHitAll_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
					//hPixelHitMapAllReflected[locBar][locXbin] = GetOrCreate_Histogram<TH2S>(Form("hPixelHitAllReflected_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
					//hPixelHitTimeMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hPixelHitTime_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; Pixel Hit Channel; Pixel Hit t [ns]", locBar,xbin_min,xbin_max), 6912, 0, 6912, 50, 0, 100);
				}
				
				gDirectory->cd(".."); //End of single bar histograms
			}
		}		

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
}

bool DCustomAction_dirc_reactions::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{

	const DDetectorMatches* locDetectorMatches = NULL;
        locEvent->GetSingle(locDetectorMatches);
	DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];
	
	// truth information on tracks hitting DIRC bar (for comparison)
	vector<const DDIRCTruthBarHit*> locDIRCBarHits;
	locEvent->Get(locDIRCBarHits);

	vector<const DDIRCPmtHit*> locDIRCPmtHits;
	locEvent->Get(locDIRCPmtHits);

	// Get selected particle from reaction for DIRC analysis
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(dParticleComboStepIndex);
	auto locParticle = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dParticleIndex) : locParticleComboStep->Get_FinalParticle_Measured(dParticleIndex);

	// Get track and histogram DIRC quantities
	const DChargedTrack* locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(dParticleIndex));
	const DChargedTrackHypothesis* locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
	const DTrackTimeBased* locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
	
	//Lock_Action(); //ACQUIRE ROOT LOCK!!
	//hExtrapolatedBarHitXY_PreCut->Fill(posInBar.X(), posInBar.Y());
	//Unlock_Action(); //RELEASE ROOT LOCK!!

	// require well reconstructed tracks for initial studies
	int locDCHits = locTrackTimeBased->Ndof + 5;
	double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
	double locP = locParticle->lorentzMomentum().Vect().Mag();
	if(locDCHits < 15 || locTheta < 1.0 || locTheta > 12.0 || locP > 12.0)
		return true;
	
	// require has good match to TOF hit for cleaner sample
	shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	bool foundTOF = dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	if(!foundTOF || locTOFHitMatchParams->dDeltaXToHit > 10.0 || locTOFHitMatchParams->dDeltaYToHit > 10.0)
		return true;

	/*
	// get FCAL for check on residual
	shared_ptr<const DFCALShowerMatchParams> locFCALShowerMatchParams;
	bool foundFCAL = dParticleID->Get_BestFCALMatchParams(locTrackTimeBased, locDetectorMatches, locFCALShowerMatchParams);
	const DFCALShower* locMatchedFCALShower;
	if(foundFCAL) {
	  locMatchedFCALShower = locFCALShowerMatchParams->dFCALShower; 
	}
	*/

	Particle_t locPID = locTrackTimeBased->PID();
	double locMass = ParticleMass(locPID);

	// get DIRC match parameters (contains LUT information)
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	bool foundDIRC = dParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
	vector<int> locUsedPixel;

	if(foundDIRC) {

		DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
		DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
		double locExpectedThetaC = locDIRCMatchParams->dExpectedThetaC;
		double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;

		///////////////////////////////////////////
		// option to cheat and use truth bar hit //
		///////////////////////////////////////////	
		if(DIRC_TRUTH_BARHIT && locDIRCBarHits.size() > 0) {

	                TVector3 bestMatchPos, bestMatchMom;
                	double bestMatchDist = 999.;
                	for(int i=0; i<(int)locDIRCBarHits.size(); i++) {
                        	TVector3 locDIRCBarHitPos(locDIRCBarHits[i]->x, locDIRCBarHits[i]->y, locDIRCBarHits[i]->z);
                        	TVector3 locDIRCBarHitMom(locDIRCBarHits[i]->px, locDIRCBarHits[i]->py, locDIRCBarHits[i]->pz);
                        	if((posInBar - locDIRCBarHitPos).Mag() < bestMatchDist) {
                                	bestMatchDist = (posInBar - locDIRCBarHitPos).Mag();
                                	bestMatchPos = locDIRCBarHitPos;
                                	bestMatchMom = locDIRCBarHitMom;
                        	}
                	}

                	momInBar = bestMatchMom;
                	posInBar = bestMatchPos;
        	}

	
		// get binning for histograms
		int locBar = dDIRCGeometry->GetBar(posInBar.Y());
		//if(fabs(posInBar.Y() - dDIRCGeometry->GetBarY(locBar)) > 1.0)
		//    return true;

		int locXbin = (int)(posInBar.X()/5.0) + 19;
		
		// check that histogram index exists
		TVector3 locBarHitVect(posInBar.X(), posInBar.Y(), posInBar.Z() - 65.);
		if(locBar < 0 || locXbin < 0 || locXbin > 39 || locBarHitVect.Theta()*180/TMath::Pi() > 12.0)
			return true;

		Lock_Action(); //ACQUIRE ROOT LOCK!!
		hExtrapolatedBarHitXY->Fill(posInBar.X(), posInBar.Y());
		if(DIRC_FILL_BAR_MAP) hDeltaTOF[locBar][locXbin]->Fill(locTOFHitMatchParams->dDeltaXToHit,locTOFHitMatchParams->dDeltaYToHit);
		Unlock_Action(); //RELEASE ROOT LOCK!!

		double locAngle = dDIRCLut->CalcAngle(locP, locMass);
		map<Particle_t, double> locExpectedAngle = dDIRCLut->CalcExpectedAngles(locP);
		
		// get map of DIRCMatches to PMT hits
		map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParamsMap;
		locDetectorMatch.Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParamsMap);
		map<Particle_t, double> logLikelihoodSum;
		  
		// loop over associated hits for LUT diagnostic plots
		for(uint loc_i=0; loc_i<locDIRCPmtHits.size(); loc_i++) {
		        bool locIsReflected = false;
			vector<pair<double, double>> locDIRCPhotons = dDIRCLut->CalcPhoton(locDIRCPmtHits[loc_i], locExtrapolatedTime, posInBar, momInBar, locExpectedAngle, locAngle, locPID, locIsReflected, logLikelihoodSum);
			double locHitTime = locDIRCPmtHits[loc_i]->t - locExtrapolatedTime;
			int locChannel = locDIRCPmtHits[loc_i]->ch;
			if(locChannel >= DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels) locChannel -= DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels;

			// fill PMT hits without any association cuts
			Lock_Action(); //ACQUIRE ROOT LOCK!!
			if(DIRC_FILL_BAR_MAP) {
				hHitTimeMapAll[locBar][locXbin]->Fill(locHitTime);
			}
			Unlock_Action(); //RELEASE ROOT LOCK!!
					
			// format final pixel x' and y' axes for view from behind PMTs looking downstream
			int pixel_row = dDIRCGeometry->GetPixelRow(locChannel);
			int pixel_col = dDIRCGeometry->GetPixelColumn(locChannel);
			int pmt_id = dDIRCGeometry->GetPmtID(locChannel);

			// fill histograms for candidate photons in timing cut
			if(locHitTime > 0 && locHitTime < 150.0) {

				Lock_Action(); //ACQUIRE ROOT LOCK!!
				if(DIRC_FILL_BAR_MAP && locP > 4.) {
				  /*
					hPixelHitTimeMap[locBar][locXbin]->Fill(locChannel, locHitTime);
					hPixelHitMap3D[locBar][locXbin]->Fill(pixel_row, pixel_col, locHitTime);
					if(locHitTime < 38.)
						hPixelHitMapAll[locBar][locXbin]->Fill(pixel_row, pixel_col);
					else
						hPixelHitMapAllReflected[locBar][locXbin]->Fill(pixel_row, pixel_col);
				  */
				}
				Unlock_Action(); //RELEASE ROOT LOCK!!
				
			}
			
			// if found associated photons loop over them and fill histos
			if(locDIRCPhotons.size() > 0) {

				// loop over candidate photons
				for(uint loc_j = 0; loc_j<locDIRCPhotons.size(); loc_j++) {
					double locDeltaT = locDIRCPhotons[loc_j].first - locHitTime;
					double locThetaC = locDIRCPhotons[loc_j].second;
					
					if(fabs(locThetaC-locExpectedThetaC)<0.02) {	
						Lock_Action(); //ACQUIRE ROOT LOCK!!
						hDiff->Fill(locDeltaT);
						if(locIsReflected) hDiffReflected->Fill(locDeltaT);
						else hDiffDirect->Fill(locDeltaT);
						
						if(DIRC_FILL_BAR_MAP) {
						        hDiffMap[locBar][locXbin]->Fill(locDeltaT);
							if(locIsReflected) hDiffMapReflected[locBar][locXbin]->Fill(locDeltaT);
							else hDiffMapDirect[locBar][locXbin]->Fill(locDeltaT);
							hHitTimeMap[locBar][locXbin]->Fill(locHitTime);
						}
						Unlock_Action(); //RELEASE ROOT LOCK!!
					}					

					// fill histograms for candidate photons in timing cut
					if(fabs(locDeltaT) < 6.0) {
						Lock_Action(); //ACQUIRE ROOT LOCK!!
						hThetaC->Fill(locThetaC);
						hDeltaThetaC->Fill(locThetaC-locExpectedThetaC);
						hThetaCVsP->Fill(momInBar.Mag(), locThetaC);
						hDeltaThetaCVsP->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
						if(locIsReflected) {
						  double weight = 1;
						  if(fabs(locDeltaT) > 3.0) weight = -1;
						  hDeltaThetaCReflected->Fill(locThetaC-locExpectedThetaC,weight);
						  hDeltaThetaCVsPReflected->Fill(momInBar.Mag(),locThetaC-locExpectedThetaC);
						}
						else {
						  double weight = 1;
						  if(fabs(locDeltaT) > 2.0) weight = -1;
						  if(fabs(locDeltaT) > 4.0) weight = 0;
						  hDeltaThetaCDirect->Fill(locThetaC-locExpectedThetaC,weight);
						  hDeltaThetaCVsPDirect->Fill(momInBar.Mag(),locThetaC-locExpectedThetaC);
						}
						hDeltaThetaCVsChannel->Fill(locChannel, locThetaC-locExpectedThetaC);
						
						if(DIRC_FILL_BAR_MAP) {
						  hDeltaThetaCVsPMap[locBar][locXbin]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
						  if(locIsReflected) {
						    double weight = 1;
						    if(fabs(locDeltaT) > 3.0) weight = -1;
						    hDeltaThetaCVsPMapReflected[locBar][locXbin]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC, weight);
						    hDeltaThetaCReflectedVsChannelMap[locBar]->Fill(locChannel, locThetaC-locExpectedThetaC, weight);
						    hDeltaThetaCReflectedVsPMTMap[locBar]->Fill(pmt_id, locThetaC-locExpectedThetaC, weight);
						  }
						  else {
						    double weight = 1;
						    if(fabs(locDeltaT) > 2.0) weight = -1;
						    if(fabs(locDeltaT) > 4.0) weight = 0;
						    hDeltaThetaCVsPMapDirect[locBar][locXbin]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC, weight);
						    hDeltaThetaCDirectVsChannelMap[locBar]->Fill(locChannel, locThetaC-locExpectedThetaC, weight);
						    hDeltaThetaCDirectVsPMTMap[locBar]->Fill(pmt_id, locThetaC-locExpectedThetaC, weight);
						  }
						  hDeltaThetaCVsChannelMap[locBar]->Fill(locChannel, locThetaC-locExpectedThetaC);
						  hDeltaThetaCVsDeltaYMap[locBar]->Fill(posInBar.Y() - dDIRCGeometry->GetBarY(locBar), locThetaC-locExpectedThetaC);
						  hThetaCVsPMap[locBar][locXbin]->Fill(momInBar.Mag(), locThetaC);
						}
						Unlock_Action(); //RELEASE ROOT LOCK!!
						
						if(std::find(locUsedPixel.begin(), locUsedPixel.end(), locChannel) != locUsedPixel.end()) 
							continue;
						
						Lock_Action(); //ACQUIRE ROOT LOCK!!
						if(DIRC_FILL_BAR_MAP && locP > 4.) {
							//hPixelHitTimeMap[locBar][locXbin]->Fill(locChannel, locHitTime);
							if(locHitTime < 38.) 
								hPixelHitMap[locBar][locXbin]->Fill(pixel_row, pixel_col);
							else
								hPixelHitMapReflected[locBar][locXbin]->Fill(pixel_row, pixel_col);
						}
						locUsedPixel.push_back(locChannel);
						Unlock_Action(); //RELEASE ROOT LOCK!!
					}
				}	  
			}			  
		}
		
		// fill histograms with per-track quantities
		Lock_Action(); //ACQUIRE ROOT LOCK!!
		hNphC->Fill(locDIRCMatchParams->dNPhotons);
		//hThetaCVsP->Fill(momInBar.Mag(), locDIRCMatchParams->dThetaC); 
		if(DIRC_FILL_BAR_MAP) {
		  hNphCMap[locBar][locXbin]->Fill(locDIRCMatchParams->dNPhotons);
		}
		
		// for likelihood and difference for given track mass hypothesis
		if(locPID == Positron || locPID == Electron) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodElectron);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[Positron]-logLikelihoodSum[PiPlus]);
			if(DIRC_FILL_BAR_MAP) 
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[Positron]-logLikelihoodSum[PiPlus]);
		}
		else if(locPID == PiPlus || locPID == PiMinus) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodPion);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[PiPlus]-logLikelihoodSum[KPlus]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[PiPlus]-logLikelihoodSum[KPlus]);
		}
		else if(locPID == KPlus || locPID == KMinus) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[PiPlus]-logLikelihoodSum[KPlus]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[PiPlus]-logLikelihoodSum[KPlus]);
		}
		else if(locPID == Proton || locPID == AntiProton) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodProton);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodKaon - locDIRCMatchParams->dLikelihoodProton);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[KPlus]-logLikelihoodSum[Proton]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[KPlus]-logLikelihoodSum[Proton]);
		}
		Unlock_Action(); //RELEASE ROOT LOCK!!

	}

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}

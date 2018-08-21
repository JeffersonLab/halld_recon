// $Id$
//
//    File: DCustomAction_dirc_reactions.cc
//

#include "DCustomAction_dirc_reactions.h"

void DCustomAction_dirc_reactions::Initialize(JEventLoop* locEventLoop)
{
	DIRC_TRUTH_BARHIT = false;
	if(gPARMS->Exists("DIRC:TRUTH_BARHIT"))
		gPARMS->GetParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

	DIRC_FILL_BAR_MAP = false;
	gPARMS->SetDefaultParameter("DIRC:FILL_BAR_MAP",DIRC_FILL_BAR_MAP);

	// get PID algos
        const DParticleID* locParticleID = NULL;
        locEventLoop->GetSingle(locParticleID);
        dParticleID = locParticleID;

        locEventLoop->GetSingle(dDIRCLut);
	locEventLoop->GetSingle(dAnalysisUtilities);

	// get DIRC geometry
	vector<const DDIRCGeometry*> locDIRCGeometry;
        locEventLoop->Get(locDIRCGeometry);
        dDIRCGeometry = locDIRCGeometry[0];

	// set PID for different passes in debuging histograms
	dFinalStatePIDs.push_back(Positron);
	dFinalStatePIDs.push_back(PiPlus);
	dFinalStatePIDs.push_back(KPlus);
	dFinalStatePIDs.push_back(Proton);

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!
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

		hExtrapolatedBarHitXY = GetOrCreate_Histogram<TH2I>(Form("hExtrapolatedBarHitXY_%s",locParticleName.data()), "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);

		hDiff = GetOrCreate_Histogram<TH1I>(Form("hDiff_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleROOTName.data()), 400,-20,20);
		hNphC = GetOrCreate_Histogram<TH1I>(Form("hNphC_%s",locParticleName.data()), Form("# photons; %s # photons", locParticleROOTName.data()), 150, 0, 150);
		hThetaC = GetOrCreate_Histogram<TH1I>(Form("hThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #theta_{C} [rad]", locParticleROOTName.data()), 250, 0.6, 1.0);
		hDeltaThetaC = GetOrCreate_Histogram<TH1I>(Form("hDeltaThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		hLikelihood = GetOrCreate_Histogram<TH1I>(Form("hLikelihood_%s",locParticleName.data()), Form("; %s -lnL; entries [#]", locParticleROOTName.data()),1000,0.,1000.);
		hLikelihoodDiff = GetOrCreate_Histogram<TH1I>(Form("hLikelihoodDiff_%s",locParticleName.data()), Form("; %s;entries [#]", locLikelihoodName.data()),100,-200.,200.);
		
		hThetaCVsP = GetOrCreate_Histogram<TH2I>(Form("hThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 250, 0.6, 1.0);
		hDeltaThetaCVsP = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
		hLikelihoodDiffVsP = GetOrCreate_Histogram<TH2I>(Form("hLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName.data()), 120, 0.0, 12.0, 100, -200, 200);
		hReactionLikelihoodDiffVsP = GetOrCreate_Histogram<TH2I>(Form("hReactionLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName.data()), 120, 0.0, 12.0, 100, -200, 200);

		// Map of histograms for every bar, binned in x position
		if(DIRC_FILL_BAR_MAP) {

			for(int locBar=0; locBar<48; locBar++) {
				
				CreateAndChangeTo_Directory(Form("Bar %d", locBar), Form("Bar %d", locBar));
				double bar_y = dDIRCGeometry->GetBarY(locBar);
				
				for(int locXbin=0; locXbin<40; locXbin++) {
					
					double xbin_min = -100.0 + locXbin*5.0; 
					double xbin_max = xbin_min + 5.0;
					
					// skip creation of histograms for bins outside active area
					TVector3 locBinVect((xbin_min+xbin_max)/2., bar_y, 586.5 - 65.);
					if(locBinVect.Theta()*180/TMath::Pi() > 12.) 
						continue;
					
					hDiffMap[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hDiff_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; %s t_{calc}-t_{measured} [ns]; entries [#]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 80,-20,20);
					hNphCMap[locBar][locXbin] = GetOrCreate_Histogram<TH1I>(Form("hNphC_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f] # photons; %s # photons", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 80, 0, 80);
					
					hDeltaThetaCVsPMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hDeltaThetaCVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f] cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locBar,xbin_min,xbin_max,locParticleROOTName.data()), 60, 0.0, 12.0, 60,-0.15,0.15);
					hReactionLikelihoodDiffVsPMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hReactionLikelihoodDiffVsP_%s_%d_%d",locParticleName.data(),locBar,locXbin),  Form("Bar %d, xbin [%0.0f,%0.0f]; p (GeV/c); %s", locBar,xbin_min,xbin_max,locLikelihoodName.data()), 60, 0.0, 12.0, 50, -200, 200);
					
					hTruthPixelHitMap[locBar][locXbin] = GetOrCreate_Histogram<TH2I>(Form("hTruthPixelHit_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; Pixel Hit X ; Pixel Hit Y", locBar,xbin_min,xbin_max), 144, 0, 144, 48, 0, 48);
					
				}
				
				gDirectory->cd(".."); //End of single bar histograms
			}
		}		

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	japp->RootUnLock(); //RELEASE ROOT LOCK!!
}

bool DCustomAction_dirc_reactions::Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo)
{

	const DDetectorMatches* locDetectorMatches = NULL;
        locEventLoop->GetSingle(locDetectorMatches);

	// truth information on tracks hitting DIRC bar (for comparison)
	vector<const DDIRCTruthBarHit*> locDIRCBarHits;
	locEventLoop->Get(locDIRCBarHits);

	// Get selected particle from reaction for DIRC analysis
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(dParticleComboStepIndex);
	auto locParticle = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(dParticleIndex) : locParticleComboStep->Get_FinalParticle_Measured(dParticleIndex);

	// Get track and histogram DIRC quantities
	const DChargedTrack* locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(dParticleIndex));
	const DChargedTrackHypothesis* locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
	const DTrackTimeBased* locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
	
	// require well reconstructed tracks for initial studies
	int locDCHits = locTrackTimeBased->Ndof + 5;
	double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
	double locP = locParticle->lorentzMomentum().Vect().Mag();
	if(locDCHits < 25 || locTheta < 2.0 || locTheta > 12.0 || locP > 12.0)
		return true;
	
	// require has good match to TOF hit for cleaner sample
	shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	bool foundTOF = dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	if(!foundTOF || locTOFHitMatchParams->dDeltaXToHit > 10.0 || locTOFHitMatchParams->dDeltaYToHit > 10.0)
		return true;
	
	// get expected thetaC from the extrapolation
	vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_DIRC);
	if(extrapolations.size()==0) 
		return true;
	
	DVector3 momInBar = extrapolations[0].momentum;
	DVector3 posInBar = extrapolations[0].position;
	
	////////////////////////////////////////////
	// option to cheat and use truth position //
	////////////////////////////////////////////
	if(DIRC_TRUTH_BARHIT && locDIRCBarHits.size() > 0) {
		
		TVector3 bestMatchPos, bestMatchMom;
		double bestMatchDist = 999.;
		for(uint i=0; i<locDIRCBarHits.size(); i++) {
			TVector3 locDIRCBarHitPos(locDIRCBarHits[0]->x, locDIRCBarHits[0]->y, locDIRCBarHits[0]->z);
			TVector3 locDIRCBarHitMom(locDIRCBarHits[0]->px, locDIRCBarHits[0]->py, locDIRCBarHits[0]->pz);
			if((extrapolations[0].position - locDIRCBarHitPos).Mag() < bestMatchDist) {
				bestMatchDist = (extrapolations[0].position - locDIRCBarHitPos).Mag();
				bestMatchPos = locDIRCBarHitPos;
				bestMatchMom = locDIRCBarHitMom;
			}
		}
		
		momInBar = bestMatchMom;
		posInBar = bestMatchPos;
	}
	
	Lock_Action(); //ACQUIRE ROOT LOCK!!
	hExtrapolatedBarHitXY->Fill(posInBar.X(), posInBar.Y());
	Unlock_Action(); //RELEASE ROOT LOCK!!

	Particle_t locPID = locTrackTimeBased->PID();
	double locMass = locTrackTimeBased->mass();
	double locExpectedThetaC = acos(sqrt(momInBar.Mag()*momInBar.Mag() + locMass*locMass)/momInBar.Mag()/1.473);
	
	// get binning for histograms
	int locBar = dDIRCGeometry->GetBar(posInBar.Y());
	int locXbin = (int)(posInBar.X()/5.0) + 19;

	// check that histogram index exists
	TVector3 locBarHitVect(posInBar.X(), posInBar.Y(), posInBar.Z() - 65.);
	if(locBar < 0 || locXbin < 0 || locXbin > 39 || locBarHitVect.Theta()*180/TMath::Pi() > 12.0)
		return true;

	// get DIRC match parameters (contains LUT information)
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	bool foundDIRC = dParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
	vector<int> locUsedPixel;

	if(foundDIRC) {

		// recalculate likelihood
		double logLikelihoodSum[4] = {0, 0, 0, 0};
		double locExpectedAngle[4];
		for(int loc_i = 0; loc_i<4; loc_i++) 
			locExpectedAngle[loc_i] = acos(sqrt(locP*locP + ParticleMass(dFinalStatePIDs[loc_i])*ParticleMass(dFinalStatePIDs[loc_i]))/locP/1.473);
		
		// loop over hits associated with track (from LUT)
		//vector< pair<double,double> > locPhotons = locDIRCMatchParams->dPhotons;
		vector< vector<double> > locPhotons = locDIRCMatchParams->dPhotons;
		if(locPhotons.size() > 0) {

			// loop over candidate photons
			for(uint loc_j = 0; loc_j<locPhotons.size(); loc_j++) {
				double locThetaC = locPhotons[loc_j][0];				
				double locDeltaT = locPhotons[loc_j][1];
				int locChannel = (int)locPhotons[loc_j][2];
				if(locChannel >= 108*64) locChannel -= 108*64;

				// format final pixel x' and y' axes for view from behind PMTs looking downstream
				int pixel_x = dDIRCGeometry->GetPixelX(locChannel);
				int pixel_y = dDIRCGeometry->GetPixelY(locChannel);

				Lock_Action(); //ACQUIRE ROOT LOCK!!
				hDiff->Fill(locDeltaT);
				if(DIRC_FILL_BAR_MAP) hDiffMap[locBar][locXbin]->Fill(locDeltaT);
				Unlock_Action(); //RELEASE ROOT LOCK!!

				// fill histograms for candidate photons in timing cut
				if(fabs(locDeltaT) < 2.0) {
					Lock_Action(); //ACQUIRE ROOT LOCK!!
					hThetaC->Fill(locThetaC);
					hDeltaThetaC->Fill(locThetaC-locExpectedThetaC);
					hDeltaThetaCVsP->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
					if(DIRC_FILL_BAR_MAP) hDeltaThetaCVsPMap[locBar][locXbin]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
					Unlock_Action(); //RELEASE ROOT LOCK!!

					// likelihood recalculation
					if(fabs(locThetaC-0.5*(locExpectedAngle[1]+locExpectedAngle[2]))<0.02) {
						
						// calculate likelihood for each mass hypothesis
						for(uint loc_k = 0; loc_k<4; loc_k++) {
							logLikelihoodSum[loc_k] += TMath::Log( dDIRCLut->CalcLikelihood(locExpectedAngle[loc_k], locThetaC));
						}
					}
					
					if(std::find(locUsedPixel.begin(), locUsedPixel.end(), locChannel) != locUsedPixel.end()) 
						continue;
					
					Lock_Action(); //ACQUIRE ROOT LOCK!!
					if(DIRC_FILL_BAR_MAP && locP > 4.) {
						hTruthPixelHitMap[locBar][locXbin]->Fill(pixel_x, pixel_y);
					}
					locUsedPixel.push_back(locChannel);
					Unlock_Action(); //RELEASE ROOT LOCK!!
				}
			}	  
		}			  
		
		// fill histograms with per-track quantities
		Lock_Action(); //ACQUIRE ROOT LOCK!!
		hNphC->Fill(locDIRCMatchParams->dNPhotons);
		hThetaCVsP->Fill(momInBar.Mag(), locDIRCMatchParams->dThetaC); 
		if(DIRC_FILL_BAR_MAP) hNphCMap[locBar][locXbin]->Fill(locDIRCMatchParams->dNPhotons);
		
		// for likelihood and difference for given track mass hypothesis
		if(locPID == Positron || locPID == Electron) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodElectron);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[0]-logLikelihoodSum[1]);
			if(DIRC_FILL_BAR_MAP) 
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[0]-logLikelihoodSum[1]);
		}
		else if(locPID == PiPlus || locPID == PiMinus) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodPion);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[1]-logLikelihoodSum[2]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[1]-logLikelihoodSum[2]);
		}
		else if(locPID == KPlus || locPID == KMinus) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[1]-logLikelihoodSum[2]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[1]-logLikelihoodSum[2]);
		}
		else if(locPID == Proton || locPID == AntiProton) {
			hLikelihood->Fill(-1. * locDIRCMatchParams->dLikelihoodProton);
			hLikelihoodDiff->Fill(locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
			hLikelihoodDiffVsP->Fill(locP, locDIRCMatchParams->dLikelihoodKaon - locDIRCMatchParams->dLikelihoodProton);
			hReactionLikelihoodDiffVsP->Fill(locP, logLikelihoodSum[3]-logLikelihoodSum[2]);
			if(DIRC_FILL_BAR_MAP)
				hReactionLikelihoodDiffVsPMap[locBar][locXbin]->Fill(locP, logLikelihoodSum[3]-logLikelihoodSum[2]);
		}
		Unlock_Action(); //RELEASE ROOT LOCK!!

	}

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}

// -----------------------------------------
// DEventProcessor_dirc_hists.cc
// -----------------------------------------

#include "DEventProcessor_dirc_hists.h"

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_dirc_hists());
  }
}

DEventProcessor_dirc_hists::DEventProcessor_dirc_hists() {
}

DEventProcessor_dirc_hists::~DEventProcessor_dirc_hists() {
}

jerror_t DEventProcessor_dirc_hists::init(void) {
  
  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
	  gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  DIRC_TRUTH_BARHIT = false;
  if(gPARMS->Exists("DIRC:TRUTH_BARHIT"))
	  gPARMS->GetParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

  TDirectory *dir = new TDirectoryFile("DIRC","DIRC");
  dir->cd();
 
  // list of particle IDs for histograms (and alternate hypotheses for likelihood diff)
  deque<TString> locLikelihoodName;
  dFinalStatePIDs.push_back(Positron);    locLikelihoodName.push_back("ln L(e+) - ln L(#pi+)");
  dFinalStatePIDs.push_back(Electron);    locLikelihoodName.push_back("ln L(e-) - ln L(#pi-)");
  dFinalStatePIDs.push_back(PiPlus);      locLikelihoodName.push_back("ln L(#pi+) - ln L(K+)");
  dFinalStatePIDs.push_back(PiMinus);     locLikelihoodName.push_back("ln L(#pi-) - ln L(K-)");
  dFinalStatePIDs.push_back(KPlus);       locLikelihoodName.push_back("ln L(#pi+) - ln L(K+)");
  dFinalStatePIDs.push_back(KMinus);      locLikelihoodName.push_back("ln L(#pi-) - ln L(K-)");
  dFinalStatePIDs.push_back(Proton);      locLikelihoodName.push_back("ln L(K+) - ln L(p)");
  dFinalStatePIDs.push_back(AntiProton);  locLikelihoodName.push_back("ln L(K-) - ln L(#bar{p}");
 
  // plots for each hypothesis
  for(uint loc_i=0; loc_i<dFinalStatePIDs.size(); loc_i++) {
	  Particle_t locPID = dFinalStatePIDs[loc_i];
	  string locParticleName = ParticleType(locPID);
	  string locParticleROOTName = ParticleName_ROOT(locPID);
	  
	  TDirectory *locParticleDir = new TDirectoryFile(locParticleName.data(),locParticleName.data());
	  locParticleDir->cd();

	  hDiff[locPID] = new TH1I(Form("hDiff_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleROOTName.data()), 400,-20,20);
	  hNphC[locPID] = new TH1I(Form("hNphC_%s",locParticleName.data()), Form("# photons; %s # photons", locParticleROOTName.data()), 150, 0, 150);
	  hThetaC[locPID] = new TH1I(Form("hThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #theta_{C} [rad]", locParticleROOTName.data()), 250, 0.6, 1.0);
	  hDeltaThetaC[locPID] = new TH1I(Form("hDeltaThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
	  hLikelihood[locPID] = new TH1I(Form("hLikelihood_%s",locParticleName.data()), Form("; %s -lnL; entries [#]", locParticleROOTName.data()),1000,0.,1000.);
	  hLikelihoodDiff[locPID] = new TH1I(Form("hLikelihoodDiff_%s",locParticleName.data()), Form("; %s;entries [#]", locLikelihoodName[loc_i].Data()),100,-200.,200.);


	  hThetaCVsP[locPID] = new TH2I(Form("hThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 250, 0.75, 0.85);
	  hDeltaThetaCVsP[locPID] = new TH2I(Form("hDeltaThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
	  hLikelihoodDiffVsP[locPID] = new TH2I(Form("hLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName[loc_i].Data()), 120, 0.0, 12.0, 100, -200, 200);

	  hDeltaTVsP[locPID] = new TH2I(Form("hDeltaTVsP_%s",locParticleName.data()), Form("#Delta T vs. momentum; p (GeV/c); %s #Delta T (ns)", locParticleROOTName.data()), 120, 0.0, 12.0, 100, -1.5, 1.5);

	  dir->cd();
  }

  gDirectory->cd("/");
 
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::brun(jana::JEventLoop *loop, int32_t runnumber)
{
   // get PID algos
   const DParticleID* locParticleID = NULL;
   loop->GetSingle(locParticleID);
   dParticleID = locParticleID;

   return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::evnt(JEventLoop *loop, uint64_t eventnumber) {

  // retrieve tracks and detector matches 
  vector<const DTrackTimeBased*> locTimeBasedTracks;
  loop->Get(locTimeBasedTracks);

  const DDetectorMatches* locDetectorMatches = NULL;
  loop->GetSingle(locDetectorMatches);

  // plot DIRC LUT variables for specific tracks  
  for (unsigned int loc_i = 0; loc_i < locTimeBasedTracks.size(); loc_i++){

	  const DTrackTimeBased* locTrackTimeBased = locTimeBasedTracks[loc_i];

	  // require well reconstructed tracks for initial studies
	  int locDCHits = locTrackTimeBased->Ndof + 5;
	  double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
	  double locP = locTrackTimeBased->momentum().Mag();
	  if(locDCHits < 15 || locTheta < 1.0 || locTheta > 12.0 || locP > 12.0)
		  continue;

	  // require has good match to TOF hit for cleaner sample
	  shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	  bool foundTOF = dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	  if(!foundTOF || locTOFHitMatchParams->dDeltaXToHit > 10.0 || locTOFHitMatchParams->dDeltaYToHit > 10.0)
		  continue;

	  Particle_t locPID = locTrackTimeBased->PID();

	  // get DIRC match parameters (contains LUT information)
	  shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	  bool foundDIRC = dParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
	  
	  if(foundDIRC) {

		  DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
		  DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
		  double locExpectedThetaC = locDIRCMatchParams->dExpectedThetaC;

		  // loop over hits associated with track (from LUT)
		  vector< vector<double> > locPhotons = locDIRCMatchParams->dPhotons;
		  if(locPhotons.size() > 0) {
			  
			  // loop over candidate photons
			  for(uint loc_j = 0; loc_j<locPhotons.size(); loc_j++) {
				  double locThetaC = locPhotons[loc_j][0];
				  double locDeltaT = locPhotons[loc_j][1];
				  //int locSensorId = (int)locPhotons[loc_j][2];
				  if(fabs(locThetaC-locExpectedThetaC)<0.02)
					  hDiff[locPID]->Fill(locDeltaT);
				  
				  // fill histograms for candidate photons in timing cut
				  if(fabs(locDeltaT) < 2.0) {
					  hThetaC[locPID]->Fill(locThetaC);
					  hDeltaThetaC[locPID]->Fill(locThetaC-locExpectedThetaC);
					  hDeltaThetaCVsP[locPID]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
				  }
			  }	  
		  }			  
		  
		  // remove final states not considered
		  if(std::find(dFinalStatePIDs.begin(),dFinalStatePIDs.end(),locPID) == dFinalStatePIDs.end())
			  continue;
		  
		  // fill histograms with per-track quantities
		  hNphC[locPID]->Fill(locDIRCMatchParams->dNPhotons);
		  hThetaCVsP[locPID]->Fill(momInBar.Mag(), locDIRCMatchParams->dThetaC); 
		  hDeltaTVsP[locPID]->Fill(momInBar.Mag(), locDIRCMatchParams->dDeltaT);

		  // for likelihood and difference for given track mass hypothesis
		  if(locPID == Positron || locPID == Electron) {
			  hLikelihood[locPID]->Fill(-1. * locDIRCMatchParams->dLikelihoodElectron);
			  hLikelihoodDiff[locPID]->Fill(locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			  hLikelihoodDiffVsP[locPID]->Fill(locP, locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
		  }
		  else if(locPID == PiPlus || locPID == PiMinus) {
			  hLikelihood[locPID]->Fill(-1. * locDIRCMatchParams->dLikelihoodPion);
			  hLikelihoodDiff[locPID]->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID]->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
		  }
		  else if(locPID == KPlus || locPID == KMinus) {
			  hLikelihood[locPID]->Fill(-1. * locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiff[locPID]->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID]->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
		  }
		  else if(locPID == Proton) {
			  hLikelihood[locPID]->Fill(-1. * locDIRCMatchParams->dLikelihoodProton);
			  hLikelihoodDiff[locPID]->Fill(locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID]->Fill(locP, locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
		  }
	  }
  }
  
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::erun(void) {
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::fini(void) {
  return NOERROR;
}

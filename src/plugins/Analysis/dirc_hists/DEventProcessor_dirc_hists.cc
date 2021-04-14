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
  
  DIRC_TRUTH_BARHIT = false;
  if(gPARMS->Exists("DIRC:TRUTH_BARHIT"))
	  gPARMS->GetParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

  DIRC_CUT_TDIFF = 3.0;
  if(gPARMS->Exists("DIRC:HIST_CUT_TDIFF"))
          gPARMS->GetParameter("DIRC:HIST_CUT_TDIFF",DIRC_CUT_TDIFF);

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

  dMaxChannels = DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels;

  // plots for each bar
  TDirectory *locBarDir = new TDirectoryFile("PerBarDiagnostic","PerBarDiagnostic");
  locBarDir->cd();
  for(int i=0; i<DDIRCGeometry::kBars; i++) {
	  hDiffBar[i] = new TH2I(Form("hDiff_bar%02d",i), Form("Bar %02d; Channel ID; t_{calc}-t_{measured} [ns]; entries [#]", i), dMaxChannels, 0, dMaxChannels, 400,-20,20);
	  hNphCBar[i] = new TH1I(Form("hNphC_bar%02d",i), Form("Bar %02d; # photons", i), 150, 0, 150);
	  hNphCBarVsP[i] = new TH2I(Form("hNphCVsP_bar%d",i), Form("Bar %02d # photons vs. momentum; p (GeV/c); # photons", i), 120, 0, 12.0, 150, 0, 150);
	  hNphCBarInclusive[i] = new TH1I(Form("hNphCInclusive_bar%02d",i), Form("Bar %02d; # photons", i), 150, 0, 150);
	  hNphCBarInclusiveVsP[i] = new TH2I(Form("hNphCInclusiveVsP_bar%d",i), Form("Bar %02d # photons vs. momentum; p (GeV/c); # photons", i), 120, 0, 12.0, 150, 0, 150);
	  hDeltaThetaCBar[i] = new TH1I(Form("hDeltaThetaC_bar%d",i), Form("Bar %02d cherenkov angle; #Delta#theta_{C} [rad]", i), 200,-0.2,0.2);
	  hDeltaThetaCVsDeltaYBar[i] = new TH2I(Form("hDeltaThetaCVsDeltaYBar_bar%d", i), Form("Bar %02d cherenkov angle vs. Y position vs cherenkov angle; #Delta#theta_{C} [rad]; #Delta Y (cm)", i), 200,-0.2,0.2, 100, -2.0, 2.0);
	  for(int locXbin=0; locXbin<40; locXbin++) {
	    double xbin_min = -100.0 + locXbin*5.0;
	    double xbin_max = xbin_min + 5.0;
	    
	    hDeltaThetaCVsDeltaYBarX[i][locXbin] = new TH2I(Form("hDeltaThetaCVsDeltaYBar_bar%d_%d", i,locXbin), Form("Bar %02d, xbin [%0.0f,%0.0f] Y position vs cherenkov angle; #Delta#theta_{C} [rad]; #Delta Y (cm)", i,xbin_min,xbin_max), 200,-0.2,0.2, 100, -2.0, 2.0);
	    hDeltaThetaCVsDeltaYBarXPos[i][locXbin] = new TH2I(Form("hDeltaThetaCVsDeltaYBarPos_bar%d_%d", i,locXbin), Form("PiPlus Bar %02d, xbin [%0.0f,%0.0f] Y position vs cherenkov angle; #Delta#theta_{C} [rad]; #Delta Y (cm)", i,xbin_min,xbin_max), 200,-0.2,0.2, 100, -2.0, 2.0);
	    hDeltaThetaCVsDeltaYBarXNeg[i][locXbin] = new TH2I(Form("hDeltaThetaCVsDeltaYBarNeg_bar%d_%d", i,locXbin), Form("PiMinus Bar %02d, xbin [%0.0f,%0.0f] Y position vs cherenkov angle; #Delta#theta_{C} [rad]; #Delta Y (cm)", i,xbin_min,xbin_max), 200,-0.2,0.2, 100, -2.0, 2.0);
	  }
  }
  dir->cd();
 
  // plots for each hypothesis
  for(uint loc_i=0; loc_i<dFinalStatePIDs.size(); loc_i++) {
	  Particle_t locPID = dFinalStatePIDs[loc_i];
	  hExtrapolatedBarHitTime_BadTime[locPID].resize(2);
	  hExtrapolatedBarHitXY_BadTime[locPID].resize(2);
	  hExtrapolatedBarTimeVsPixelHitTime[locPID].resize(2);
	  hExtrapolatedBarTimeVsPixelHitTime_BadTime[locPID].resize(2);
	  hDiffVsEventNumber[locPID].resize(2);

	  hDiff[locPID].resize(2);
	  hDiffVsChannelDirect[locPID].resize(2);
	  hDiffVsChannelReflected[locPID].resize(2);
	  hNphC[locPID].resize(2);
	  hNphCInclusive[locPID].resize(2);
	  hThetaC[locPID].resize(2);
	  hDeltaThetaC[locPID].resize(2);
	  hDeltaThetaC_BadTime[locPID].resize(2);
	  hLikelihood[locPID].resize(2);
	  hLikelihoodDiff[locPID].resize(2);
	  
	  hNphCVsP[locPID].resize(2);
	  hNphCInclusiveVsP[locPID].resize(2);
	  hThetaCVsP[locPID].resize(2);
	  hDeltaThetaCVsP[locPID].resize(2);
	  hLikelihoodDiffVsP[locPID].resize(2);
	  hDeltaTVsP[locPID].resize(2);

	  hExtrapolationTimeVsStartTime[locPID].resize(2);
	  hExtrapolationTimeVsStartTime_BadTime[locPID].resize(2);
	  hTimeCalcVsMeas[locPID].resize(2);
	  hPixelHitMap_BadTime[locPID].resize(2);
  }
  
  for(uint loc_i=0; loc_i<dFinalStatePIDs.size(); loc_i++) {
	  Particle_t locPID = dFinalStatePIDs[loc_i];
	  string locParticleName = ParticleType(locPID);
	  string locParticleROOTName = ParticleName_ROOT(locPID);
	  
	  TDirectory *locParticleDir = new TDirectoryFile(locParticleName.data(),locParticleName.data());
	  locParticleDir->cd();
	  
	  hExtrapolatedBarHitXY[locPID] = new TH2I(Form("hExtrapolatedBarHitXY_%s",locParticleName.data()), "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);
	  hExtrapolatedBarHitTime[locPID] = new TH1I(Form("hExtrapolatedBarHitTime_%s",locParticleName.data()), "; Bar Hit Time (ns)", 200, 0, 100);

	  string boxName[2] = {"North", "South"};
	  for(uint loc_box = 0; loc_box<2; loc_box++) {
		  TDirectory *locBoxDir = new TDirectoryFile(boxName[loc_box].data(),boxName[loc_box].data());
		  locBoxDir->cd();

		  hExtrapolatedBarHitTime_BadTime[locPID][loc_box] = new TH1I(Form("hExtrapolatedBarHitTime_BadTime%s",locParticleName.data()), "; Bar Hit Time (ns)", 200, 0, 100);
		  hExtrapolatedBarHitXY_BadTime[locPID][loc_box] = new TH2I(Form("hExtrapolatedBarHitXY_BadTime_%s",locParticleName.data()), "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100); 
		  hExtrapolatedBarTimeVsPixelHitTime[locPID][loc_box] = new TH2I(Form("hExtrapolatedBarTimeVsPixelHitTime_%s",locParticleName.data()), "; Pixel Hit Time (nx); Bar Hit Time (ns)", 200, 0, 100, 200, 0, 100);
		  hExtrapolatedBarTimeVsPixelHitTime_BadTime[locPID][loc_box] = new TH2I(Form("hExtrapolatedBarTimeVsPixelHitTime_BadTime_%s",locParticleName.data()), "; Pixel Hit Time (ns); Bar Hit Time (ns)", 200, 0, 100, 200, 0, 100);

		  hDiff[locPID][loc_box] = new TH1I(Form("hDiff_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; entries [#]", locParticleName.data()), 400,-100,100);

		  hDiffVsEventNumber[locPID][loc_box] = new TH2I(Form("hDiffVsEventNumber_%s",locParticleName.data()), Form("; %s t_{calc}-t_{measured} [ns]; event number; entries [#]", locParticleName.data()), 1000, 0, 1e6, 400,-100,100);

		  hDiffVsChannelDirect[locPID][loc_box] = new TH2I(Form("hDiffVsChannelDirect_%s",locParticleName.data()), Form("; Channel ID; %s t_{calc}-t_{measured} [ns]; entries [#]",locParticleName.data()), dMaxChannels, 0, dMaxChannels, 400,-20,20);
		  hDiffVsChannelReflected[locPID][loc_box] = new TH2I(Form("hDiffVsChannelReflected_%s",locParticleName.data()), Form("; Channel ID; %s t_{calc}-t_{measured} [ns]; entries [#]",locParticleName.data()), dMaxChannels, 0, dMaxChannels, 400,-20,20);
		  hNphC[locPID][loc_box] = new TH1I(Form("hNphC_%s",locParticleName.data()), Form("# photons; %s # photons", locParticleROOTName.data()), 150, 0, 150);
		  hNphCInclusive[locPID][loc_box] = new TH1I(Form("hNphCInclusive_%s",locParticleName.data()), Form("# photons; %s # photons", locParticleROOTName.data()), 150, 0, 150);
		  hThetaC[locPID][loc_box] = new TH1I(Form("hThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #theta_{C} [rad]", locParticleROOTName.data()), 250, 0.6, 1.0);
		  hDeltaThetaC[locPID][loc_box] = new TH1I(Form("hDeltaThetaC_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		  hDeltaThetaC_BadTime[locPID][loc_box] = new TH1I(Form("hDeltaThetaC_BadTime_%s",locParticleName.data()), Form("cherenkov angle; %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 200,-0.2,0.2);
		  hLikelihood[locPID][loc_box] = new TH1I(Form("hLikelihood_%s",locParticleName.data()), Form("; %s -lnL; entries [#]", locParticleROOTName.data()),1000,0.,1000.);
		  hLikelihoodDiff[locPID][loc_box] = new TH1I(Form("hLikelihoodDiff_%s",locParticleName.data()), Form("; %s;entries [#]", locLikelihoodName[loc_i].Data()),100,-200.,200.);
		  
		  
		  hNphCVsP[locPID][loc_box] = new TH2I(Form("hNphCVsP_%s",locParticleName.data()), Form("# photons vs. momentum; p (GeV/c); %s # photons", locParticleROOTName.data()), 120, 0, 12.0, 150, 0, 150);
		  hNphCInclusiveVsP[locPID][loc_box] = new TH2I(Form("hNphCInclusiveVsP_%s",locParticleName.data()), Form("# photons vs. momentum; p (GeV/c); %s # photons", locParticleROOTName.data()), 120, 0, 12.0, 150, 0, 150);
		  hThetaCVsP[locPID][loc_box] = new TH2I(Form("hThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 250, 0.75, 0.85);
		  hDeltaThetaCVsP[locPID][loc_box] = new TH2I(Form("hDeltaThetaCVsP_%s",locParticleName.data()),  Form("cherenkov angle vs. momentum; p (GeV/c); %s #Delta#theta_{C} [rad]", locParticleROOTName.data()), 120, 0.0, 12.0, 200,-0.2,0.2);
		  hLikelihoodDiffVsP[locPID][loc_box] = new TH2I(Form("hLikelihoodDiffVsP_%s",locParticleName.data()),  Form("; p (GeV/c); %s", locLikelihoodName[loc_i].Data()), 120, 0.0, 12.0, 100, -200, 200);
		  
		  hDeltaTVsP[locPID][loc_box] = new TH2I(Form("hDeltaTVsP_%s",locParticleName.data()), Form("#Delta T vs. momentum; p (GeV/c); %s #Delta T (ns)", locParticleROOTName.data()), 120, 0.0, 12.0, 200, -100, 100);
		  
		  hExtrapolationTimeVsStartTime[locPID][loc_box] = new TH2I(Form("hExtrapolationTimeVsStartTime_%s",locParticleName.data()), Form("%s; t_0 start time (ns); track extrapolation time (ns)", locParticleName.data()), 200, -40, 40, 200, -40, 40);
		  hExtrapolationTimeVsStartTime_BadTime[locPID][loc_box] = new TH2I(Form("hExtrapolationTimeVsStartTime_BadTime_%s",locParticleName.data()), Form("%s; t_0 start time (ns); track extrapolation time (ns)", locParticleName.data()), 200, -40, 40, 200, -40, 40);
		  
		  hTimeCalcVsMeas[locPID][loc_box] = new TH2I(Form("hTimeCalcVsMeas_%s",locParticleName.data()), Form("%s; Measured time (ns); Calculated time (ns)", locParticleName.data()), 200, 0, 200, 200, 0, 200);
		  hPixelHitMap_BadTime[locPID][loc_box] = new TH2S(Form("hPixelHit_BadTime_%s",locParticleName.data()), Form("%s; pixel rows; pixel columns", locParticleName.data()), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
		  
		  locParticleDir->cd();
	  }
	  dir->cd();
  }
  
  int locBar = 3;
  string locParticleName = "PiPlus";
  // occupancy for fixed position and momentum
  TDirectory *locMapDir = new TDirectoryFile("HitMapBar3","HitMapBar3");
  locMapDir->cd();
  for(int locXbin=0; locXbin<40; locXbin++) {
	  double xbin_min = -100.0 + locXbin*5.0;
	  double xbin_max = xbin_min + 5.0;
	 
	  hHitTimeMap[locXbin] = new TH1I(Form("hHitTimeMap_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; t_{measured} [ns]; entries [#]",locBar,xbin_min,xbin_max), 100,0,100); 
	  hPixelHitMap[locXbin] = new TH2S(Form("hPixelHit_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
	  hPixelHitMapReflected[locXbin] = new TH2S(Form("hPixelHitReflected_%s_%d_%d",locParticleName.data(),locBar,locXbin), Form("Bar %d, xbin [%0.0f,%0.0f]; pixel rows; pixel columns", locBar,xbin_min,xbin_max), 144, -0.5, 143.5, DDIRCGeometry::kBars, -0.5, 47.5);
  }

  gDirectory->cd("/");
 
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::brun(jana::JEventLoop *loop, int32_t runnumber)
{

   return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::evnt(JEventLoop *loop, uint64_t eventnumber) {

  // check trigger type
  const DTrigger* locTrigger = NULL;
  loop->GetSingle(locTrigger);
  if(!locTrigger->Get_IsPhysicsEvent())
	  return NOERROR;

   // get PID algos
   const DParticleID* locParticleID = NULL;
   loop->GetSingle(locParticleID);

   vector<const DDIRCGeometry*> locDIRCGeometryVec;
   loop->Get(locDIRCGeometryVec);
   auto locDIRCGeometry = locDIRCGeometryVec[0];

   // Initialize DIRC LUT
   const DDIRCLut* dDIRCLut = nullptr;
  loop->GetSingle(dDIRCLut);

  // retrieve tracks and detector matches 
  vector<const DTrackTimeBased*> locTimeBasedTracks;
  loop->Get(locTimeBasedTracks);

  vector<const DDIRCPmtHit*> locDIRCPmtHits;
  loop->Get(locDIRCPmtHits);

  const DDetectorMatches* locDetectorMatches = NULL;
  loop->GetSingle(locDetectorMatches);
  DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];

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
	  bool foundTOF = locParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	  if(!foundTOF || locTOFHitMatchParams->dDeltaXToHit > 10.0 || locTOFHitMatchParams->dDeltaYToHit > 10.0)
		  continue;

	  Particle_t locPID = locTrackTimeBased->PID();
	  double locMass = ParticleMass(locPID);

	  double locInputStartTime = 0.; //locTrackTimeBased->t0();
	  //if(locTrackTimeBased->t0_detector() == SYS_TOF) {
		  //cout<<"TOF hit"<<endl;
		  //continue;
	  //}
	  //vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_DIRC);
	  //if(extrapolations.size()==0) continue;
	  double locExtrapolationTime = 0.; //extrapolations[0].t;

	  // get DIRC match parameters (contains LUT information)
	  shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	  bool foundDIRC = locParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
	 
	  if(foundDIRC) {

		  TVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
		  TVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
		  double locExpectedThetaC = locDIRCMatchParams->dExpectedThetaC;
		  double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
		  int locBar = locDIRCGeometry->GetBar(posInBar.Y());
		  //if(fabs(posInBar.Y() - locDIRCGeometry->GetBarY(locBar)) > 1.0)
		  //  continue;

		  int locBox = 1;
		  if(locBar > 23) //continue; // skip north box for now
			  locBox = 0;

		  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
                  hExtrapolatedBarHitXY[locPID]->Fill(posInBar.X(), posInBar.Y());
		  hExtrapolatedBarHitTime[locPID]->Fill(locExtrapolatedTime);
        	  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK          

		  double locAngle = dDIRCLut->CalcAngle(locP, locMass);
		  map<Particle_t, double> locExpectedAngle = dDIRCLut->CalcExpectedAngles(locP);

		  // get map of DIRCMatches to PMT hits
		  map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParamsMap;
		  locDetectorMatch.Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParamsMap);
		  map<Particle_t, double> logLikelihoodSum;

	          int locPhotonInclusive = 0;

		  // loop over associated hits for LUT diagnostic plots
		  for(uint loc_i=0; loc_i<locDIRCPmtHits.size(); loc_i++) {
		          bool locIsReflected = false;
			  vector<pair<double, double>> locDIRCPhotons = dDIRCLut->CalcPhoton(locDIRCPmtHits[loc_i], locExtrapolatedTime, posInBar, momInBar, locExpectedAngle, locAngle, locPID, locIsReflected, logLikelihoodSum);
			  double locHitTime = locDIRCPmtHits[loc_i]->t - locExtrapolatedTime;
			  int locChannel = locDIRCPmtHits[loc_i]->ch%dMaxChannels;
			  if(locHitTime > 0 && locHitTime < 150) locPhotonInclusive++;

			  int pixel_row = locDIRCGeometry->GetPixelRow(locChannel);
			  int pixel_col = locDIRCGeometry->GetPixelColumn(locChannel);

			  // if find track which points to relevant bar, fill photon yield and matched
			  int locXbin = (int)(posInBar.X()/5.0) + 19;
			  if(locXbin >= 0 && locXbin < 40 && locBar == 3 && locPID == PiPlus && momInBar.Mag() > 4.0) {
				  
				  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
				  hHitTimeMap[locXbin]->Fill(locHitTime);
				  if(locHitTime < 38)
					  hPixelHitMap[locXbin]->Fill(pixel_row, pixel_col);
				  else
					  hPixelHitMapReflected[locXbin]->Fill(pixel_row, pixel_col);	  
				  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
			  }

			  if(locDIRCPhotons.size() > 0) {

				  // loop over candidate photons
				  for(uint loc_j = 0; loc_j<locDIRCPhotons.size(); loc_j++) {
					  double locDeltaT = locDIRCPhotons[loc_j].first - locHitTime;
					  double locThetaC = locDIRCPhotons[loc_j].second;

					  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

					  if(fabs(locDeltaT + 15.) < 5.)
						  hDeltaThetaC_BadTime[locPID][locBox]->Fill(locThetaC-locExpectedThetaC);

					  if(fabs(locThetaC-locExpectedThetaC)<0.05) {
						  hTimeCalcVsMeas[locPID][locBox]->Fill(locHitTime, locDIRCPhotons[loc_j].first);
						  
						  if(fabs(locDeltaT + 15.) < 5.) {
							  hPixelHitMap_BadTime[locPID][locBox]->Fill(pixel_row, pixel_col);
							  hExtrapolatedBarHitXY_BadTime[locPID][locBox]->Fill(posInBar.X(), posInBar.Y());
							  hExtrapolatedBarHitTime_BadTime[locPID][locBox]->Fill(locExtrapolatedTime);
							  hExtrapolatedBarTimeVsPixelHitTime_BadTime[locPID][locBox]->Fill(locDIRCPmtHits[loc_i]->t, locExtrapolatedTime);
							  hExtrapolationTimeVsStartTime_BadTime[locPID][locBox]->Fill(locInputStartTime, locExtrapolationTime);
						  }
						  else if(fabs(locDeltaT) < 5.) {
							  hExtrapolatedBarTimeVsPixelHitTime[locPID][locBox]->Fill(locDIRCPmtHits[loc_i]->t, locExtrapolatedTime);
							  hExtrapolationTimeVsStartTime[locPID][locBox]->Fill(locInputStartTime, locExtrapolationTime);
						  }
					  }					  

					  // diagnostic plots for good cherenkov photons
					  if(fabs(locThetaC-locExpectedThetaC)<0.05) {
						  hDiff[locPID][locBox]->Fill(locDeltaT);
						  hDiffVsEventNumber[locPID][locBox]->Fill(eventnumber, locDeltaT);
						  if(locHitTime < 38)
							  hDiffVsChannelDirect[locPID][locBox]->Fill(locChannel,locDeltaT);
						  else 
							  hDiffVsChannelReflected[locPID][locBox]->Fill(locChannel,locDeltaT);
						  if(locPID == PiPlus || locPID == PiMinus) 
							  hDiffBar[locBar]->Fill(locChannel,locDeltaT);
					  }
					  
					  // fill histograms for candidate photons in timing cut
					  if(fabs(locDeltaT) < DIRC_CUT_TDIFF) {
						  hThetaC[locPID][locBox]->Fill(locThetaC);
						  hDeltaThetaC[locPID][locBox]->Fill(locThetaC-locExpectedThetaC);
						  hDeltaThetaCVsP[locPID][locBox]->Fill(momInBar.Mag(), locThetaC-locExpectedThetaC);
						  if(locPID == PiPlus || locPID == PiMinus) {
							  hDeltaThetaCBar[locBar]->Fill(locThetaC-locExpectedThetaC);
							  if(momInBar.Mag() > 2.0) {
							    hDeltaThetaCVsDeltaYBar[locBar]->Fill(locThetaC-locExpectedThetaC,posInBar.Y() - locDIRCGeometry->GetBarY(locBar));
							    hDeltaThetaCVsDeltaYBarX[locBar][locXbin]->Fill(locThetaC-locExpectedThetaC,posInBar.Y() - locDIRCGeometry->GetBarY(locBar));
							    if(locPID == PiPlus) hDeltaThetaCVsDeltaYBarXPos[locBar][locXbin]->Fill(locThetaC-locExpectedThetaC,posInBar.Y() - locDIRCGeometry->GetBarY(locBar));
							    else hDeltaThetaCVsDeltaYBarXNeg[locBar][locXbin]->Fill(locThetaC-locExpectedThetaC,posInBar.Y() - locDIRCGeometry->GetBarY(locBar));
							  }
						  }
					  }
					  
					  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
				  }
			  }
		  }
		  
		  // remove final states not considered
		  if(std::find(dFinalStatePIDs.begin(),dFinalStatePIDs.end(),locPID) == dFinalStatePIDs.end())
			  continue;
		    
		  japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

		  // fill histograms with per-track quantities
		  hNphC[locPID][locBox]->Fill(locDIRCMatchParams->dNPhotons);
		  hNphCInclusive[locPID][locBox]->Fill(locPhotonInclusive);
		  hNphCVsP[locPID][locBox]->Fill(momInBar.Mag(), locDIRCMatchParams->dNPhotons);
		  hNphCInclusiveVsP[locPID][locBox]->Fill(momInBar.Mag(), locPhotonInclusive);
		  hThetaCVsP[locPID][locBox]->Fill(momInBar.Mag(), locDIRCMatchParams->dThetaC); 
		  hDeltaTVsP[locPID][locBox]->Fill(momInBar.Mag(), locDIRCMatchParams->dDeltaT);

		  if(locPID == PiPlus || locPID == PiMinus) {
			  hNphCBar[locBar]->Fill(locDIRCMatchParams->dNPhotons);
			  hNphCBarInclusive[locBar]->Fill(locPhotonInclusive);
			  hNphCBarVsP[locBar]->Fill(momInBar.Mag(), locDIRCMatchParams->dNPhotons);
			  hNphCBarInclusiveVsP[locBar]->Fill(momInBar.Mag(), locPhotonInclusive);
		  }

		  // for likelihood and difference for given track mass hypothesis
		  if(locPID == Positron || locPID == Electron) {
			  hLikelihood[locPID][locBox]->Fill(-1. * locDIRCMatchParams->dLikelihoodElectron);
			  hLikelihoodDiff[locPID][locBox]->Fill(locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
			  hLikelihoodDiffVsP[locPID][locBox]->Fill(locP, locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
		  }
		  else if(locPID == PiPlus || locPID == PiMinus) {
			  hLikelihood[locPID][locBox]->Fill(-1. * locDIRCMatchParams->dLikelihoodPion);
			  hLikelihoodDiff[locPID][locBox]->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID][locBox]->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
		  }
		  else if(locPID == KPlus || locPID == KMinus) {
			  hLikelihood[locPID][locBox]->Fill(-1. * locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiff[locPID][locBox]->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID][locBox]->Fill(locP, locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
		  }
		  else if(locPID == Proton) {
			  hLikelihood[locPID][locBox]->Fill(-1. * locDIRCMatchParams->dLikelihoodProton);
			  hLikelihoodDiff[locPID][locBox]->Fill(locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiffVsP[locPID][locBox]->Fill(locP, locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
		  }

		  japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
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

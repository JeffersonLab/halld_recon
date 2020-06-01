// $Id$
//
//    File: DEventProcessor_dirc_tree.cc
//

#include "DEventProcessor_dirc_tree.h"

// Routine used to create our DEventProcessor

extern "C"
{
  void InitPlugin(JApplication *locApplication)
  {
    InitJANAPlugin(locApplication);
    locApplication->AddProcessor(new DEventProcessor_dirc_tree()); //register this plugin
    locApplication->AddFactoryGenerator(new DFactoryGenerator_dirc_tree()); //register the factory generator
  }
} // "C"

//------------------
// init
//------------------
jerror_t DEventProcessor_dirc_tree::init(void)
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
  // japp->RootWriteLock();
  //  ... create historgrams or trees ...
  // japp->RootUnLock();
  //

  japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!

  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
    gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->Cd("/");

  TObject* locTree = gDirectory->Get("dirc");
  if(locTree == NULL)
    fTree = new TTree("dirc", "dirc tree");
  else
    fTree = static_cast<TTree*>(locTree);

  fcEvent = new TClonesArray("DrcEvent");
  fTree->Branch("DrcEvent",&fcEvent,256000,2);
  
  japp->RootUnLock(); //RELEASE ROOT LOCK!!
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_dirc_tree::brun(jana::JEventLoop* locEventLoop, int locRunNumber)
{
  // This is called whenever the run number changes

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_dirc_tree::evnt(jana::JEventLoop* loop, uint64_t locEventNumber)
{
  // get DIRC geometry
  vector<const DDIRCGeometry*> locDIRCGeometryVec;
  loop->Get(locDIRCGeometryVec);
  // next line commented out to supress warning, variable not used
  //  auto locDIRCGeometry = locDIRCGeometryVec[0];

  // get PID algos
  const DParticleID* locParticleID = NULL;
  loop->GetSingle(locParticleID);

  vector<const DAnalysisResults*> locAnalysisResultsVector;
  loop->Get(locAnalysisResultsVector);

  vector<const DTrackTimeBased*> locTimeBasedTracks;
  loop->Get(locTimeBasedTracks);

  vector<const DDIRCPmtHit*> locDIRCPmtHits;
  loop->Get(locDIRCPmtHits);

  const DDetectorMatches* locDetectorMatches = NULL;
  loop->GetSingle(locDetectorMatches);
  DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];

  // cheat and get truth info of track at bar
  vector<const DDIRCTruthBarHit*> locDIRCBarHits;
  loop->Get(locDIRCBarHits);

  japp->RootWriteLock();
    
  TClonesArray& cevt = *fcEvent;
  cevt.Clear();

  for (size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); loc_i++){
    deque<const DParticleCombo*> locPassedParticleCombos;
    locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);
    const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
    if(locReaction->Get_ReactionName() != "p2pi_dirc_tree" &&
       locReaction->Get_ReactionName() != "p2k_dirc_tree") continue;

    std::vector<double> previousInv;
    
    // loop over combos
    for(size_t icombo = 0; icombo < locPassedParticleCombos.size(); ++icombo){
      double chisq = locPassedParticleCombos[icombo]->Get_KinFitResults()->Get_ChiSq();
      DLorentzVector locMissingP4 = fAnalysisUtilities->Calc_MissingP4(locReaction, locPassedParticleCombos[icombo], false);
      auto locParticleComboStep = locPassedParticleCombos[icombo]->Get_ParticleComboStep(0);
     
      // calculate pi+pi- and K+K- invariant mass by taking the final state and subtracting the proton 
      DLorentzVector locInvP4 = fAnalysisUtilities->Calc_FinalStateP4(locReaction, locPassedParticleCombos[icombo], 0, true);
      for(size_t parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
	auto locParticle = locParticleComboStep->Get_FinalParticle(parti);
        if(locParticle->PID() == Proton) locInvP4 -= locParticle->lorentzMomentum();
      }

      for(size_t parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
	auto locParticle = locParticleComboStep->Get_FinalParticle(parti); // Get_FinalParticle_Measured(parti);
	if(locParticle->charge() == 0) continue;	

	// get track
	auto locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(parti));
	auto locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
     
	// require well reconstructed tracks for initial studies
	int locDCHits = locTrackTimeBased->Ndof + 5;	
	double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
	double locP = locTrackTimeBased->momentum().Mag();
	if(locDCHits < 10 || locTheta < 1.0 || locTheta > 12.0 || locP > 12.0) continue;

	// require has good match to TOF hit for cleaner sample
	shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
	bool foundTOF = locParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	if(!foundTOF || locTOFHitMatchParams->Get_DistanceToTrack() > 20.0) continue;
	double toftrackdist = locTOFHitMatchParams->Get_DistanceToTrack();
	double toftrackdeltat = locChargedTrackHypothesis->Get_TimeAtPOCAToVertex() - locChargedTrackHypothesis->t0();
	Particle_t locPID = locTrackTimeBased->PID();

	// get DIRC match parameters (contains LUT information)
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	bool foundDIRC = locParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);

	if(foundDIRC){
	  DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
	  DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
	  double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
	  int locBar = locDIRCGeometryVec[0]->GetBar(posInBar.Y());

	  fEvent = new DrcEvent();
	  fEvent->SetType(2);
	  fEvent->SetMomentum(TVector3(momInBar.X(),momInBar.Y(),momInBar.Z()));
	  fEvent->SetPdg(PDGtype(locPID));
	  fEvent->SetTime(locExtrapolatedTime);
	  fEvent->SetParent(0);
	  fEvent->SetId(locBar);// bar id where the particle hit the detector
	  fEvent->SetPosition(TVector3(posInBar.X(), posInBar.Y(), posInBar.Z()));
	  fEvent->SetDcHits(locDCHits);
	  fEvent->SetTofTrackDist(toftrackdist);
	  fEvent->SetTofTrackDeltaT(toftrackdeltat);
	  fEvent->SetInvMass(locInvP4.M());
	  fEvent->SetMissMass(locMissingP4.M2());
	  fEvent->SetChiSq(chisq);

	  if(locDIRCBarHits.size() > 0) {

	    TVector3 bestMatchPos, bestMatchMom;
	    double bestFlightTime = 999.;
	    double bestMatchDist = 999.;
	    int bestBar = -1;
	    for(int i=0; i<(int)locDIRCBarHits.size(); i++) {
	      TVector3 locDIRCBarHitPos(locDIRCBarHits[i]->x, locDIRCBarHits[i]->y, locDIRCBarHits[i]->z);
	      TVector3 locDIRCBarHitMom(locDIRCBarHits[i]->px, locDIRCBarHits[i]->py, locDIRCBarHits[i]->pz);
	      if((posInBar - locDIRCBarHitPos).Mag() < bestMatchDist) {
		bestMatchDist = (posInBar - locDIRCBarHitPos).Mag();
		bestMatchPos = locDIRCBarHitPos;
		bestMatchMom = locDIRCBarHitMom;
		bestFlightTime = locDIRCBarHits[i]->t;
		bestBar = locDIRCBarHits[i]->bar;
	      }
	    }

	    fEvent->SetMomentum_Truth(TVector3(bestMatchMom.X(), bestMatchMom.Y(), bestMatchMom.Z()));
	    fEvent->SetPosition_Truth(TVector3(bestMatchPos.X(), bestMatchPos.Y(), bestMatchPos.Z()));
	    fEvent->SetTime_Truth(bestFlightTime);
	    fEvent->SetId_Truth(bestBar);
	  }
	  
	  DrcHit hit;
	  for(const auto dhit : locDIRCPmtHits){
	    int ch=dhit->ch;
	    int pmt=ch/DDIRCGeometry::kPixels;
	    int pix=ch%DDIRCGeometry::kPixels;

	    hit.SetChannel(ch);
	    hit.SetPmtId(pmt);
	    hit.SetPixelId(pix);
	    hit.SetLeadTime(dhit->t);
	    hit.SetTotTime(dhit->tot); 
	    fEvent->AddHit(hit);      
	  }
      
	  if(fEvent->GetHitSize()>0) new (cevt[ cevt.GetEntriesFast()]) DrcEvent(*fEvent);
	}
      }
    }
  }
  
  if(cevt.GetEntriesFast()>0) fTree->Fill();
  japp->RootUnLock();
	
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_dirc_tree::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_dirc_tree::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}


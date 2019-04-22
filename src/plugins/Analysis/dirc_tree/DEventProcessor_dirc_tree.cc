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

  // get DIRC geometry
  vector<const DDIRCGeometry*> locDIRCGeometry;
  locEventLoop->Get(locDIRCGeometry);
  dDIRCGeometry = locDIRCGeometry[0];

  // get PID algos
  const DParticleID* locParticleID = NULL;
  locEventLoop->GetSingle(locParticleID);
  dParticleID = locParticleID;

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_dirc_tree::evnt(jana::JEventLoop* loop, uint64_t locEventNumber)
{
  vector<const DAnalysisResults*> locAnalysisResultsVector;
  loop->Get(locAnalysisResultsVector);

  vector<const DTrackTimeBased*> locTimeBasedTracks;
  loop->Get(locTimeBasedTracks);

  vector<const DDIRCPmtHit*> locDIRCPmtHits;
  loop->Get(locDIRCPmtHits);

  const DDetectorMatches* locDetectorMatches = NULL;
  loop->GetSingle(locDetectorMatches);
  DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];

  japp->RootWriteLock();
    
  TClonesArray& cevt = *fcEvent;
  cevt.Clear();

  for (size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); loc_i++){
    deque<const DParticleCombo*> locPassedParticleCombos;
    locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);
    const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
    std::vector<double> previousInv;
    
    // loop over combos
    for(size_t icombo = 0; icombo < locPassedParticleCombos.size(); ++icombo){
      double chisq = locPassedParticleCombos[icombo]->Get_KinFitResults()->Get_ChiSq();
      DLorentzVector locMissingP4 = fAnalysisUtilities->Calc_MissingP4(locReaction, locPassedParticleCombos[icombo], false);
      auto locParticleComboStep = locPassedParticleCombos[icombo]->Get_ParticleComboStep(0);
     
      // calculate pi+pi- and K+K- invariant mass by taking the final state and subtracting the proton 
      DLorentzVector locInvP4 = fAnalysisUtilities->Calc_FinalStateP4(locReaction, locPassedParticleCombos[icombo], 0, false);
      for(size_t parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
	auto locParticle = locParticleComboStep->Get_FinalParticle(parti);
        if(locParticle->PID() == Proton) locInvP4 -= locParticle->lorentzMomentum();
      }

      int numIt=0;
      for(size_t parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
	auto locParticle = locParticleComboStep->Get_FinalParticle(parti); // Get_FinalParticle_Measured(parti);
	if(locParticle->charge() == 0) continue;	

/*
	// we expect only rho or phi events:  g,p->pi+,pi-,p  g,p->K+,K-,p
	if(locParticle->PID() == PiPlus || locParticle->PID() == PiMinus || locParticle->PID() == KPlus || locParticle->PID() == KMinus){
	  locInvP4 += locParticle->lorentzMomentum();
	  numIt++;
	  if(numIt==2){
	    double tm = locInvP4.M();
	    // do not store doubles
	    if(std::find_if(previousInv.begin(), previousInv.end(), [&tm](double m){return fabs(m-tm)<0.000000001;}) != previousInv.end()) { continue; }
	    previousInv.push_back(tm);
	  }
	}
*/

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
	bool foundTOF = dParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
	if(!foundTOF || locTOFHitMatchParams->Get_DistanceToTrack() > 20.0) continue;
	double toftrackdist = locTOFHitMatchParams->Get_DistanceToTrack();
	Particle_t locPID = locTrackTimeBased->PID();

	// get DIRC match parameters (contains LUT information)
	shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	bool foundDIRC = dParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);

	if(foundDIRC){
	  DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
	  DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
	  double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
	  int locBar = dDIRCGeometry->GetBar(posInBar.Y());

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
	  fEvent->SetInvMass(locInvP4.M());
	  fEvent->SetMissMass(locMissingP4.M2());
	  fEvent->SetChiSq(chisq);

	  DrcHit hit;
	  for(const auto dhit : locDIRCPmtHits){
	    int ch=dhit->ch;
	    int pmt=ch/64;
	    int pix=ch%64;

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


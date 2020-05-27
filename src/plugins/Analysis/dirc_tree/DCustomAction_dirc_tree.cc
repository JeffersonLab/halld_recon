// $Id$
//
//    File: DCustomAction_dirc_tree.cc
//

#include "DCustomAction_dirc_tree.h"


void DCustomAction_dirc_tree::Initialize(JEventLoop* locEventLoop)
{
  DIRC_TRUTH_BARHIT = false;
  if(gPARMS->Exists("DIRC:TRUTH_BARHIT"))
    gPARMS->GetParameter("DIRC:TRUTH_BARHIT",DIRC_TRUTH_BARHIT);

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
   
}

bool DCustomAction_dirc_tree::Perform_Action(JEventLoop* locEventLoop, const DParticleCombo* locParticleCombo)
{

  const DDetectorMatches* locDetectorMatches = NULL;
  locEventLoop->GetSingle(locDetectorMatches);
  DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];
	
  // truth information on tracks hitting DIRC bar (for comparison)
  vector<const DDIRCTruthBarHit*> locDIRCBarHits;
  locEventLoop->Get(locDIRCBarHits);

  vector<const DDIRCPmtHit*> locDIRCPmtHits;
  locEventLoop->Get(locDIRCPmtHits);

  // Get selected particle from reaction for DIRC analysis
  const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(dParticleComboStepIndex);

  TClonesArray& cevt = *fcEvent;
  cevt.Clear();

  for(unsigned int parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
    
    auto locParticle = Get_UseKinFitResultsFlag() ? locParticleComboStep->Get_FinalParticle(parti) : locParticleComboStep->Get_FinalParticle_Measured(parti);
    
    // Get track
    const DChargedTrack* locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(parti));
    const DChargedTrackHypothesis* locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
    const DTrackTimeBased* locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();

    // require well reconstructed tracks for initial studies
    int locDCHits = locTrackTimeBased->Ndof + 5;
    double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
    double locP = locParticle->lorentzMomentum().Vect().Mag();

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

    if(foundDIRC){

      DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
      DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
      double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
      int locBar = dDIRCGeometry->GetBar(posInBar.Y());

      fEvent = new DrcEvent();
      fEvent->SetType(0);
      fEvent->SetMomentum(TVector3(momInBar.X(),momInBar.Y(),momInBar.Z()));
      fEvent->SetPdg(PDGtype(locPID));
      fEvent->SetTime(locExtrapolatedTime);
      fEvent->SetParent(0);
      fEvent->SetId(locBar);// bar id where the particle hit the detector
      fEvent->SetPosition(TVector3(posInBar.X(), posInBar.Y(), posInBar.Z()));
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
  
  japp->RootWriteLock();
  if(cevt.GetEntriesFast()>0) fTree->Fill();
  japp->RootUnLock();

  return true;
}

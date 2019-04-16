// -----------------------------------------
// DEventProcessor_pid_dirc.cc
// created on: 07.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#include "DEventProcessor_pid_dirc.h"
#include "TCanvas.h"
#include "TH1.h"


// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_pid_dirc());
    app->AddFactoryGenerator(new JFactoryGenerator_dirc_tree()); //register the factory generator
  }
}

DEventProcessor_pid_dirc::DEventProcessor_pid_dirc() {
  fTree = NULL;
  fEvent = NULL;
}

DEventProcessor_pid_dirc::~DEventProcessor_pid_dirc() {}

jerror_t DEventProcessor_pid_dirc::init(void) {

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

jerror_t DEventProcessor_pid_dirc::brun(jana::JEventLoop *loop, int32_t runnumber)
{
//////////////////////////////////////////////////////////////////////////////
// dapp and geom are not used but without it dirc_hits.so will no be loaded
//root [0] gSystem->Load("dirc_hits.so");
//cling::DynamicLibraryManager::loadLibrary(): dirc_hits.so: undefined symbol: _ZTV25DEventSourceRESTGenerator
   // Get the geometry
   DApplication* dapp=dynamic_cast<DApplication*>(loop->GetJApplication());
   DGeometry *geom = dapp->GetDGeometry(runnumber);
////////////////////////////////////////////////////////////////////////////


  // get DIRC geometry
  vector<const DDIRCGeometry*> locDIRCGeometry;
  loop->Get(locDIRCGeometry);
  dDIRCGeometry = locDIRCGeometry[0];

  // get PID algos
  const DParticleID* locParticleID = NULL;
  loop->GetSingle(locParticleID);
  dParticleID = locParticleID;


   return NOERROR;
}
//------------------
// evnt
//------------------



jerror_t DEventProcessor_pid_dirc::evnt(JEventLoop *loop, uint64_t eventnumber) {

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

  double chisq(-1);

  for (unsigned int loc_i = 0; loc_i < locAnalysisResultsVector.size(); loc_i++){
    deque<const DParticleCombo*> locPassedParticleCombos;
    locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);


for(size_t j = 0; j < locPassedParticleCombos.size(); ++j){
    // chisq
    chisq = locPassedParticleCombos[j]->Get_KinFitResults ()->Get_ChiSq();
    // calculate 2pi P4
    DLorentzVector locP4_2pi, locP4_2k;
    double chisq_pi(-1), chisq_k(-1);

    auto locParticleComboStep = locPassedParticleCombos[j]->Get_ParticleComboStep(0);
    const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
    auto locActions = locReaction->Get_AnalysisActions();

    DLorentzVector locMissingP4 = dAnalysisUtilities->Calc_MissingP4(locReaction, locPassedParticleCombos[j] , false);


    for(unsigned int parti=0; parti<locParticleComboStep->Get_NumFinalParticles(); parti++){
      auto locParticle = locParticleComboStep->Get_FinalParticle(parti);

      if(locParticle->PID() == PiPlus || locParticle->PID() == PiMinus){
         locP4_2pi += locParticle->lorentzMomentum();
        chisq_pi = chisq;
        }
      if(locParticle->PID() == KPlus || locParticle->PID() == KMinus){
         locP4_2k += locParticle->lorentzMomentum();
        chisq_k = chisq;

        }
      //if(locParticle->PID() == PiPlus || locParticle->PID() == PiMinus)cout<<" ##########   ##########   ####### Iam in EventProcessor print Rho "<< locParticle->PID()<< endl;
      //if(locParticle->PID() == KPlus || locParticle->PID() == KMinus)cout<<" ##########   ##########   ####### Iam in EventProcessor print Phi "<< locParticle->PID()<< endl;
      // Get track
      auto locChargedTrack = static_cast<const DChargedTrack*>(locParticleComboStep->Get_FinalParticle_SourceObject(parti));
      auto locChargedTrackHypothesis = locChargedTrack->Get_Hypothesis(locParticle->PID());
      auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();

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

      if(foundDIRC){

        DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
        DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
        double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
        int locBar = dDIRCGeometry->GetBar(posInBar.Y());

        fEvent = new DrcEvent();

        fEvent->SetPhiMass(locP4_2k.M());
        fEvent->SetRhoMass(locP4_2pi.M());
        fEvent->SetMissingMass(locMissingP4.E());
        fEvent->SetChiSq_k(chisq_k);
        fEvent->SetChiSq_pi(chisq_pi);

        fEvent->SetType(2);
        fEvent->SetMomentum(TVector3(momInBar.X(),momInBar.Y(),momInBar.Z()));
        fEvent->SetPdg(PDGtype(locPID));
        fEvent->SetTime(locExtrapolatedTime);
        fEvent->SetParent(0);
        fEvent->SetId(locBar);// bar id where the particle hit the detector
        fEvent->SetPosition(TVector3(posInBar.X(), posInBar.Y(), posInBar.Z()));

        //cout<<" ##########   ##########   ####### Iam in EventProcessor print PID  "<< locParticle->PID()<<" RhoMass "<<locP4_2pi.M()<< " PhiMass "<<locP4_2k.M()<<" MissingMass "<< locMissingP4.E()<<endl;

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
jerror_t DEventProcessor_pid_dirc::erun(void) {
  return NOERROR;
}
//------------------
// fini
//------------------
jerror_t DEventProcessor_pid_dirc::fini(void) {
  // TCanvas *c = new TCanvas("c","c",800,500);
  // hfine->Draw();
  // c->Modified();
  // c->Update();
  // c->Print("htime.png");
  return NOERROR;
}

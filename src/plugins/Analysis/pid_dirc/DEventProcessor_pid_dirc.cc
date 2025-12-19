// -----------------------------------------
// DEventProcessor_pid_dirc.cc
// created on: 07.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#include "DEventProcessor_pid_dirc.h"
#include "TCanvas.h"
#include "TH1.h"

#include <DANA/DEvent.h>


// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new DEventProcessor_pid_dirc());
  }
}

DEventProcessor_pid_dirc::DEventProcessor_pid_dirc() {
  fTree = NULL;
  fEvent = NULL;
}

DEventProcessor_pid_dirc::~DEventProcessor_pid_dirc() {}

void DEventProcessor_pid_dirc::Init() {
  string locOutputFileName = "hd_root.root";
  auto app = GetApplication();
  if(app->GetJParameterManager()->Exists("OUTPUT_FILENAME"))
    app->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->Cd("/");
  
  fTree = new TTree("dirc", "dirc tree");
  fcEvent = new TClonesArray("DrcEvent");
  fTree->Branch("DrcEvent",&fcEvent,256000,1);
}

void DEventProcessor_pid_dirc::BeginRun(const std::shared_ptr<const JEvent>& event)
{
   // Get the geometry
   DGeometry *geom = DEvent::GetDGeometry(event);

   // Outer detector geometry parameters
   vector<double>tof_face;
   geom->Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z", tof_face);
   vector<double>tof_plane;  
   geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='0']", tof_plane);
   double dTOFz=tof_face[2]+tof_plane[2]; 
   geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='1']", tof_plane);
   dTOFz+=tof_face[2]+tof_plane[2];
   dTOFz*=0.5;  // mid plane between tof Planes

   double dDIRCz;
   vector<double>dirc_face;
   vector<double>dirc_plane;
   vector<double>dirc_shift;
   vector<double>bar_plane;
   geom->Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z", dirc_face);
   geom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML10']/@X_Y_Z/plane[@value='1']", dirc_plane);
   geom->Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", dirc_shift);
   
   dDIRCz=dirc_face[2]+dirc_plane[2]+dirc_shift[2] + 0.8625; // last shift is the average center of quartz bar (585.862)
   std::cout<<"dDIRCz "<<dDIRCz<<std::endl;
}

//TH1F *hfine = new TH1F("hfine","hfine",200,1600,1800);

void DEventProcessor_pid_dirc::Process(const std::shared_ptr<const JEvent>& event) {

  vector<const DMCThrown*> mcthrowns;
  vector<const DMCTrackHit*> mctrackhits;
  vector<const DDIRCTruthBarHit*> dircBarHits;
  vector<const DDIRCTruthPmtHit*> dircPmtHits;
  vector<const DDIRCTDCDigiHit*> dataDigiHits;
  vector<const DDIRCPmtHit*> dataPmtHits;
  vector<const DL1Trigger*> trig;
  
  event->Get(mcthrowns);
  event->Get(mctrackhits);
  event->Get(dircPmtHits);
  event->Get(dircBarHits);
  event->Get(dataDigiHits);
  event->Get(dataPmtHits);
  event->Get(trig);

  // get PID algos
  const DParticleID* locParticleID = NULL;
  event->GetSingle(locParticleID);

  vector<const DDIRCGeometry*> locDIRCGeometryVec;
  event->Get(locDIRCGeometryVec);
  auto locDIRCGeometry = locDIRCGeometryVec[0];

  // Initialize DIRC LUT
  const DDIRCLut* dDIRCLut = nullptr;
  event->GetSingle(dDIRCLut);

  // retrieve tracks and detector matches
  vector<const DTrackTimeBased*> locTimeBasedTracks;
  event->Get(locTimeBasedTracks);
  
  const DDetectorMatches* locDetectorMatches = NULL;
  event->GetSingle(locDetectorMatches);
  DDetectorMatches locDetectorMatch = (DDetectorMatches)locDetectorMatches[0];
  
  DEvent::GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK

  // check for LED triggers
  int trigger = 0;
  if (trig.size() > 0) {
    // LED appears as "bit" 15 in L1 front panel trigger monitoring plots
    if (trig[0]->fp_trig_mask & 0x4000) trigger = 1;
    // Physics trigger appears as "bit" 1 in L1 trigger monitoring plots
    if (trig[0]->trig_mask & 0x1) trigger = 2;    
  }


  // LED specific information
  double locLEDRefTime = 0;
  // double locLEDRefAdcTime = 0;
  // double locLEDRefTdcTime = 0;
  if(trigger==1) {
    vector<const DDIRCLEDRef*> dircLEDRefs;
    event->Get(dircLEDRefs);
    for(uint i=0; i<dircLEDRefs.size(); i++) {
      const DDIRCLEDRef* dircLEDRef = (DDIRCLEDRef*)dircLEDRefs[i];
      // locLEDRefAdcTime = dircLEDRef->t_fADC;
      // locLEDRefTdcTime = dircLEDRef->t_TDC;
      locLEDRefTime = dircLEDRef->t_TDC;
      break;
    }
  }
  
  // loop over mc/reco tracks
  TClonesArray& cevt = *fcEvent;
  cevt.Clear();
  int locThrownPDGID = 0;
  for (unsigned int m = 0; m < mcthrowns.size(); m++){
    if(m==0) locThrownPDGID = mcthrowns[m]->pdgtype;
    if(dircPmtHits.size() > 0.){
      fEvent = new DrcEvent();
      fEvent->SetType(trigger);
      DrcHit hit;
      
      // loop over PMT's hits
      for (unsigned int h = 0; h < dircPmtHits.size(); h++){
	int relevant(0);
	// identify bar id
	for (unsigned int j = 0; j < dircBarHits.size(); j++){
		//if(j != fabs(dircPmtHits[h]->key_bar)) continue;
	  if(mcthrowns[m]->myid == dircBarHits[j]->track){
	    // double px = mcthrowns[m]->momentum().X();
	    // double py = mcthrowns[m]->momentum().Y();
	    // double pz = mcthrowns[m]->momentum().Z();
	    
	    double px = dircBarHits[j]->px;
	    double py = dircBarHits[j]->py;
	    double pz = dircBarHits[j]->pz;

	    fEvent->SetMomentum(TVector3(px,py,pz));
	    fEvent->SetPdg(mcthrowns[m]->pdgtype);
	    fEvent->SetTime(dircBarHits[j]->t);
	    fEvent->SetParent(mcthrowns[m]->parentid);
	    fEvent->SetId(dircBarHits[j]->bar);// bar id where the particle hit the detector
	    fEvent->SetPosition(TVector3(dircBarHits[j]->x, dircBarHits[j]->y, dircBarHits[j]->z)); // position where the charged particle hit the radiator
	    relevant++;
	  }
	}	
	
	if(relevant<1) continue;
	int ch=dircPmtHits[h]->ch;
	int pmt=ch/64;
	int pix=ch%64;
	
	hit.SetChannel(dircPmtHits[h]->ch);
	hit.SetPmtId(pmt);
	hit.SetPixelId(pix);
	hit.SetPosition(TVector3(dircPmtHits[h]->x,dircPmtHits[h]->y,dircPmtHits[h]->z));
	hit.SetEnergy(dircPmtHits[h]->E);
	hit.SetLeadTime(dircPmtHits[h]->t);
	hit.SetPathId(dircPmtHits[h]->path);
	hit.SetNreflections(dircPmtHits[h]->refl);
	fEvent->AddHit(hit);
      }
      
      if(fEvent->GetHitSize()>0) new (cevt[ cevt.GetEntriesFast()]) DrcEvent(*fEvent);      
    }
  }

#if 0
  // reconstruct DIRC LUT variables for specific tracks  
  for (unsigned int loc_i = 0; loc_i < locTimeBasedTracks.size(); loc_i++){

    const DTrackTimeBased* locTrackTimeBased = locTimeBasedTracks[loc_i];
    Particle_t locPID = locTrackTimeBased->PID();
    if(PDGtype(locPID) != locThrownPDGID) continue;
    
    // require well reconstructed tracks for initial studies
    int locDCHits = locTrackTimeBased->Ndof + 5;
    double locTheta = locTrackTimeBased->momentum().Theta()*180/TMath::Pi();
    double locP = locTrackTimeBased->momentum().Mag();
    if(locDCHits < 10 || locTheta < 1.0 || locTheta > 12.0 || locP > 12.0)
      continue;
    
    // require has good match to TOF hit for cleaner sample
    shared_ptr<const DTOFHitMatchParams> locTOFHitMatchParams;
    bool foundTOF = locParticleID->Get_BestTOFMatchParams(locTrackTimeBased, locDetectorMatches, locTOFHitMatchParams);
    if(!foundTOF || locTOFHitMatchParams->dDeltaXToHit > 10.0 || locTOFHitMatchParams->dDeltaYToHit > 10.0)
      continue;
    double toftrackdist = locTOFHitMatchParams->Get_DistanceToTrack();
    double toftrackdeltat = 0.; //locChargedTrackHypothesis->Get_TimeAtPOCAToVertex() - locTrackTimeBased->t0();
    
    double locMass = ParticleMass(locPID);
    
    // get DIRC match parameters (contains LUT information)
    shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
    bool foundDIRC = locParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
    
    if(foundDIRC) {
	    
      TVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos;
      TVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
      double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
      int locBar = locDIRCGeometry->GetBar(posInBar.Y());
      
      fEvent = new DrcEvent();
      fEvent->SetType(0);
      fEvent->SetMomentum(TVector3(momInBar.X(),momInBar.Y(),momInBar.Z()));
      fEvent->SetPdg(PDGtype(locPID));
      fEvent->SetTime(locExtrapolatedTime);
      fEvent->SetParent(0);
      fEvent->SetId(locBar);// bar id where the particle hit the detector
      fEvent->SetPosition(TVector3(posInBar.X(), posInBar.Y(), posInBar.Z()));
      fEvent->SetNPhotons(locDIRCMatchParams->dNPhotons);
      fEvent->SetLikelihoodElectron(locDIRCMatchParams->dLikelihoodElectron);
      fEvent->SetLikelihoodPion(locDIRCMatchParams->dLikelihoodPion);
      fEvent->SetLikelihoodKaon(locDIRCMatchParams->dLikelihoodKaon);
      fEvent->SetLikelihoodProton(locDIRCMatchParams->dLikelihoodProton);
      fEvent->SetDcHits(locDCHits);
      fEvent->SetTofTrackDist(toftrackdist);
      fEvent->SetTofTrackDeltaT(toftrackdeltat);
      
      DrcHit hit;
      for(const auto dhit : dataPmtHits){
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
#endif
  
/*
  // calibrated hists
  if(dataPmtHits.size()>0){
    fEvent = new DrcEvent();
    fEvent->SetType(trigger);
    fEvent->SetTime(locLEDRefTime);
	  
    DrcHit hit;
    for (const auto dhit : dataPmtHits) {
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

    // for (const auto dhit : dataDigiHits) {
    //   int ch=dhit->channel;
    //   int pmt=ch/64;
    //   int pix=ch%64;
	
    //   hit.SetChannel(ch);
    //   hit.SetPmtId(pmt);
    //   hit.SetPixelId(pix);
    //   hit.SetLeadTime(dhit->time);
    //   hit.SetTotTime(dhit->edge);
    //   fEvent->AddHit(hit);      
    // }
    
    
    if(fEvent->GetHitSize()>0) new (cevt[ cevt.GetEntriesFast()]) DrcEvent(*fEvent);
  }
*/
  
  if(cevt.GetEntriesFast()>0) fTree->Fill();
  DEvent::GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK
}

void DEventProcessor_pid_dirc::EndRun() {
}

void DEventProcessor_pid_dirc::Finish() {
  // TCanvas *c = new TCanvas("c","c",800,500);
  // hfine->Draw();
  // c->Modified();
  // c->Update();
  // c->Print("htime.png");
}

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
  }
}

DEventProcessor_pid_dirc::DEventProcessor_pid_dirc() {
  fTree = NULL;
  fEvent = NULL;
}

DEventProcessor_pid_dirc::~DEventProcessor_pid_dirc() {}

jerror_t DEventProcessor_pid_dirc::init(void) {
  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
    gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->Cd("/");
  
  fTree = new TTree("dirc", "dirc tree");
  fcEvent = new TClonesArray("DrcEvent");
  fTree->Branch("DrcEvent",&fcEvent,256000,1);
  return NOERROR;
}

jerror_t DEventProcessor_pid_dirc::brun(jana::JEventLoop *loop, int32_t runnumber)
{
   // Get the geometry
   DApplication* dapp=dynamic_cast<DApplication*>(loop->GetJApplication());
   DGeometry *geom = dapp->GetDGeometry(runnumber);

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
   
   return NOERROR;
}

//TH1F *hfine = new TH1F("hfine","hfine",200,1600,1800);

jerror_t DEventProcessor_pid_dirc::evnt(JEventLoop *loop, uint64_t eventnumber) {

  vector<const DMCThrown*> mcthrowns;
  vector<const DMCTrackHit*> mctrackhits;
  vector<const DDIRCTruthBarHit*> dircBarHits;
  vector<const DDIRCTruthPmtHit*> dircPmtHits;
  vector<const DDIRCTDCDigiHit*> dataDigiHits;
  vector<const DDIRCPmtHit*> dataPmtHits;
  vector<const DL1Trigger*> trig;
  
  loop->Get(mcthrowns);
  loop->Get(mctrackhits);
  loop->Get(dircPmtHits);
  loop->Get(dircBarHits);
  loop->Get(dataDigiHits);
  loop->Get(dataPmtHits);
  loop->Get(trig);
  
  japp->RootWriteLock(); //ACQUIRE ROOT LOCK

  int trigger = 0;
  if(trig.size() > 0){
    // LED appears as "bit" 15 in L1 front panel trigger monitoring plots
    if (trig[0]->fp_trig_mask & 0x4000) trigger=15;  
  }

  // // get LED trigger
  // double locLEDRefTime = 0;
  // if(true){
  //   // Get LED SiPM reference
  //   vector<const DCAEN1290TDCHit*> sipmtdchits;
  //   vector<const Df250PulseData*> sipmadchits;

  //   loop->Get(sipmtdchits);
  //   loop->Get(sipmadchits);

  //   for(uint i=0; i<sipmadchits.size(); i++) {
  //     const Df250PulseData* sipmadchit = (Df250PulseData*)sipmadchits[i];
  //     if(sipmadchit->rocid == 77 && sipmadchit->slot == 16 && sipmadchit->channel == 15) {
  // 	locLEDRefTime = ((sipmadchit->course_time<<6) + sipmadchit->fine_time) * 0.0625; // convert time from flash to ns
  // 	//hfine->Fill(sipmadchit->course_time<<6);
  //     }
  //   }
  // }

  // loop over mc/reco tracks
  TClonesArray& cevt = *fcEvent;
  cevt.Clear();
  for (unsigned int m = 0; m < mcthrowns.size(); m++){
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
  
  // calibrated hists
  //if(dataDigiHits.size()>0){
  if(dataPmtHits.size()>0){
    fEvent = new DrcEvent();
    fEvent->SetType(trigger);
	  
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
  
  if(cevt.GetEntriesFast()>0) fTree->Fill();
  japp->RootUnLock(); //RELEASE ROOT LOCK
      
  return NOERROR;
}

jerror_t DEventProcessor_pid_dirc::erun(void) {
  return NOERROR;
}

jerror_t DEventProcessor_pid_dirc::fini(void) {
  // TCanvas *c = new TCanvas("c","c",800,500);
  // hfine->Draw();
  // c->Modified();
  // c->Update();
  // c->Print("htime.png");
  return NOERROR;
}

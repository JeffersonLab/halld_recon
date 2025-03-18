// -----------------------------------------
// DEventProcessor_lut_dirc.cc
// created on: 29.11.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#include <DANA/DEvent.h>
#include "DEventProcessor_lut_dirc.h"

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new DEventProcessor_lut_dirc());
  }
}

DEventProcessor_lut_dirc::DEventProcessor_lut_dirc() {
  fTree = NULL;
}

DEventProcessor_lut_dirc::~DEventProcessor_lut_dirc() {
}

void DEventProcessor_lut_dirc::Init() {
  string locOutputFileName = "hd_root.root";
  auto params = GetApplication()->GetJParameterManager();
  if(params->Exists("OUTPUT_FILENAME"))
    params->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->Cd("/");

  fTree = new TTree("lut_dirc","Look-up table for the geometrical reconstruction.");
  int Nnodes = 30000;    
  for(int l=0; l<48; l++){
    fLut[l] = new TClonesArray("DrcLutNode");
    fTree->Branch(Form("LUT_%d",l),&fLut[l],256000,-1);
    TClonesArray &fLuta = *fLut[l];
    for (Long64_t n=0; n<Nnodes; n++) {
      new((fLuta)[n]) DrcLutNode(-1);
    }
  }
}

void DEventProcessor_lut_dirc::Process(const std::shared_ptr<const JEvent>& event) {
  vector<const DMCThrown*> mcthrowns;
  vector<const DDIRCTruthPmtHit*> dircPmtHits;
  
  event->Get(mcthrowns);
  event->Get(dircPmtHits);

  if(mcthrowns.size()<1) return;
  if(dircPmtHits.size()!=1) return;
  
  GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK
  
  // loop over PMT's hits
  for (unsigned int h = 0; h < dircPmtHits.size(); h++){
    
    int ch=dircPmtHits[h]->ch;
    int lutId = dircPmtHits[h]->key_bar;
    TVector3 dir =  TVector3(mcthrowns[0]->momentum().X(),
			     mcthrowns[0]->momentum().Y(),
			     mcthrowns[0]->momentum().Z()).Unit();

    //std::cout<<"dir.X() "<<dir.X() <<" "<<dir.Y() <<" "<<dir.Z() << " | "
    //	     <<mom.X() <<" "<<mom.Y() <<" "<<mom.Z() <<std::endl;
        
    if(lutId>=0 && lutId<48)
      ((DrcLutNode*)(fLut[lutId]->At(ch)))->
	AddEntry(lutId,               // lut/bar id
		 ch,                  // pixel id
		 dir,
		 dircPmtHits[h]->path,
		 dircPmtHits[h]->refl,
		 dircPmtHits[h]->t,
		 TVector3(dircPmtHits[h]->x,dircPmtHits[h]->y,dircPmtHits[h]->z),
		 TVector3(dircPmtHits[h]->x,dircPmtHits[h]->y,dircPmtHits[h]->z));
  }
  GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK
}

void DEventProcessor_lut_dirc::EndRun() {
}

void DEventProcessor_lut_dirc::Finish() {
  auto lockSvc = GetApplication()->GetService<JLockService>();
  lockSvc->RootWriteLock();
  fTree->Fill();
  lockSvc->RootUnLock();
}

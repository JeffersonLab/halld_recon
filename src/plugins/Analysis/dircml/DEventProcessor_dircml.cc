// -----------------------------------------
// DEventProcessor_dircml.cc
// -----------------------------------------

#include "DEventProcessor_dircml.h"

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_dircml());
  }
}

DEventProcessor_dircml::DEventProcessor_dircml() {
}

DEventProcessor_dircml::~DEventProcessor_dircml() {
}

jerror_t DEventProcessor_dircml::init(void) {
  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
    gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->cd("/");

  TObject* locTree = gDirectory->Get("dirc");
  if(locTree == NULL)
    dircmlTree = new TTree("dircml_flat", "Flat Tree for DIRC ML");
  else
    dircmlTree = static_cast<TTree*>(locTree);

  dircmlTree->Branch("NPixels",&NPixels,"NPixels/I");
  dircmlTree->Branch("PixelRow",PixelRow,"PixelRow[NPixels]/I");
  dircmlTree->Branch("PixelCol",PixelCol,"PixelCol[NPixels]/I");
  dircmlTree->Branch("PixelTime",PixelTime,"PixelTime[NPixels]/D");
 
  return NOERROR;
}

jerror_t DEventProcessor_dircml::brun(jana::JEventLoop *loop, int32_t runnumber)
{


  return NOERROR;
}

jerror_t DEventProcessor_dircml::evnt(JEventLoop *loop, uint64_t eventnumber) {
  vector<const DBeamPhoton*> beam_photons;
  vector<const DMCThrown*> mcthrowns;
  vector<const DMCTrackHit*> mctrackhits;
  vector<const DDIRCTruthBarHit*> dircBarHits;
  vector<const DDIRCTruthPmtHit*> dircPmtHits;
  vector<const DDIRCPmtHit*> dircRecoPmtHits;
  
  loop->Get(beam_photons);
  loop->Get(mcthrowns);
  loop->Get(mctrackhits);
  loop->Get(dircPmtHits);
  loop->Get(dircBarHits);
  loop->Get(dircRecoPmtHits);

  // get DIRC geometry
  vector<const DDIRCGeometry*> locDIRCGeometryVec;
  loop->Get(locDIRCGeometryVec);
  auto locDIRCGeometry = locDIRCGeometryVec[0];

  // reset hits for TTree
  NPixels = 0;
  for(int i=0; i<1000; i++) {
	  PixelRow[i] = 0;
	  PixelCol[i] = 0; 
	  PixelTime[i] = 0.;
  }
  
  // fill TTree for event
  for (unsigned int h = 0; h < dircPmtHits.size(); h++){
	
     int ch=dircPmtHits[h]->ch;
     double t = dircPmtHits[h]->t;
     
     // get pixel labels
     int pixel_row = locDIRCGeometry->GetPixelRow(ch);
     int pixel_col = locDIRCGeometry->GetPixelColumn(ch);

     PixelRow[NPixels] = pixel_row;
     PixelCol[NPixels] = pixel_col;
     PixelTime[NPixels] = t;
     NPixels++;
  }
  dircmlTree->Fill();
  
  return NOERROR;
}

jerror_t DEventProcessor_dircml::erun(void) {
  return NOERROR;
}

jerror_t DEventProcessor_dircml::fini(void) {
  return NOERROR;
}

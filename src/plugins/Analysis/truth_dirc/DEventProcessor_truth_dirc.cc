// -----------------------------------------
// DEventProcessor_truth_dirc.cc
// -----------------------------------------

#include "DEventProcessor_truth_dirc.h"

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_truth_dirc());
  }
}

DEventProcessor_truth_dirc::DEventProcessor_truth_dirc() {
}

DEventProcessor_truth_dirc::~DEventProcessor_truth_dirc() {
}

jerror_t DEventProcessor_truth_dirc::init(void) {
  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
    gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->cd("/");

  TDirectory *mainDir = gDirectory;
  TDirectory *dircDir = gDirectory->mkdir("DIRC_truth");
  dircDir->cd();

  int nChannels = 108*64;
  hTruthPixelHitTime = new TH2F("hTruthPixelHitTime", "; Pixel Channel # ; #Delta t (ns)", nChannels, 0, nChannels, 200, -100, 100);

  hTruthBarHitXY = new TH2F("hTruthBarHitXY", "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);
  hTruthBarHitBar = new TH1F("hTruthBarHitBar", "; Bar #", 48, 0.5, 47.5);
  hTruthPmtHitZY_North = new TH2F("hTruthPmtHitZY_North", "North Box; PMT Hit Z (cm); PMT Hit Y (cm)", 100, 525, 560, 110, 0., 110.);
  hTruthPmtHitZY_South = new TH2F("hTruthPmtHitZY_South", "South Box; PMT Hit Z (cm); PMT Hit Y (cm)", 100, 525, 560, 110, -110., 0.);
 
  hTruthPmtHit_North = new TH2F("hTruthPmtHit_North", "North Box; Pmt Hit Column ; Pixel Hit Row", 6, 0, 6, 18, 0, 18);
  hTruthPmtHit_South = new TH2F("hTruthPmtHit_South", "South Box; Pmt Hit Column ; Pixel Hit Row", 6, 0, 6, 18, 0, 18);
  hTruthPixelHit_North = new TH2F("hTruthPixelHit_North", "North Box; Pixel Hit X ; Pixel Hit Y", 144, 0, 144, 48, 0, 48);
  hTruthPixelHit_South = new TH2F("hTruthPixelHit_South", "South Box; Pixel Hit X ; Pixel Hit Y", 144, 0, 144, 48, 0, 48);

  hPixelHit_North = new TH2F("hPixelHit_North", "North Box; Pixel Hit X ; Pixel Hit Y", 144, 0, 144, 48, 0, 48);
  hPixelHit_South = new TH2F("hPixelHit_South", "South Box; Pixel Hit X ; Pixel Hit Y", 144, 0, 144, 48, 0, 48);
  mainDir->cd();
 
  return NOERROR;
}

jerror_t DEventProcessor_truth_dirc::brun(jana::JEventLoop *loop, int32_t runnumber)
{


  return NOERROR;
}

jerror_t DEventProcessor_truth_dirc::evnt(JEventLoop *loop, uint64_t eventnumber) {
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

  for (unsigned int j = 0; j < dircBarHits.size(); j++){
    //double px = dircBarHits[j]->px;
    //double py = dircBarHits[j]->py;
    //double pz = dircBarHits[j]->pz;

    double x = dircBarHits[j]->x;
    double y = dircBarHits[j]->y;
    int bar = dircBarHits[j]->bar;

    japp->RootWriteLock(); //ACQUIRE ROOT LOCK
    hTruthBarHitXY->Fill(x, y);
    hTruthBarHitBar->Fill(bar);
    japp->RootUnLock();
  }

  for (unsigned int h = 0; h < dircPmtHits.size(); h++){
	
     int ch=dircPmtHits[h]->ch;
    
     double x = dircPmtHits[h]->x;
     double y = dircPmtHits[h]->y;
     double z = dircPmtHits[h]->z;
     double t = dircPmtHits[h]->t;
     double t_fixed = dircPmtHits[h]->t_fixed;

     // get PMT labels
     int pmt_column = locDIRCGeometry->GetPmtColumn(ch); 
     int pmt_row = locDIRCGeometry->GetPmtRow(ch);

     // get pixel labels
     int pixel_row = locDIRCGeometry->GetPixelRow(ch);
     int pixel_col = locDIRCGeometry->GetPixelColumn(ch);

     japp->RootWriteLock(); //ACQUIRE ROOT LOCK
     hTruthPixelHitTime->Fill(ch, t-t_fixed);
     if(x < 0.) {
	hTruthPmtHitZY_South->Fill(z, y);
	hTruthPmtHit_South->Fill(pmt_column, pmt_row);
	hTruthPixelHit_South->Fill(pixel_x, pixel_y);
     }
     else {
	hTruthPmtHitZY_North->Fill(z, y);
	hTruthPmtHit_North->Fill(pmt_column, pmt_row);
	hTruthPixelHit_North->Fill(pixel_x, pixel_y);
     }
     japp->RootUnLock();
  }

  for (unsigned int h = 0; h < dircRecoPmtHits.size(); h++){
	  
	  int ch=dircRecoPmtHits[h]->ch;
	  //double t = dircRecoPmtHits[h]->t;
	  
	  // get pixel labels
	  int pixel_row = locDIRCGeometry->GetPixelRow(ch);
	  int pixel_col = locDIRCGeometry->GetPixelColumn(ch);
	  
	  // comparison of truth and reco hits
	  /*
	  vector<const DDIRCTruthPmtHit*> dircTruthPmtHits;  
	  dircRecoPmtHits[h]->Get(dircTruthPmtHits);
	  if(dircTruthPmtHits.empty())
		  cout<<"didn't find associated truth hit"<<endl;
	  else if(fabs(t - dircTruthPmtHits[0]->t_fixed) > 1.0)
		  cout<<"found associated truth hit with large DeltaT = "<<t - dircTruthPmtHits[0]->t_fixed<<endl;
	  */

	  japp->RootWriteLock(); //ACQUIRE ROOT LOCK
	  if(ch < 108*64) {
		  hPixelHit_South->Fill(pixel_x, pixel_y);
	  }
	  else {
		  hPixelHit_North->Fill(pixel_x, pixel_y);
	  }
	  japp->RootUnLock();
  }
  
  return NOERROR;
}

jerror_t DEventProcessor_truth_dirc::erun(void) {
  return NOERROR;
}

jerror_t DEventProcessor_truth_dirc::fini(void) {
  return NOERROR;
}

// -----------------------------------------
// DEventProcessor_truth_dirc.cc
// -----------------------------------------

#include "DEventProcessor_truth_dirc.h"
#include <DANA/DEvent.h>

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new DEventProcessor_truth_dirc());
  }
}

DEventProcessor_truth_dirc::DEventProcessor_truth_dirc() {
}

DEventProcessor_truth_dirc::~DEventProcessor_truth_dirc() {
}

void DEventProcessor_truth_dirc::Init() {
  string locOutputFileName = "hd_root.root";
  auto params = GetApplication()->GetJParameterManager();
  if(params->Exists("OUTPUT_FILENAME"))
    params->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  //go to file
  TFile* locFile = (TFile*)gROOT->FindObject(locOutputFileName.c_str());
  if(locFile != NULL)
    locFile->cd("");
  else
    gDirectory->cd("/");

  TDirectory *mainDir = gDirectory;
  TDirectory *dircDir = gDirectory->mkdir("DIRC_truth");
  dircDir->cd();

  hTruthWavelength = new TH1F("hTruthWavelength", "; Wavelength (nm)", 300, 200, 800);

  int nChannels = 108*64;
  hTruthPixelHitTime = new TH2F("hTruthPixelHitTime", "; Pixel Channel # ; #Delta t (ns)", nChannels, 0, nChannels, 200, -100, 100);

  hTruthBarHitXY = new TH2F("hTruthBarHitXY", "; Bar Hit X (cm); Bar Hit Y (cm)", 200, -100, 100, 200, -100, 100);
  hTruthBarHitBar = new TH1F("hTruthBarHitBar", "; Bar #", 48, 0.5, 47.5);
  hTruthPmtHitZY_North = new TH2F("hTruthPmtHitZY_North", "North Box; PMT Hit Z (cm); PMT Hit Y (cm)", 100, 525, 560, 110, 0., 110.);
  hTruthPmtHitZY_South = new TH2F("hTruthPmtHitZY_South", "South Box; PMT Hit Z (cm); PMT Hit Y (cm)", 100, 525, 560, 110, -110., 0.);
 
  hTruthPmtHit_North = new TH2F("hTruthPmtHit_North", "North Box; Pmt Hit Column ; Pixel Hit Row", 6, 0, 6, 18, 0, 18);
  hTruthPmtHit_South = new TH2F("hTruthPmtHit_South", "South Box; Pmt Hit Column ; Pixel Hit Row", 6, 0, 6, 18, 0, 18);
  hTruthPixelHit_North = new TH2F("hTruthPixelHit_North", "North Box; Pixel Hit X ; Pixel Hit Y", 144, -0.5, 143.5, 48, -0.5, 47.5);
  hTruthPixelHit_South = new TH2F("hTruthPixelHit_South", "South Box; Pixel Hit X ; Pixel Hit Y", 144, -0.5, 143.5, 48, -0.5, 47.5);

  hPixelHit_North = new TH2F("hPixelHit_North", "North Box; Pixel Hit X ; Pixel Hit Y", 144, -0.5, 143.5, 48, -0.5, 47.5);
  hPixelHit_South = new TH2F("hPixelHit_South", "South Box; Pixel Hit X ; Pixel Hit Y", 144, -0.5, 143.5, 48, -0.5, 47.5);
  mainDir->cd();
 
  return;
}

void DEventProcessor_truth_dirc::BeginRun(const std::shared_ptr<const JEvent>& event)
{


  return;
}

void DEventProcessor_truth_dirc::Process(const std::shared_ptr<const JEvent>& event) {
  vector<const DBeamPhoton*> beam_photons;
  vector<const DMCThrown*> mcthrowns;
  vector<const DMCTrackHit*> mctrackhits;
  vector<const DDIRCTruthBarHit*> dircBarHits;
  vector<const DDIRCTruthPmtHit*> dircPmtHits;
  vector<const DDIRCPmtHit*> dircRecoPmtHits;
  
  event->Get(beam_photons);
  event->Get(mcthrowns);
  event->Get(mctrackhits);
  event->Get(dircPmtHits);
  event->Get(dircBarHits);
  event->Get(dircRecoPmtHits);

  // get DIRC geometry
  vector<const DDIRCGeometry*> locDIRCGeometryVec;
  event->Get(locDIRCGeometryVec);
  auto locDIRCGeometry = locDIRCGeometryVec[0];

  for (unsigned int j = 0; j < dircBarHits.size(); j++){
    //double px = dircBarHits[j]->px;
    //double py = dircBarHits[j]->py;
    //double pz = dircBarHits[j]->pz;

    double x = dircBarHits[j]->x;
    double y = dircBarHits[j]->y;
    int bar = dircBarHits[j]->bar;

    GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK
    hTruthBarHitXY->Fill(x, y);
    hTruthBarHitBar->Fill(bar);
    GetLockService(event)->RootUnLock();
  }

  for (unsigned int h = 0; h < dircPmtHits.size(); h++){
	
     int ch=dircPmtHits[h]->ch;
    
     double x = dircPmtHits[h]->x;
     double y = dircPmtHits[h]->y;
     double z = dircPmtHits[h]->z;
     double t = dircPmtHits[h]->t;
     double t_fixed = dircPmtHits[h]->t_fixed;
     double lambda = 1240./dircPmtHits[h]->E/1e9;

     // get PMT labels
     int pmt_column = locDIRCGeometry->GetPmtColumn(ch); 
     int pmt_row = locDIRCGeometry->GetPmtRow(ch);

     // get pixel labels
     int pixel_row = locDIRCGeometry->GetPixelRow(ch);
     int pixel_col = locDIRCGeometry->GetPixelColumn(ch);

     GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK
     hTruthWavelength->Fill(lambda);
     hTruthPixelHitTime->Fill(ch, t-t_fixed);
     if(x < 0.) {
	hTruthPmtHitZY_South->Fill(z, y);
	hTruthPmtHit_South->Fill(pmt_column, pmt_row);
	hTruthPixelHit_South->Fill(pixel_row, pixel_col);
     }
     else {
	hTruthPmtHitZY_North->Fill(z, y);
	hTruthPmtHit_North->Fill(pmt_column, pmt_row);
	hTruthPixelHit_North->Fill(pixel_row, pixel_col);
     }
     GetLockService(event)->RootUnLock();
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

	  GetLockService(event)->RootWriteLock(); //ACQUIRE ROOT LOCK
	  if(ch < 108*64) {
		  hPixelHit_South->Fill(pixel_row, pixel_col);
	  }
	  else {
		  hPixelHit_North->Fill(pixel_row, pixel_col);
	  }
	  GetLockService(event)->RootUnLock();
  }
}

void DEventProcessor_truth_dirc::EndRun() {
}

void DEventProcessor_truth_dirc::Finish() {
}

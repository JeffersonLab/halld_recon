// $Id$
//
//    File: JEventProcessor_FCAL_Pi0TOF.cc
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_Pi0TOF.h"

#include "DANA/DEvent.h"

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FCAL_Pi0TOF());
  }
} // "C"


//------------------
// JEventProcessor_FCAL_Pi0TOF (Constructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::JEventProcessor_FCAL_Pi0TOF()
{
  DO_METHOD = 0;
  USE_TRACKS = 0;

  japp->SetDefaultParameter( "FCAL_Pi0TOF:DO_METH0D", DO_METHOD );
  japp->SetDefaultParameter( "FCAL_Pi0TOF:USE_TRACKS", USE_TRACKS );

  m_time_FCALRF_cut = 3.0;
  m_time_FCALFCAL_cut = 5.0;
  
  japp->SetDefaultParameter( "FCAL_Pi0TOF:time_FCALRF_cut", m_time_FCALRF_cut );
  japp->SetDefaultParameter( "FCAL_Pi0TOF:time_FCALFCAL_cut", m_time_FCALFCAL_cut );
  
  cout <<"DO_METHOD " << DO_METHOD << " USE_TRACKS " << USE_TRACKS << endl;
}

//------------------
// ~JEventProcessor_FCAL_Pi0TOF (Destructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::~JEventProcessor_FCAL_Pi0TOF()
{

}

//------------------
// Init
//------------------
void JEventProcessor_FCAL_Pi0TOF::Init()
{
  // This is called once at program startup. 

  gDirectory->mkdir("FCAL_Pi0");
  gDirectory->cd("FCAL_Pi0");
  hCurrentGainConstants = new TProfile("CurrentGainConstants", "Current Gain Constants", 2800, -0.5, 2799.5);
  gDirectory->cd("..");
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_Pi0TOF::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes

  // Put the current gain constants into the output file
  vector< double > raw_gains;
  // This is called whenever the run number changes
  GetCalib(event, "/FCAL/gains", raw_gains);
  for (unsigned int i=0; i<raw_gains.size(); i++){
    hCurrentGainConstants->Fill(i,raw_gains[i]);
  }
  DGeometry* dgeom = GetDGeometry(event);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    throw JException("No geometry accessbile to ccal_timing monitoring plugin.");
  }
  JCalibration *jcalib = GetJCalibration(event);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");
}

//------------------
// Process
//------------------
void JEventProcessor_FCAL_Pi0TOF::Process(const std::shared_ptr<const JEvent>& event)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // event->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // event->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);
  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1) throw JException("Missing DFCALGeometry");
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  
  vector<const DNeutralParticle *> neutralParticleVector;
  event->Get(neutralParticleVector);
  
  vector<const DTOFPoint*> tof_points;
  event->Get(tof_points);
  
  vector<const DVertex*> kinfitVertex;
  event->Get(kinfitVertex);

  vector<const DL1Trigger *> locL1Triggers;
  vector<const DBeamPhoton *> locBeamPhotons;  
  event->Get(locL1Triggers);
  event->Get(locBeamPhotons);
  /*
  const DEventRFBunch * locEventRFBunches = NULL;                                                                                                                                                               try {                                                                                                                                                                                               
    event->GetSingle(locEventRFBunches, "CalorimeterOnly" );                                                                                                                                      
  } catch (...) { 
    return; 
  }                                                                                                                                                                   
  double locRFTime = locEventRFBunches->dTime;
  if(locEventRFBunches->dNumParticleVotes < 3 ) 
    return;    
  */
  vector<const DEventRFBunch*> locEventRFBunches;
  event->Get(locEventRFBunches);
  double locRFTime = locEventRFBunches.empty() ? 0.0 : locEventRFBunches[0]->dTime;
  
  // uint32_t locL1Trigger_fp = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->fp_trig_mask;
  // uint32_t locL1Trigger = locL1Triggers.empty() ? 0.0 : locL1Triggers[0]->trig_mask;
  
  int trig_bit[33];
  if (locL1Triggers.size() > 0) {
    for (unsigned int bit = 0; bit < 32; bit ++) {
      trig_bit[bit + 1] = (locL1Triggers[0]->trig_mask & (1 << bit)) ? 1 : 0;
      if(trig_bit[bit + 1] == 1) //htrig_bit->Fill(Float_t(bit+1));
	Fill1DHistogram("FCAL_Pi0HFA","","trig_bit",
			Float_t(bit+1),
			";Trigger bit #;Count [a.u.]",
			100, 0., 100.);
      
    }
  }
  
  DVector3 vertex;
  vertex.SetXYZ(m_beamSpotX, m_beamSpotY, m_targetZ);
  
  //Use kinfit when available
  double kinfitVertexX = m_beamSpotX;
  double kinfitVertexY = m_beamSpotY;
  double kinfitVertexZ = m_targetZ;
  
  vector< const JObject* > locObjectsToSave;
  if (USE_TRACKS == 0) {
    for (unsigned int i = 0 ; i < kinfitVertex.size(); i++) {
      kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
      kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
      kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
    }
  }
  
  for (unsigned int i = 0; i < neutralParticleVector.size(); i++){
    const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
    //bool bo_pho1 = true;
    //if( photon1 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) bo_pho1 = false;
    if( photon1 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) continue;
    // Go into the FCAL shower and find the largest energy deposition
    const DNeutralShower *shower1 = photon1->Get_NeutralShower();
    if(shower1->dDetectorSystem != SYS_FCAL) continue;
    DFCALShower *fcalShower1 = (DFCALShower *) shower1->dBCALFCALShower;
    const DFCALCluster *fcalCluster1;
    fcalShower1->GetSingle(fcalCluster1);
    int ch1 = fcalCluster1->getChannelEmax();
    double emax1 = fcalCluster1->getEmax();
    double xShower1=fcalShower1->getPosition().X();
    double yShower1=fcalShower1->getPosition().Y();
    double zShower1=fcalShower1->getPosition().Z();
    double xShowerlog1=fcalShower1->getPosition_log().X();
    double yShowerlog1=fcalShower1->getPosition_log().Y();
    double zShowerlog1=fcalShower1->getPosition_log().Z();
    xShower1 = xShowerlog1;
    yShower1 = yShowerlog1;
    zShower1 = zShowerlog1;
    int row = fcalGeom.row(static_cast<float>(yShowerlog1));
    int col = fcalGeom.column(static_cast<float>(xShowerlog1));
    double x1 = xShower1 - kinfitVertexX;
    double y1 = yShower1 - kinfitVertexY;
    double z1 = zShower1 - kinfitVertexZ;
    double xl1 = xShowerlog1 - kinfitVertexX;
    double yl1 = yShowerlog1 - kinfitVertexY;
    double zl1 = zShowerlog1 - kinfitVertexZ;
    double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
    double radiusShowerlog1=sqrt(pow(xShowerlog1,2)+pow(yShowerlog1,2));
    double radius1 = fcalGeom.positionOnFace(ch1).Mod();
    int ring1 = (int) (radius1 / (5 * k_cm));
    int rings1 = (int) (radiusShower1 / (5 * k_cm));
    int ringl1 = (int) (radiusShowerlog1 / (5 * k_cm));
    
    // double frac_ring_thres = frac_thres_1_to_5;
    // if (ring1 >= 6)
    //   frac_ring_thres = frac_thres_6_to_23;
    // double frac_rings_thres = frac_thres_1_to_5;
    // if (rings1 >= 6)
    //   frac_rings_thres = frac_thres_6_to_23;
    // double frac_ringl_thres = frac_thres_1_to_5;
    // if (ringl1 >= 6)
    //   frac_ringl_thres = frac_thres_6_to_23;
    
    double frac1 = fcalCluster1->getEmax()/fcalCluster1->getEnergy();
    double Eclust1 = fcalCluster1->getEnergy();
    double Ephoton1 = photon1->lorentzMomentum().E();
    DVector3 vertex1(xl1, yl1, zl1);
    double r1 = vertex1.Mag();
    double t1 = fcalShower1->getTime() - (r1 / TMath::C() * 1e7);
    double p1 = Ephoton1 ;
    double p1x = p1 * sin(vertex1.Theta()) * cos(vertex1.Phi());
    double p1y = p1 * sin(vertex1.Theta()) * sin(vertex1.Phi());
    double p1z = p1 * cos(vertex1.Theta());
    TLorentzVector photon1P4(p1x, p1y, p1z, p1);
    double tdiff1 = t1 - locRFTime;
    
    Fill2DHistogram("XY","","xy_all", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
    Fill2DHistogram("CR","","xy_all", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    if (emax1 > 1.0) {
      Fill2DHistogram("XY","","xy_1GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_1GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 2.0) {
      Fill2DHistogram("XY","","xy_2GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_2GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 3.0) {
      Fill2DHistogram("XY","","xy_3GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_3GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 4.0) {
      Fill2DHistogram("XY","","xy_4GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_4GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }    
    if (emax1 > 5.0) {
      Fill2DHistogram("XY","","xy_5GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_5GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 6.0) {
      Fill2DHistogram("XY","","xy_6GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_6GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }    
    if (emax1 > 7.0) {
      Fill2DHistogram("XY","","xy_7GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_7GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 8.0) {
      Fill2DHistogram("XY","","xy_8GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_8GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 9.0) {
      Fill2DHistogram("XY","","xy_9GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_9GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    if (emax1 > 10.0) {
      Fill2DHistogram("XY","","xy_10GeV", xl1, yl1, ";x [cm];y [cm];Counts", 500, -125., 125., 500, -125., 125.);
      Fill2DHistogram("CR","","xy_10GeV", col - 29, row - 29, ";Column;Row;Counts", 59, -30, 30, 59, -30, 30);
    }
    
    Fill1DHistogram("FCAL_Pi0HFA","","tdiff1", tdiff1, ";t_{#gamma}^{1} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99.);
    Fill1DHistogram("FCAL_Pi0HFA","","E", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","","P", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","","Emax", emax1, ";E_{max};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("Emax_ring_%d", ring1), emax1, ";E_{max};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_ring_%d", ring1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_ring_%d", ring1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_rings_%d", rings1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_rings_%d", rings1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("E_ringl_%d", ringl1), Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
    Fill1DHistogram("FCAL_Pi0HFA","",Form("P_ringl_%d", ringl1), Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    if(radiusShower1 < 108.4239 && radiusShower1 > 20.785){
      Fill1DHistogram("FCAL_Pi0HFA","","E_cut", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
      Fill1DHistogram("FCAL_Pi0HFA","","P_cut", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    }
    if(radiusShowerlog1 < 108.4239 && radiusShowerlog1 > 20.785){
      Fill1DHistogram("FCAL_Pi0HFA","","E_cutlog", Eclust1, ";E_{cluster};Count [a.u.]", 12000, 0., 12.);
      Fill1DHistogram("FCAL_Pi0HFA","","P_cutlog", Ephoton1, ";E_{#gamma};Count [a.u.]", 12000, 0., 12.);
    }

    int tof_match1 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x1, y1, z1);
    int ltof_match1 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x1, y1, z1);
    for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
      
      double xtof = (*tof_p)->pos.X() - kinfitVertexX;
      double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
      double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
      double xltof = (*tof_p)->pos.X() - kinfitVertexX;
      double yltof = (*tof_p)->pos.Y() - kinfitVertexY;
      double zltof = (*tof_p)->pos.Z() - kinfitVertexZ;
      
      //double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
      //double ttof = (*tof_p)->t - (rtof/TMath::C());
      
      xtof = xtof * (z1 / ztof);
      ytof = ytof * (z1 / ztof);
      xltof = xltof * (zl1 / zltof);
      yltof = yltof * (zl1 / zltof);
      
      int hbar  = (*tof_p)->dHorizontalBar;			
      int hstat = (*tof_p)->dHorizontalBarStatus;
      int vbar  = (*tof_p)->dVerticalBar;
      int vstat = (*tof_p)->dVerticalBarStatus;
      
      double dx, dy;
      if( hstat==3 && vstat==3 ) {
	dx = x1 - xtof;
	dy = y1 - ytof;
      } else if( vstat==3 ) {
	dx = x1 - bar2x(vbar)*(z1 / ztof);
	dy = y1 - ytof;
      } else if( hstat==3 ) {
	dx = x1 - xtof;
	dy = y1 - bar2x(hbar)*(z1 / ztof);
      } else {
	dx = x1 - bar2x(vbar)*(z1 / ztof);
	dy = y1 - bar2x(hbar)*(z1 / ztof);
      }

      double dlx, dly;
      if( hstat==3 && vstat==3 ) {
	dlx = xl1 - xltof;
	dly = yl1 - yltof;
      } else if( vstat==3 ) {
	dlx = xl1 - bar2x(vbar)*(zl1 / zltof);
	dly = yl1 - yltof;
      } else if( hstat==3 ) {
	dlx = xl1 - xltof;
	dly = yl1 - bar2x(hbar)*(zl1 / zltof);
      } else {
	dlx = xl1 - bar2x(vbar)*(zl1 / zltof);
	dly = yl1 - bar2x(hbar)*(zl1 / zltof);
      }

      
      if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	//if( dt > -1. && dt < 3. ) tof_match = 1;
	tof_match1 = 1;
      }
      if( fabs(dlx) < 6. && fabs(dly) < 6. ) { 
	//if( dt > -1. && dt < 3. ) tof_match = 1;
	ltof_match1 = 1;
      }
    } // end DTOFPoint loop
        
    for (unsigned int j = i + 1; j < neutralParticleVector.size(); j ++){
      const DNeutralParticleHypothesis *photon2 = neutralParticleVector[j]->Get_Hypothesis(Gamma);
      //bool bo_pho2 = true;
      //if (photon2 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) bo_pho2 = false;
      if (photon2 == nullptr && (USE_TRACKS == 0 || USE_TRACKS == 2)) continue;
      const DNeutralShower *shower2 = photon2->Get_NeutralShower();
      if(shower2->dDetectorSystem != SYS_FCAL) continue;
      DFCALShower *fcalShower2 = (DFCALShower *) shower2->dBCALFCALShower;
      const DFCALCluster *fcalCluster2;
      fcalShower2->GetSingle(fcalCluster2);
      int ch2 = fcalCluster2->getChannelEmax();
      double xShower2=fcalShower2->getPosition().X();
      double yShower2=fcalShower2->getPosition().Y();
      double zShower2=fcalShower1->getPosition().Z();
      double xShowerlog2=fcalShower2->getPosition_log().X();
      double yShowerlog2=fcalShower2->getPosition_log().Y();
      double zShowerlog2=fcalShower2->getPosition_log().Z();
      xShower2 = xShowerlog2;
      yShower2 = yShowerlog2;
      zShower2 = zShowerlog2;
      double x2 = xShower2 - kinfitVertexX;
      double y2 = yShower2 - kinfitVertexY;
      double z2 = zShower2 - kinfitVertexZ;
      double xl2 = xShowerlog2 - kinfitVertexX;
      double yl2 = yShowerlog2 - kinfitVertexY;
      double zl2 = zShowerlog2 - kinfitVertexZ;
      double radiusShower2=sqrt(pow(xShower2,2)+pow(yShower2,2));
      double radiusShowerlog2=sqrt(pow(xShowerlog2,2)+pow(yShowerlog2,2));
      double radius2 = fcalGeom.positionOnFace(ch2).Mod();
      int ring2 = (int) (radius2 / (5 * k_cm));
      int rings2 = (int) (radiusShower2 / (5 * k_cm));
      int ringl2 = (int) (radiusShowerlog2 / (5 * k_cm));

      double frac_ring_thres = frac_thres_1_to_5;
      if (ring2 >= 6)
	frac_ring_thres = frac_thres_6_to_23;
      double frac_rings_thres = frac_thres_1_to_5;
      if (rings2 >= 6)
	frac_rings_thres = frac_thres_6_to_23;
      double frac_ringl_thres = frac_thres_1_to_5;
      if (ringl2 >= 6)
	frac_ringl_thres = frac_thres_6_to_23;

      double frac2 = fcalCluster2->getEmax()/fcalCluster2->getEnergy();
      double Eclust2 = fcalCluster2->getEnergy();
      double Ephoton2 = photon2->lorentzMomentum().E();
      DVector3 vertex2(xl2, yl2, zl2);
      double r2 = vertex2.Mag();
      double t2 = fcalShower2->getTime() - (r2 / TMath::C() * 1e7);
      double p2 = Ephoton2;
      double p2x = p2 * sin(vertex2.Theta()) * cos(vertex2.Phi());
      double p2y = p2 * sin(vertex2.Theta()) * sin(vertex2.Phi());
      double p2z = p2 * cos(vertex2.Theta());
      TLorentzVector photon2P4(p2x, p2y, p2z, p2);
      double tdiff2 = t2 - locRFTime;
    
      Fill1DHistogram("FCAL_Pi0HFA","","tdiff2", tdiff2, ";t_{#gamma}^{2} - t_{RF} [ns];Count [a.u.]", 1000, -99., 99.);
      double Eratio = Eclust1 / Eclust2;
      bool bo_5p = false;
      bool bo_10p = false;
      bool bo_20p = false;
      bool bo_30p = false;
      if (0.95 <= Eratio && Eratio <= 1.05) bo_5p = true;
      if (0.90 <= Eratio && Eratio <= 1.10) bo_10p = true;
      if (0.80 <= Eratio && Eratio <= 1.20) bo_20p = true;
      if (0.70 <= Eratio && Eratio <= 1.30) bo_30p = true;
      double pi0Mass = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).M();
      double pi0Masslog = (photon1P4 + photon2P4).M();
      pi0Mass = pi0Masslog;
      double theta_pi0 = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).Theta() * TMath::RadToDeg();
      double pi0P = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).P();
      double theta_pi0log = (photon1P4 + photon2P4).Theta() * TMath::RadToDeg();
      theta_pi0 = theta_pi0log;
      double pi0E = (photon1P4 + photon2P4).E();
      double pi0Plog = (photon1P4 + photon2P4).P();
      pi0P = pi0Plog;
      double avgE = 0.5*fcalCluster1->getEnergy() + 0.5*fcalCluster2->getEnergy();
      DVector3 distance = vertex1 - vertex2;
      double d = distance.Mag();
      bool api0 = false;
      bool aeta = false;
      if (0.11 <= pi0Masslog && pi0Masslog <= 0.16) api0 = true;
      if (0.51 <= pi0Masslog && pi0Masslog <= 0.58) aeta = true;
      
      int tof_match2 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x2, y2, z2);
      int ltof_match2 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x2, y2, z2);
      for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
	
	double xtof = (*tof_p)->pos.X() - kinfitVertexX;
	double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
	double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
	double xltof = (*tof_p)->pos.X() - kinfitVertexX;
	double yltof = (*tof_p)->pos.Y() - kinfitVertexY;
	double zltof = (*tof_p)->pos.Z() - kinfitVertexZ;
	
	//double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
	//double ttof = (*tof_p)->t - (rtof/TMath::C());
	
	xtof = xtof * (z2 / ztof);
	ytof = ytof * (z2 / ztof);
	xltof = xltof * (zl2 / zltof);
	yltof = yltof * (zl2 / zltof);
	
	int hbar  = (*tof_p)->dHorizontalBar;			
	int hstat = (*tof_p)->dHorizontalBarStatus;
	int vbar  = (*tof_p)->dVerticalBar;
	int vstat = (*tof_p)->dVerticalBarStatus;
	
	double dx, dy;
	if( hstat==3 && vstat==3 ) {
	  dx = x2 - xtof;
	  dy = y2 - ytof;
	} else if( vstat==3 ) {
	  dx = x2 - bar2x(vbar)*(z2 / ztof);
	  dy = y2 - ytof;
	} else if( hstat==3 ) {
	  dx = x2 - xtof;
	  dy = y2 - bar2x(hbar)*(z2 / ztof);
	} else {
	  dx = x2 - bar2x(vbar)*(z2 / ztof);
	  dy = y2 - bar2x(hbar)*(z2 / ztof);
	}

	double dlx, dly;
	if( hstat==3 && vstat==3 ) {
	  dlx = xl2 - xltof;
	  dly = yl2 - yltof;
	} else if( vstat==3 ) {
	  dlx = xl2 - bar2x(vbar)*(zl2 / zltof);
	  dly = yl2 - yltof;
	} else if( hstat==3 ) {
	  dlx = xl2 - xltof;
	  dly = yl2 - bar2x(hbar)*(zl2 / zltof);
	} else {
	  dlx = xl2 - bar2x(vbar)*(zl2 / zltof);
	  dly = yl2 - bar2x(hbar)*(zl2 / zltof);
	}
	
	if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	  //if( dt > -1. && dt < 3. ) tof_match = 1;
	  tof_match2 = 1;
	}
	if( fabs(dlx) < 6. && fabs(dly) < 6. ) { 
	  //if( dt > -1. && dt < 3. ) tof_match = 1;
	  ltof_match2 = 1;
	}
      } // end DTOFPoint loop
      
      if (((tof_match1 == 0 && tof_match2 == 0) || (ltof_match1 == 0 && ltof_match2 == 0)) && 
	  (fabs(tdiff1) < m_time_FCALRF_cut) && (fabs(tdiff2) < m_time_FCALRF_cut) && 
	  (fabs(t1-t2) < m_time_FCALFCAL_cut)) {
	
	//if (bo_pho1 && bo_pho2) {
	if (DO_METHOD == 0) {
	  Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass",
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_%d", ring1),
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass_ring_%d", ring2),
			  pi0Mass,
			  "#pi^{0} Mass; #pi^{0} Mass;",
			  500, 0.05, 0.7);
	  if (neutralParticleVector.size() == 2) {
	    Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass2g",
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2g_ring_%d", ring1),
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	    Fill1DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2g_ring_%d", ring2),
			    pi0Mass, 
			    "#pi^{0} Mass; #pi^{0} Mass;",
			    500, 0.05, 0.7);
	  }
	  if(frac1 > frac_ring_thres){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_Fid",
			      ch1, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_FidBoth",
				ch1, pi0Mass,
				"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      }
	    }
	    if(radiusShower1<20.785 || radiusShower2<20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_InnerRing",
			      ch1, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    }
	    
	  }
	  if(frac2 > frac_ring_thres){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			    ch2, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_Fid",
			      ch2, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      if(radiusShower2<108.4239 && radiusShower2>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_FidBoth",
				ch2, pi0Mass,
				"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      }
	    }
	    if(radiusShower1<20.785 || radiusShower2<20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_InnerRing",
			      ch1, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    }
	    
	  }
	  // Energy Weighted Pi0 Mass
	  for(auto hit : fcalCluster1->GetHits()){
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
				    hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
				    hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    if(radiusShower2<108.4239  && radiusShower2>20.785){
	      Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
				      hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
				      hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
					hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
					"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
					hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
					"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      }
	    }
	    
	  }
	  
	  for(auto hit : fcalCluster2->GetHits()){
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
				    hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
				    hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 500, 0.05, 0.7);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
				      hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
				      hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      if(radiusShower2<108.4239 && radiusShower2>20.785){
		Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
					hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
					"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
					hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
					"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					2800, -0.5, 2799.5, 500, 0.05, 0.7);
	      }
	    }
	    
	  }
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring1),
			    avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 500, 0.05, 0.7);
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring2),
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 500, 0.05, 0.7);
	  if (bo_5p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_5_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_5_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_10p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_10_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_10_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_20p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_20_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_20_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (bo_30p){
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_30_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_30_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	  }
	  if (neutralParticleVector.size() == 2) {
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_ring_%d", ring1),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_ring_%d", ring2),
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    if (bo_5p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_10p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_20p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    }
	    if (bo_30p){
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	    } 
	  }
	  if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.25){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_Fiducial",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_FiducialBoth",
				avgE, pi0Mass,
				"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
				100, 0.0, 10.0, 500, 0.05, 0.7);
	      }
	    }
	    
	  }
	  if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.5){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500_Fiducial",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500_FiducialBoth",
				avgE, pi0Mass,
				"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
				100, 0.0, 10.0, 500, 0.05, 0.7);
	      }
	    }
	    
	  }
	  if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.1){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100_Fiducial",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100_FiducialBoth",
				avgE, pi0Mass,
				"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
				100, 0.0, 10.0, 500, 0.05, 0.7);
	      }
	    }
	  }
	  if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.05){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 500, 0.05, 0.7);
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50_Fiducial",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 500, 0.05, 0.7);
	      if(radiusShower1<108.4239 && radiusShower1>20.785){
		Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50_FiducialBoth",
				avgE, pi0Mass,
				"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
				100, 0.0, 10.0, 500, 0.05, 0.7);
	      }
	    }
	  }
	}
	//}
	
	if (DO_METHOD == 1 || DO_METHOD == 2) {
	  for (unsigned int k = 0; k < (unsigned int) locBeamPhotons.size(); k ++) {
	    
	    const DBeamPhoton *ebeam = locBeamPhotons[k]; 
	    //double eb = ebeam->lorentzMomentum().E();
	    double tb = ebeam->time();
	    double zb = ebeam->position().Z();
	    double eb = ebeam->lorentzMomentum().E();
	    double locDeltaTRF = tb - (locRFTime + (zb - m_targetZ) / 29.9792458);
	    Fill1DHistogram("FCAL_Pi0TOF","","TaggerTiming1", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
	    //Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t1 - t2, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100.);
	    //Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t2 - t1, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100.);
	    double weight = 0;
	    if (fabs(locDeltaTRF) <= 2.004) {
	      weight = 1;
	    } else if ( ( -(2.004 + 3.0 * 4.008) <= locDeltaTRF && locDeltaTRF <= -(2.004 + 4.008) ) || 
			( (2.004 + 4.008) <= locDeltaTRF && locDeltaTRF <= (2.004 + 3.0 * 4.008) ) ) {
	      weight = -0.25;
	    } else {
	      continue;
	    }
	    double dE = eb - pi0E;
	    if (DO_METHOD == 1) {
	      if (tof_match1 == 0 && tof_match2 == 0) {
		
		Fill1DHistogram("FCAL_Pi0TOF","","TaggerTiming2", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t1 - t2, pi0Mass, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","","PhotonTiming1", t2 - t1, pi0Mass, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		
		Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass",
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_%d", ring1),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_rings_%d", rings1),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_ring_%d", ring2),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_rings_%d", rings2),
					pi0Mass, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);

		if (frac1 > frac_ring_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_th_ring_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac1 > frac_rings_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_th_rings_%d", rings1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac2 > frac_ring_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_th_ring_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac2 > frac_rings_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass_th_rings_%d", rings2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);

		
		if (neutralParticleVector.size() == 2) {
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","","Pi0Mass",
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_rings_%d", rings1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_ring_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_rings_%d", rings2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);

		  if (frac1 > frac_ring_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_th_ring_%d", ring1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac1 > frac_rings_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_th_rings_%d", rings1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac2 > frac_ring_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_th_ring_%d", ring2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac2 > frac_rings_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2g_th_rings_%d", rings2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);

		}	  
		if(frac1 > frac_rings_thres){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
					  ch1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  
		  if(radiusShower2<108.4239 && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_Fid",
					    ch1, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_FidBoth",
					      ch1, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  if(radiusShower1<20.785 || radiusShower2<20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_InnerRing",
					    ch1, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  }
		  
		}
		if(frac2 > frac_rings_thres){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
					  ch2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShower1<108.4239 && radiusShower1>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_Fid",
					    ch2, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShower2<108.4239 && radiusShower2>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_FidBoth",
					      ch2, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  if(radiusShower1<20.785 || radiusShower2<20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_InnerRing",
					    ch1, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  }
		  
		}
		// Energy Weighted Pi0 Mass
		for(auto hit : fcalCluster1->GetHits()){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted",
					  hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy() * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared",
					  hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShower2<108.4239  && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_Fid",
					    hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy() * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_Fid",
					    hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_FidBoth",
					      hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy() * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_FidBoth",
					      hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		
		for(auto hit : fcalCluster2->GetHits()){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted",
					  hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy() * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared",
					  hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShower1<108.4239 && radiusShower1>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_Fid",
					    hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy() * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_Fid",
					    hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShower2<108.4239 && radiusShower2>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_FidBoth",
					      hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy() * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_FidBoth",
					      hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_%d", ring1),
					Eclust1, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_ring_%d", ring2),
					Eclust2, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_rings_%d", rings1),
					Eclust1, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_rings_%d", rings2),
					Eclust2, pi0Mass, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);

		if (api0) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_theta_%d", ring1),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_theta_%d", ring2),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_thetas_%d", rings1),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_thetas_%d", rings2),
					  pi0P, theta_pi0, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		}

		if (neutralParticleVector.size() == 2) {
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_ring_%d", ring1),
					  Eclust1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_ring_%d", ring2),
					  Eclust2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_rings_%d", rings1),
					  Eclust1, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_rings_%d", rings2),
					  Eclust2, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		
		if (bo_5p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_rings_%d", rings1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_rings_%d", rings2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_thetas_%d", rings1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_5_thetas_%d", rings2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_rings_%d", rings1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_5_rings_%d", rings2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_5",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if(radiusShower2<108.4239 && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_5_Fiducial",
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_5_FiducialBoth",
					      avgE, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		if (bo_10p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_rings_%d", rings1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_rings_%d", rings2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_thetas_%d", rings1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_10_thetas_%d", rings2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_rings_%d", rings1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_10_rings_%d", rings2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_10",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		  if(radiusShower2<108.4239 && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_10_Fiducial",
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_10_FiducialBoth",
					      avgE, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		if (bo_20p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_rings_%d", rings1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_rings_%d", rings2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_thetas_%d", rings1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_20_thetas_%d", rings2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_rings_%d", rings1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_20_rings_%d", rings2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_20",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		  if(radiusShower2<108.4239 && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_20_Fiducial",
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_20_FiducialBoth",
					      avgE, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		}
		if (bo_30p){
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_%d", ring1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_ring_%d", ring2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_rings_%d", rings1),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_rings_%d", rings2),
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_theta_%d", ring1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_theta_%d", ring2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_thetas_%d", rings1),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0MassVsE_30_thetas_%d", rings2),
					    pi0P, theta_pi0, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_rings_%d", rings1),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","",Form("Pi0Mass2gVsE_30_rings_%d", rings2),
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_30",
					  avgE, pi0Mass, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if(radiusShower2<108.4239 && radiusShower2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_30_Fiducial",
					    avgE, pi0Mass, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShower1<108.4239 && radiusShower1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsE_30_FiducialBoth",
					      avgE, pi0Mass, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		}
	      }
	    }
	    if (DO_METHOD == 2) {
	      if (ltof_match1 == 0 && ltof_match2 == 0) {
		
		Fill1DHistogram("FCAL_Pi0log","","TaggerTiming1", locDeltaTRF, ";t_{e^{-}} - t_{#gamma} [ns];Count [a.u.]", 500, -100., 100.);
		Fill2DWeightedHistogram("FCAL_Pi0log","","PhotonTiming1", t1 - t2, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","","PhotonTiming1", t2 - t1, pi0Masslog, weight, ";t_{#gamma} - t_{#gamma} [ns]; pi0 mass [GeV];Count [a.u.]", 500, -100., 100., 500, 0.05, 0.7);
		
		if (api0 && neutralParticleVector.size() == 2 && eb > 8.0) {
		  Fill1DWeightedHistogram("FCAL_Pi0log","","DeltaE",
					  dE, weight,
					  ";E_{#gamma} - E_{#pi^{0}} [GeV];Count [a.u.]",
					  1200, -2., 10.);
		  if (ring1 > 1 && ring2 > 1) {
		    Fill1DWeightedHistogram("FCAL_Pi0log","","DeltaE_ring",
					    dE, weight,
					    ";E_{#gamma} - E_{#pi^{0}} [GeV];Count [a.u.]",
					    1200, -2., 10.);
		  }
		}
		
		if (api0)
		  Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi0Clusters",
					  ring1, d, weight,
					  ";ring #;Distance between two clusters [cm];Count [a.u.]",
					  30, 0, 30, 500, 0., 100.);
		if (aeta)
		  Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi0Clusters",
					  ring1, d, weight,
					  ";ring #;Distance between two clusters [cm];Count [a.u.]",
					  30, 0, 30, 500, 0., 100.);
		
		
		
		Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass",
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_%d", ring1),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ringl_%d", ringl1),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ring_%d", ring2),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);
		Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_ringl_%d", ringl2),
					pi0Masslog, weight,
					"#pi^{0} Mass; #pi^{0} Mass;",
					500, 0.05, 0.7);

		if (frac1 > frac_ring_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_th_ring_%d", ring1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac1 > frac_ringl_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_th_ringl_%d", rings1),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac2 > frac_ring_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_th_ring_%d", ring2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		if (frac2 > frac_ringl_thres)
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass_th_ringl_%d", rings2),
					  pi0Mass, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		
		if (neutralParticleVector.size() == 2) {
		  if (api0)
		    Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi02gClusters",
					    ring1, d, weight,
					    ";ring #;Distance between two clusters [cm];Count [a.u.]",
					    30, 0, 30, 500, 0., 100.);
		  if (aeta)
		    Fill2DWeightedHistogram("FCAL_Pi0log","","DistanceBetweenPi02gClusters",
					    ring1, d, weight,
					    ";ring #;Distance between two clusters [cm];Count [a.u.]",
					    30, 0, 30, 500, 0., 100.);
		  
		  Fill1DWeightedHistogram("FCAL_Pi0log","","Pi0Mass",
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_%d", ring1),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ringl_%d", ringl1),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ring_%d", ring2),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);
		  Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_ringl_%d", ringl2),
					  pi0Masslog, weight,
					  "#pi^{0} Mass; #pi^{0} Mass;",
					  500, 0.05, 0.7);

		  if (frac1 > frac_ring_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_th_ring_%d", ring1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac1 > frac_ringl_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_th_ringl_%d", rings1),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac2 > frac_ring_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_th_ring_%d", ring2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		  if (frac2 > frac_ringl_thres)
		    Fill1DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2g_th_ringl_%d", rings2),
					    pi0Mass, weight,
					    "#pi^{0} Mass; #pi^{0} Mass;",
					    500, 0.05, 0.7);
		}	  
		if(frac1 > frac_ringl_thres){
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum",
					  ch1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  
		  if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_Fid",
					    ch1, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_FidBoth",
					      ch1, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  if(radiusShowerlog1<20.785 || radiusShowerlog2<20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_InnerRing",
					    ch1, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  }
		  
		}
		if(frac2 > frac_ringl_thres){
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum",
					  ch2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_Fid",
					    ch2, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_FidBoth",
					      ch2, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  if(radiusShowerlog1<20.785 || radiusShowerlog2<20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNum_InnerRing",
					    ch1, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  }
		  
		}
		// Energy Weighted Pi0 Mass
		for(auto hit : fcalCluster1->GetHits()){
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted",
					  hit.ch, pi0Masslog, hit.E / fcalCluster1->getEnergy() * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared",
					  hit.ch, pi0Masslog, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShowerlog2<108.4239  && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted_Fid",
					    hit.ch, pi0Masslog, hit.E / fcalCluster1->getEnergy() * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared_Fid",
					    hit.ch, pi0Masslog, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted_FidBoth",
					      hit.ch, pi0Masslog, hit.E / fcalCluster1->getEnergy() * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared_FidBoth",
					      hit.ch, pi0Masslog, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()) * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		
		for(auto hit : fcalCluster2->GetHits()){
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted",
					  hit.ch, pi0Masslog, hit.E / fcalCluster2->getEnergy() * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared",
					  hit.ch, pi0Masslog, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					  2800, -0.5, 2799.5, 500, 0.05, 0.7);
		  if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted_Fid",
					    hit.ch, pi0Masslog, hit.E / fcalCluster2->getEnergy() * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared_Fid",
					    hit.ch, pi0Masslog, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					    2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeighted_FidBoth",
					      hit.ch, pi0Masslog, hit.E / fcalCluster2->getEnergy() * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsChNumWeightedSquared_FidBoth",
					      hit.ch, pi0Masslog, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()) * weight,
					      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
					      2800, -0.5, 2799.5, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_%d", ring1),
					Eclust1, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ring_%d", ring2),
					Eclust2, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ringl_%d", ringl1),
					Eclust1, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_ringl_%d", ringl2),
					Eclust2, pi0Masslog, weight,
					"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					100, 0.0, 10.0, 500, 0.05, 0.7);
		if (api0) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_theta_%d", ring1),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_theta_%d", ring2),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_thetal_%d", ringl1),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_thetal_%d", ringl2),
					  pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 180, 0., 18.);
		}
		if (neutralParticleVector.size() == 2) {
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_%d", ring1),
					  Eclust1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ring_%d", ring2),
					  Eclust2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ringl_%d", ringl1),
					  Eclust1, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_ringl_%d", ringl2),
					  Eclust2, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		}
		
		if (bo_5p){
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ring_%d", ring1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ring_%d", ring2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ringl_%d", ringl1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_ringl_%d", ringl2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_theta_%d", ring1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_theta_%d", ring2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_thetal_%d", ringl1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_5_thetal_%d", ringl2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ringl_%d", ringl1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_5_ringl_%d", ringl2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_5",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_5_Fiducial",
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_5_FiducialBoth",
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		if (bo_10p){
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ring_%d", ring1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ring_%d", ring2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ringl_%d", ringl1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_ringl_%d", ringl2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_theta_%d", ring1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_theta_%d", ring2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_thetal_%d", ringl1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_10_thetal_%d", ringl2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ringl_%d", ringl1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_10_ringl_%d", ringl2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_10",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		  if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_10_Fiducial",
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_10_FiducialBoth",
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		  
		}
		if (bo_20p){
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ring_%d", ring1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ring_%d", ring2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ringl_%d", ringl1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_ringl_%d", ringl2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_theta_%d", ring1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_theta_%d", ring2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_thetal_%d", ringl1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_20_thetal_%d", ringl2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ringl_%d", ringl1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_20_ringl_%d", ringl2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_20",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  
		  if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_20_Fiducial",
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_20_FiducialBoth",
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		}
		if (bo_30p){
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ring_%d", ring1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ring_%d", ring2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ringl_%d", ringl1),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_ringl_%d", ringl2),
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if (api0) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_theta_%d", ring1),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_theta_%d", ring2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_thetal_%d", ringl1),
					    pi0Plog, theta_pi0log, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0MassVsE_30_thetal_%d", ringl2),
					    pi0Plog, theta_pi0log, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 180, 0., 18.);
		  }
		  if (neutralParticleVector.size() == 2) {
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ring_%d", ring1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ring_%d", ring2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ringl_%d", ringl1),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    Fill2DWeightedHistogram("FCAL_Pi0log","",Form("Pi0Mass2gVsE_30_ringl_%d", ringl2),
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		  }
		  Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_30",
					  avgE, pi0Masslog, weight,
					  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
					  100, 0.0, 10.0, 500, 0.05, 0.7);
		  if(radiusShowerlog2<108.4239 && radiusShowerlog2>20.785){
		    Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_30_Fiducial",
					    avgE, pi0Masslog, weight,
					    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					    100, 0.0, 10.0, 500, 0.05, 0.7);
		    if(radiusShowerlog1<108.4239 && radiusShowerlog1>20.785){
		      Fill2DWeightedHistogram("FCAL_Pi0log","","Pi0MassVsE_30_FiducialBoth",
					      avgE, pi0Masslog, weight,
					      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
					      100, 0.0, 10.0, 500, 0.05, 0.7);
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}

//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_Pi0TOF::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FCAL_Pi0TOF::Finish()
{
  // Called before program exit after event processing is finished.
}


//--------------------------------------------
// Get TOF position from bar
//--------------------------------------------
double JEventProcessor_FCAL_Pi0TOF::bar2x(int bar) {
  
  int ic = 2*bar - 45; 
  
  double pos;
  if( ic ==  1  || ic == -1 ) pos = 3.0*(double)ic;
  else if( ic ==  3  || ic ==  5 ) pos = 1.5*(double)(ic+2);
  else if( ic == -3  || ic == -5 ) pos = 1.5*(double)(ic-2);
  else if( ic >  5 ) pos = 3.*(ic-2);
  else pos = 3.*(ic+2);
  
  double x = 1.1*pos;
  
  return x;
}
/*
int JEventProcessor_FCAL_Pi0TOF::TOF_Match(double kinVertexX, double kinVertexY, double kinVertexZ, double x, double y, double z) {
  
  //-----   Check for match between TOF and FCAL   -----//
  int tof_match = 0;
  for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
    
    double xtof = (*tof_p)->pos.X() - kinVertexX;
    double ytof = (*tof_p)->pos.Y() - kinVertexY;
    double ztof = (*tof_p)->pos.Z() - kinVertexZ;
    
    //double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
    //double ttof = (*tof_p)->t - (rtof/TMath::C());
    
    xtof = xtof * (z / ztof);
    ytof = ytof * (z / ztof);
    
    int hbar  = (*tof_p)->dHorizontalBar;			
    int hstat = (*tof_p)->dHorizontalBarStatus;
    int vbar  = (*tof_p)->dVerticalBar;
    int vstat = (*tof_p)->dVerticalBarStatus;
    
    double dx, dy;
    if( hstat==3 && vstat==3 ) {
      dx = x - xtof;
      dy = y - ytof;
    } else if( vstat==3 ) {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - ytof;
    } else if( hstat==3 ) {
      dx = x - xtof;
      dy = y - bar2x(hbar)*(z / ztof);
    } else {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - bar2x(hbar)*(z / ztof);
    }
    
    if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
      //if( dt > -1. && dt < 3. ) tof_match = 1;
      tof_match = 1;
    }
  } // end DTOFPoint loop
  
  return tof_match;
}
*/




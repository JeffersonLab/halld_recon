// $Id$
//
//    File: JEventProcessor_TrackingPulls.cc
// Created: Thu Nov  3 14:30:19 EDT 2016
// Creator: mstaib (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_TrackingPulls.h"
#include "DANA/DEvent.h"
#include "HistogramTools.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"
#include "TRIGGER/DTrigger.h"


// Routine used to create our JEventProcessor
extern "C" {
void InitPlugin(JApplication *app) {
  InitJANAPlugin(app);
  app->Add(new JEventProcessor_TrackingPulls());
}
}  // "C"

thread_local DTreeFillData JEventProcessor_TrackingPulls::dTreeFillData;

JEventProcessor_TrackingPulls::JEventProcessor_TrackingPulls() {}

JEventProcessor_TrackingPulls::~JEventProcessor_TrackingPulls() {}

void JEventProcessor_TrackingPulls::Init() {
  // This is called once at program startup.
  auto app = GetApplication();
  int numlayers = 24;
  int numrings = 28;
  unsigned int numstraws[28] = {
      42,  42,  54,  54,  66,  66,  80,  80,  93,  93,  106, 106, 123, 123,
      135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};

  // Use -PTRACKINGPULLS:MAKE_TREE=1 to produce tree output
  MAKE_TREE = 0;
  app->SetDefaultParameter("TRACKINGPULLS:MAKE_TREE", MAKE_TREE, "Make a ROOT tree file");

  if (MAKE_TREE){
    string treeName = "tree_tracking_pulls";
    string treeFile = "tree_tracking_pulls.root";
    app->SetDefaultParameter("TRACKINGPULLS:TREENAME", treeName);
    app->SetDefaultParameter("TRACKINGPULLS:TREEFILE", treeFile);
    dTreeInterface = DTreeInterface::Create_DTreeInterface(treeName, treeFile);

    //TTREE BRANCHES
    DTreeBranchRegister locTreeBranchRegister;

    locTreeBranchRegister.Register_Single<Int_t>("eventNumber");
    locTreeBranchRegister.Register_Single<Int_t>("L1TriggerBits");

    locTreeBranchRegister.Register_Single<Int_t>("nFdcPlanes");
    locTreeBranchRegister.Register_Single<Int_t>("nCdcRings");
    locTreeBranchRegister.Register_Single<Int_t>("eventnumber");
    locTreeBranchRegister.Register_Single<Int_t>("track_index");
    locTreeBranchRegister.Register_Single<Double_t>("chi2");
    locTreeBranchRegister.Register_Single<Int_t>("ndf");
    locTreeBranchRegister.Register_Single<Int_t>("ncdchits");
    locTreeBranchRegister.Register_Single<Int_t>("nfdchits");
    locTreeBranchRegister.Register_Single<Double_t>("charge");
    locTreeBranchRegister.Register_Single<Double_t>("mom");
    locTreeBranchRegister.Register_Single<Double_t>("phi");
    locTreeBranchRegister.Register_Single<Double_t>("theta");
    locTreeBranchRegister.Register_Single<Double_t>("pos_x");
    locTreeBranchRegister.Register_Single<Double_t>("pos_y");
    locTreeBranchRegister.Register_Single<Double_t>("pos_z");
    locTreeBranchRegister.Register_Single<Int_t>("smoothed");
    locTreeBranchRegister.Register_Single<Int_t>("any_nan");
    locTreeBranchRegister.Register_Single<Int_t>("cdc_ring_multi_hits");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_resi", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_resic", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_err", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_errc", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_x", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_y", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_z", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_w", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_s", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_d", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("fdc_tdrift", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Int_t>("fdc_wire", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Int_t>("fdc_left_right", "nFdcPlanes");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("cdc_resi", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("cdc_err", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("cdc_z", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("cdc_tdrift", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Int_t>("cdc_straw", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Int_t>("cdc_left_right", "nCdcRings");
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("cdc_phi_intersect", "nCdcRings");

    //REGISTER BRANCHES
    dTreeInterface->Create_Branches(locTreeBranchRegister);
  }
  
  
    TDirectory *main = gDirectory;
    gDirectory->mkdir("TrackingPulls")->cd();

    gDirectory->mkdir("TrackInfo")->cd();

	hTrackingFOM = new TH1I("Tracking FOM", "Tracking FOM", 200, 0.0, 1.0);
	hTrack_PVsTheta = new TH2I("P Vs. Theta", "P Vs. #theta; #theta [deg.]; |P| [GeV/c]", 70, 0.0, 140.0, 50, 0.0, 10.0);
	hTrack_PhiVsTheta = new TH2I("Phi Vs. Theta", "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0, 140.0, 180, -180.0, 180.0);
	hTrack_PVsPhi = new TH2I("P Vs. Phi", "P Vs. #phi; #phi [deg.]; |P| [GeV/c]", 180, -180, 180.0, 50, 0.0, 10.0);

	gDirectory->cd("..");

    gDirectory->mkdir("TrackInfo_SmoothFailure")->cd();

	hTrackingFOM_SmoothFailure = new TH1I("Tracking FOM", "Tracking FOM", 200, 0.0, 1.0);
	hTrack_PVsTheta_SmoothFailure = new TH2I("P Vs. Theta", "P Vs. #theta; #theta [deg.]; |P| [GeV/c]", 70, 0.0, 140.0, 50, 0.0, 10.0);
	hTrack_PhiVsTheta_SmoothFailure = new TH2I("Phi Vs. Theta", "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0, 140.0, 180, -180.0, 180.0);
	hTrack_PVsPhi_SmoothFailure = new TH2I("P Vs. Phi", "P Vs. #phi; #phi [deg.]; |P| [GeV/c]", 180, -180, 180.0, 50, 0.0, 10.0);

	gDirectory->cd("..");

    gDirectory->mkdir("TrackInfo_SmoothSuccess")->cd();

	hTrackingFOM_SmoothSuccess = new TH1I("Tracking FOM", "Tracking FOM", 200, 0.0, 1.0);
	hTrack_PVsTheta_SmoothSuccess = new TH2I("P Vs. Theta", "P Vs. #theta; #theta [deg.]; |P| [GeV/c]", 70, 0.0, 140.0, 50, 0.0, 10.0);
	hTrack_PhiVsTheta_SmoothSuccess = new TH2I("Phi Vs. Theta", "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0, 140.0, 180, -180.0, 180.0);
	hTrack_PVsPhi_SmoothSuccess = new TH2I("P Vs. Phi", "P Vs. #phi; #phi [deg.]; |P| [GeV/c]", 180, -180, 180.0, 50, 0.0, 10.0);

	gDirectory->cd("..");

    gDirectory->mkdir("TrackInfo_SmoothSuccess_NaN")->cd();

	hTrackingFOM_SmoothSuccess_NaN = new TH1I("Tracking FOM", "Tracking FOM", 200, 0.0, 1.0);
	hTrack_PVsTheta_SmoothSuccess_NaN = new TH2I("P Vs. Theta", "P Vs. #theta; #theta [deg.]; |P| [GeV/c]", 70, 0.0, 140.0, 50, 0.0, 10.0);
	hTrack_PhiVsTheta_SmoothSuccess_NaN = new TH2I("Phi Vs. Theta", "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0, 140.0, 180, -180.0, 180.0);
	hTrack_PVsPhi_SmoothSuccess_NaN = new TH2I("P Vs. Phi", "P Vs. #phi; #phi [deg.]; |P| [GeV/c]", 180, -180, 180.0, 50, 0.0, 10.0);

	gDirectory->cd("..");

    gDirectory->mkdir("TrackPulls")->cd();

	hAllPulls = new TH1I("All Pulls", "Residual/Error", 100, -5.0, 5.0);
	hAllPulls_Vs_P = new TH2I("All Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0);
	hAllPulls_Vs_Phi = new TH2I("All Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0);
	hAllPulls_Vs_Theta = new TH2I("All Pulls Vs. Theta", ";#theta ;Residual/Error", 140, 0.0, 140.0, 100, -5.0, 5.0);
	hAllPulls_Vs_NDF = new TH2I("All Pulls Vs. NDF", ";Track NDF ;Residual/Error", 140, 0.0, 140.0, 100, -5.0, 5.0);
	hAllPulls_Vs_TrackingFOM = new TH2I("All Pulls Vs. Tracking FOM", ";Track FOM ;Residual/Error", 140, 0.0, 140.0, 100, -5.0, 5.0);
	hFDCWireError = new TH1I("FDC Wire Error", "Wire Residual Error", 100, 0.0, 0.1);
	hFDCCathodeError = new TH1I("FDC Cathode Error", "Cathode Residual Error", 100, 0.0, 0.1);
	hCDCError = new TH1I("CDCError", "Residual Error", 100, 0.0, 0.1);

	gDirectory->cd("..");

    gDirectory->mkdir("FDCPulls")->cd();

	hFDCAllWirePulls = new TH1I("All Wire Pulls", "Residual/Error", 100, -5.0, 5.0);
	hFDCAllCathodeulls = new TH1I("All Cathode Pulls", "Residual/Error", 100, -5.0, 5.0);
	hFDCAllWireResiduals = new TH1I("All Wire Residuals", "Residual", 100, -0.1, 0.1);
	hFDCAllCathodeResiduals = new TH1I("All Cathode Residuals", "Residual", 100, -0.1, 0.1);
	hFDCAllWireResidualsVsPlane = new TH2I("All Wire Residuals Vs. Plane", ";plane ;Residual", 24, 0.5, 24.5, 100, -0.1, 0.1);
	hFDCAllCathodeResidualsVsPlane = new TH2I("All Cathode Residuals Vs. Plane", ";plane ;Residual", 24, 0.5, 24.5, 100, -0.1, 0.1);
	hFDCAllWirePullsVsPlane = new TH2I("All Wire Pulls Vs. Plane", ";plane ;Residual/Error", 24, 0.5, 24.5, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsPlane = new TH2I("All Cathode Pulls Vs. Plane", ";plane ;Residual/Error", 24, 0.5, 24.5, 100, -5.0, 5.0);
	hFDCAllWireResidualsVsDriftTime = new TH2I("All Wire Residuals Vs Drift Time", ";Drift Time;Residual", 170, -20.0, 150.0, 100, -0.1, 0.1);
	hFDCAllWirePullsVsDriftTime = new TH2I("All Wire Pulls Vs Drift Time", ";Drift Time;Residual/Error", 170, -20.0, 150.0, 100, -5.0, 5.0);
	hFDCAllWirePullsVsP = new TH2I("All Wire Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0);
	hFDCAllWirePullsVsPhi = new TH2I("All Wire Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0);
	hFDCAllWirePullsVsTheta = new TH2I("All Wire Pulls Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsP = new TH2I("All Cathode Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsPhi = new TH2I("All Cathode Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsTheta = new TH2I("All Cathode Pulls Vs. Theta",  ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0);
	hFDCAllWireResidualsVsP = new TH2I("All Wire Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1);
	hFDCAllWireResidualsVsPhi = new TH2I("All Wire Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1);
	hFDCAllWireResidualsVsTheta = new TH2I("All Wire Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1);
	hFDCAllCathodeResidualsVsP = new TH2I("All Cathode Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1);
	hFDCAllCathodeResidualsVsPhi = new TH2I( "All Cathode Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1);
	hFDCAllCathodeResidualsVsTheta = new TH2I("All Cathode Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1);
	hFDCAllWirePullsVsNDF = new TH2I("All Wire Pulls Vs. NDF", ";Track NDF ;Residual/Error", 50, 0.5, 50.5, 100, -5.0, 5.0);
	hFDCAllWirePullsVsTrackingFOM = new TH2I("All Wire Pulls Vs. Tracking FOM", ";Track FOM ;Residual/Error", 100, 0.0, 1.0, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsNDF = new TH2I("All Cathode Pulls Vs. NDF", ";Track NDF ;Residual/Error", 50, 0.5, 50.5, 100, -5.0, 5.0);
	hFDCAllCathodePullsVsTrackingFOM = new TH2I("All Cathode Pulls Vs. Tracking FOM", ";Track FOM ;Residual/Error", 100, 0.0, 1.0, 100, -5.0, 5.0);

	gDirectory->cd("..");

	for(int layer=0; layer<numlayers; layer++) {
		// Make the Per-Plane Histograms
		char name[256];
		sprintf(name, "FDCPulls_Plane%.2i", layer+1);

		gDirectory->mkdir(name)->cd();

		hFDCAllWirePulls_ByLayer.push_back( new TH1I("All Wire Pulls", "Residual/Error", 100, -5.0, 5.0) );
		hFDCAllCathodeulls_ByLayer.push_back( new TH1I("All Cathode Pulls", "Residual/Error", 100, -5.0, 5.0) );
		hFDCAllWireResiduals_ByLayer.push_back( new TH1I("All Wire Residuals", "Residual", 100, -0.1, 0.1) );
		hFDCAllCathodeResiduals_ByLayer.push_back( new TH1I("All Cathode Residuals", "Residual", 100, -0.1, 0.1) );
		hFDCWireResidualsGoodTracks_ByLayer.push_back( new TH1I("wire_residual", "Residual", 200, -0.1, 0.1) );
		hFDCWireResidualsGoodTracksRight_ByLayer.push_back( new TH1I("wire_residual_right", "Residual", 200, -0.1, 0.1) );
		hFDCWireResidualsGoodTracksLeft_ByLayer.push_back( new TH1I("wire_residual_left", "Residual", 200, -0.1, 0.1) );
		hFDCAllWireResidualsVsDriftTime_ByLayer.push_back( new TH2I("All Wire Residuals Vs Drift Time", ";Drift Time;Residual", 170, -20.0, 150.0, 100, -0.1, 0.1) );
		hFDCAllWirePullsVsDriftTime_ByLayer.push_back( new TH2I("All Wire Pulls Vs Drift Time", ";Drift Time;Residual/Error", 170, -20.0, 150.0, 100, -5.0, 5.0) );
		hFDCAllWirePullsVsP_ByLayer.push_back( new TH2I("All Wire Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0) );
		hFDCAllWirePullsVsPhi_ByLayer.push_back( new TH2I("All Wire Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0) );
		hFDCAllWirePullsVsTheta_ByLayer.push_back( new TH2I("All Wire Pulls Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0) );
		hFDCAllCathodePullsVsP_ByLayer.push_back( new TH2I("All Cathode Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0) );
		hFDCAllCathodePullsVsPhi_ByLayer.push_back( new TH2I("All Cathode Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0) );
		hFDCAllCathodePullsVsTheta_ByLayer.push_back( new TH2I("All Cathode Pulls Vs. Theta",  ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0) );
		hFDCAllWireResidualsVsP_ByLayer.push_back( new TH2I("All Wire Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1) );
		hFDCAllWireResidualsVsPhi_ByLayer.push_back( new TH2I("All Wire Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1) );
		hFDCAllWireResidualsVsTheta_ByLayer.push_back( new TH2I("All Wire Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1) );
		hFDCAllCathodeResidualsVsP_ByLayer.push_back( new TH2I("All Cathode Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1) );
		hFDCAllCathodeResidualsVsPhi_ByLayer.push_back( new TH2I( "All Cathode Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1) );
		hFDCAllCathodeResidualsVsTheta_ByLayer.push_back( new TH2I("All Cathode Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1) );

		hFDCWirePulls_ByLayer.push_back( new TH2I("Wire Pulls", ";Wire Number ;Residual/Error", 96, 0.5, 96.5, 100, -5.0, 5.0) );
		hFDCWireResiduals_ByLayer.push_back( new TH2I("Wire Residuals", ";Wire Number ;Residual", 96,  0.5, 96.5, 100, -0.1, 0.1) );
		hFDC2DWirePulls_ByLayer.push_back( new TProfile2D("2D Wire Hit Pulls", "Mean of Wire Pulls vs. PseudoHit XY", 100, -50., 50., 100, -50., 50.) );
		hFDC2DWireResiduals_ByLayer.push_back( new TProfile2D("2D Wire Hit Residuals", "Mean of Wire Residuals vs. PseudoHit XY", 100, -50., 50., 100, -50., 50.) );
		hFDC2DWireResidualsLocal_ByLayer.push_back( new TProfile2D("2D Wire Hit Residuals Local", "Mean of Wire Residuals vs. PseudoHit WS;Perpendicular Distance to Wire; Distance Along the Wire", 
				100, -50., 50., 100, -50., 50.) );
		hFDC2DCathodePulls_ByLayer.push_back( new TProfile2D("2D Cathode Hit Pulls", "Mean of Cathode Pulls vs. PseudoHit XY", 100, -50.,  50., 100, -50., 50.) );
		hFDC2DCathodeResiduals_ByLayer.push_back( new TProfile2D("2D Cathode Hit Residuals", "Mean of Cathode Residuals vs. PseudoHit XY", 100, -50., 50., 100, -50., 50.) );
		hFDC2DCathodeResidualsLocal_ByLayer.push_back( new TProfile2D("2D Cathode Hit Residuals Local", "Mean of Cathode Residuals vs. PseudoHit WS;Perpendicular Distance to Wire; Distance Along the Wire",
            	100, -50., 50., 100, -50., 50.) );

		gDirectory->cd("..");
	}
	
    gDirectory->mkdir("CDCPulls")->cd();

	hCDCAllPulls = new TH1I("All Pulls", "Residual/Error", 100, -5.0, 5.0);
	hCDCAllResiduals = new TH1I("All Residuals", "Residual", 100, -0.1, 0.1);
	hCDCAllResidualsVsRing = new TH2I("All Residuals Vs. Ring", ";Ring ;Residual", 24, 0.5, 24.5, 100, -0.1, 0.1);
	hCDCAllPullsVsRing = new TH2I("All Pulls Vs. Ring", ";Ring ;Residual/Error", 24, 0.5, 24.5, 100, -5.0, 5.0);
	hCDCAllResidualsVsDriftTime = new TH2I("All Residuals Vs. tdrift", ";Drift Time;Residual", 170, -20.0, 150.0, 100, -0.1, 0.1);
	hCDCAllPullsVsDriftTime = new TH2I("All Pulls Vs. tdrift", ";Drift Time;Residual/Error", 170, -20.0, 150.0, 100, -5.0, 5.0);
	hCDCAllPullsVsP = new TH2I("All Pulls Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -5.0, 5.0);
	hCDCAllPullsVsPhi = new TH2I("All  Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0);
	hCDCAllPullsVsTheta = new TH2I("All Pulls Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0);
	hCDCAllResidualsVsP = new TH2I("All Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1);
	hCDCAllResidualsVsPhi = new TH2I("All Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1);
	hCDCAllResidualsVsTheta = new TH2I("All Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1);
	hCDCAllPullsVsNDF = new TH2I("All Pulls Vs. NDF", ";Track NDF ;Residual/Error", 50, 0.5, 50.5, 100, -5.0, 5.0);
	hCDCAllPullsVsTrackingFOM = new TH2I("All Pulls Vs. Tracking FOM", ";Track FOM ;Residual/Error", 100, 0.0, 1.0, 100, -5.0, 5.0);

	gDirectory->cd("..");

	for(int ring=0; ring<numrings; ring++) {
		// Make the Per-Plane Histograms
		char name[256];
		sprintf(name, "CDCPulls_Ring%.2i", ring+1);

		double nStraw = numstraws[ring];

		gDirectory->mkdir(name)->cd();

		hCDCAllPulls_ByRing.push_back( new TH1I("All Pulls", "Residual/Error", 100, -5.0, 5.0) );
		hCDCAllResiduals_ByRing.push_back( new TH1I("All Residuals", "Residual", 100, -0.1, 0.1) );
		hCDCAllResidualsVsDriftTime_ByRing.push_back( new TH2I("All Residuals Vs. tdrift", ";Drift Time;Residual", 200, 0.0, 1000.0, 100, -0.1, 0.1) );
		hCDCAllPullsVsDriftTime_ByRing.push_back( new TH2I("All Pulls Vs. tdrift", ";Drift Time;Residual/Error", 200, 0.0, 1000.0, 100, -5.0, 5.0) );
		hCDCAllResidualsVsZ_ByRing.push_back( new TH2I("All Residuals Vs. z", ";z [cm];Residual", 200, -30., 200., 100, -0.1, 0.1) );
		hCDCAllPullsVsZ_ByRing.push_back( new TH2I("All Pulls Vs. z", ";z [cm];Residual/Error", 200, -30., 200., 100, -5.0, 5.0) );
		hCDCAllPullsVsP_ByRing.push_back( new TH2I("All Pulls Vs. P", ";|P| ;Residual/Error", 50, 0.0, 10.0, 100, -5.0, 5.0) );
		hCDCAllPullsVsPhi_ByRing.push_back( new TH2I("All Pulls Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0, 5.0) );
		hCDCAllPullsVsTheta_ByRing.push_back( new TH2I("All Pulls Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0, 5.0) );
		hCDCAllResidualsVsP_ByRing.push_back( new TH2I("All Residuals Vs. P", ";|P| ;Residual/Error", 100, 0.0, 10.0, 100, -0.1, 0.1) );
		hCDCAllResidualsVsPhi_ByRing.push_back( new TH2I("All Residuals Vs. Phi", ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1) );
		hCDCAllResidualsVsTheta_ByRing.push_back( new TH2I("All Residuals Vs. Theta", ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1) );

		hCDCStrawPulls_ByRing.push_back( new TH2I("Per Straw Pulls", ";Straw Number ;Residual/Error", nStraw, 0.5, nStraw + 0.5, 100, -5.0, 5.0) );
		hCDCStrawResiduals_ByRing.push_back( new TH2I("Per Straw Residuals",  ";Straw Number ;Residual", nStraw, 0.5, nStraw + 0.5, 100, -0.1, 0.1) );

		hCDCStrawResidualsVsPhiTheta_ByRing.push_back( new TProfile2D("Residual Vs Phi-Theta", ";#theta;#phi", 70, 0.0, 140.0, 180, -180.0, 180.0) );
		hCDCStrawResidualsVsPhiZ_ByRing.push_back( new TProfile2D("Residual Vs Phi-z", ";z;#phi", 200, 0.0, 200.0, 180, -180.0, 180.0) );
		hCDCStrawResidualsVsPhiIntersectZ_ByRing.push_back( new TProfile2D("Residual Vs PhiIntersect-z", ";z;#phi Intersect", 200, 0.0, 200.0, nStraw, -180.0, 180.0) );
		hCDCStrawResidualsVsPTheta_ByRing.push_back( new TProfile2D("Residual Vs P-Theta", ";#theta;|P|", 70, 0.0, 140.0, 50, 0.0, 10.0) );

		gDirectory->cd("..");
	}
	
	main->cd();
	
	
  return; //NOERROR;
}

void JEventProcessor_TrackingPulls::BeginRun(const std::shared_ptr<const JEvent> &event) {
  // This is called whenever the run number changes
}

void JEventProcessor_TrackingPulls::Process(const std::shared_ptr<const JEvent> &event) {
  auto eventnumber = event->GetEventNumber();
  static uint32_t evntCount = 0;
  evntCount++;
  // Loop over the tracks, get the tracking pulls, and fill some histograms.
  // Easy peasy

  const DTrigger *locTrigger = NULL;
  event->GetSingle(locTrigger);
  if (locTrigger->Get_L1FrontPanelTriggerBits() != 0) return;

  vector<const DChargedTrack *> chargedTrackVector;
  event->Get(chargedTrackVector);
  DEvent::GetLockService(event)->RootWriteLock();

  for (size_t i = 0; i < chargedTrackVector.size(); i++) {
    // TODO: Should be changed to use PID FOM when ready
    const DChargedTrackHypothesis *bestHypothesis =
        chargedTrackVector[i]->Get_BestTrackingFOM();

    if (bestHypothesis == NULL) continue;

    auto locTrackTimeBased = bestHypothesis->Get_TrackTimeBased();
    double trackingFOM =
        TMath::Prob(locTrackTimeBased->chisq, locTrackTimeBased->Ndof);

    // Some quality cuts for the tracks we will use
    // Keep this minimal for now and investigate later
    float trackingFOMCut = 0.001;
    int trackingNDFCut = 5;

    if (trackingFOM < trackingFOMCut) continue;
    if (locTrackTimeBased->Ndof < trackingNDFCut) continue;

    double phi = bestHypothesis->momentum().Phi() * TMath::RadToDeg();
    double theta = bestHypothesis->momentum().Theta() * TMath::RadToDeg();
    double pmag = bestHypothesis->momentum().Mag();

    if (pmag < 0.5) continue;
    
    // Fill some track information
    hTrackingFOM->Fill(trackingFOM);
    hTrack_PVsTheta->Fill(theta, pmag);
    hTrack_PhiVsTheta->Fill(theta, phi);
    hTrack_PVsPhi->Fill(phi, pmag);

    // Get the pulls vector from the track
    auto track = bestHypothesis->Get_TrackTimeBased();

    if (MAKE_TREE){
      dTreeFillData.Fill_Single<Int_t>("nFdcPlanes", 24);
      dTreeFillData.Fill_Single<Int_t>("nCdcRings", 28);

      // Initializes TTree variables.
      for (int j = 0; j < kNumFdcPlanes; ++j) {
	dTreeFillData.Fill_Array<Double_t>("fdc_resi", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_resic", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_err", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_errc", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_x", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_y", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_z", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_w", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_s", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_d", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("fdc_tdrift", -999.9, j);
	dTreeFillData.Fill_Array<Int_t>("fdc_wire", -999, j);
	dTreeFillData.Fill_Array<Int_t>("fdc_left_right", -999, j);
      }
      for (int j = 0; j < kNumCdcRings; ++j) {
	dTreeFillData.Fill_Array<Double_t>("cdc_resi", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("cdc_err", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("cdc_z", -999.9, j);
	dTreeFillData.Fill_Array<Double_t>("cdc_tdrift", -999.9, j);
	dTreeFillData.Fill_Array<Int_t>("cdc_straw", -999, j);
	dTreeFillData.Fill_Array<Int_t>("cdc_left_right", -999, j);
	dTreeFillData.Fill_Array<Double_t>("cdc_phi_intersect", -999.9, j);
      }

      dTreeFillData.Fill_Single<Int_t>("eventnumber", (int)eventnumber);
      dTreeFillData.Fill_Single<Int_t>("track_index", (int)i);
      dTreeFillData.Fill_Single<Double_t>("chi2", track->chisq);
      dTreeFillData.Fill_Single<Int_t>("ndf", track->Ndof);
      dTreeFillData.Fill_Single<Int_t>("ncdchits", track->measured_cdc_hits_on_track);
      dTreeFillData.Fill_Single<Int_t>("nfdchits", track->measured_fdc_hits_on_track);
      dTreeFillData.Fill_Single<Double_t>("charge", track->charge());
      dTreeFillData.Fill_Single<Double_t>("mom", track->momentum().Mag());
      dTreeFillData.Fill_Single<Double_t>("phi", track->momentum().Phi() * TMath::RadToDeg());
      dTreeFillData.Fill_Single<Double_t>("theta", track->momentum().Theta() * TMath::RadToDeg());
      dTreeFillData.Fill_Single<Double_t>("pos_x", track->position().X());
      dTreeFillData.Fill_Single<Double_t>("pos_y", track->position().Y());
      dTreeFillData.Fill_Single<Double_t>("pos_z", track->position().Z());
      dTreeFillData.Fill_Single<Int_t>("smoothed", (track->IsSmoothed ? 1 : 0));
    }

    if (!track->IsSmoothed) {
		hTrackingFOM_SmoothFailure->Fill(trackingFOM);
		hTrack_PVsTheta_SmoothFailure->Fill(theta, pmag);
		hTrack_PhiVsTheta_SmoothFailure->Fill(theta, phi);
		hTrack_PVsPhi_SmoothFailure->Fill(phi, pmag);
        continue;
    } else {
		hTrackingFOM_SmoothSuccess->Fill(trackingFOM);
		hTrack_PVsTheta_SmoothSuccess->Fill(theta, pmag);
		hTrack_PhiVsTheta_SmoothSuccess->Fill(theta, phi);
		hTrack_PVsPhi_SmoothSuccess->Fill(phi, pmag);
    }

    vector<DTrackFitter::pull_t> pulls = track->pulls;

    // Check for NaNs
    int any_nan = false;
    if (MAKE_TREE)
      dTreeFillData.Fill_Single<Int_t>("any_nan", any_nan);
    for (size_t iPull = 0; iPull < pulls.size(); iPull++) {
      double err = pulls[iPull].err;
      double errc = pulls[iPull].errc;
      if (err != err || errc != errc) {
	any_nan = true;
	if (MAKE_TREE)
	  dTreeFillData.Fill_Single<Int_t>("any_nan", any_nan);
        break;
      }
    }
    if (any_nan) {
		hTrackingFOM_SmoothSuccess_NaN->Fill(trackingFOM);
		hTrack_PVsTheta_SmoothSuccess_NaN->Fill(theta, pmag);
		hTrack_PhiVsTheta_SmoothSuccess_NaN->Fill(theta, phi);
		hTrack_PVsPhi_SmoothSuccess_NaN->Fill(phi, pmag);
        continue;
    }

    for (size_t iPull = 0; iPull < pulls.size(); iPull++) {
      double resi = pulls[iPull].resi;
      double err = pulls[iPull].err;  // estimated error of measurement
      double tdrift = pulls[iPull].tdrift;
      const DCDCTrackHit *cdc_hit = pulls[iPull].cdc_hit;
      const DFDCPseudo *fdc_hit = pulls[iPull].fdc_hit;
      double z = pulls[iPull].z;  // z position at doca
      double resic = pulls[iPull].resic;
      double errc = pulls[iPull].errc;

	  hAllPulls->Fill(resi / err);
	  hAllPulls_Vs_P->Fill(pmag, resi / err);
	  hAllPulls_Vs_Phi->Fill(phi, resi / err);
	  hAllPulls_Vs_Theta->Fill(theta, resi / err);
	  hAllPulls_Vs_NDF->Fill(locTrackTimeBased->Ndof, resi / err);
	  hAllPulls_Vs_TrackingFOM->Fill(trackingFOM, resi / err);

      if (fdc_hit != nullptr) {
		  hFDCWireError->Fill(err);
		  hFDCCathodeError->Fill(errc);
      } else {
	      hCDCError->Fill(err);
      }

      // Fill some detector specific info
      // Fill them in order = super-hacked
      static int nextPlane = 1;
      static int nextRing = 1;

      if (fdc_hit != nullptr && fdc_hit->wire->layer <= nextPlane) {
        if (fdc_hit->wire->layer == nextPlane) nextPlane++;

		hFDCAllWirePulls->Fill(resi / err);
		hFDCAllCathodeulls->Fill(resic / errc);
		hFDCAllWireResiduals->Fill(resi);
		hFDCAllCathodeResiduals->Fill(resic);
		hFDCAllWireResidualsVsPlane->Fill(fdc_hit->wire->layer, resi);
		hFDCAllCathodeResidualsVsPlane->Fill(fdc_hit->wire->layer, resic);
		hFDCAllWirePullsVsPlane->Fill(fdc_hit->wire->layer, resi / err);
		hFDCAllCathodePullsVsPlane->Fill(fdc_hit->wire->layer, resic / errc);
		hFDCAllWireResidualsVsDriftTime->Fill(tdrift, resi);
		hFDCAllWirePullsVsDriftTime->Fill(tdrift, resi / err);
		hFDCAllWirePullsVsP->Fill(pmag, resi / err);
		hFDCAllWirePullsVsTheta->Fill(theta, resi / err);
		hFDCAllWirePullsVsPhi->Fill(phi, resi / err);
		hFDCAllCathodePullsVsP->Fill(pmag, resic / errc);
		hFDCAllCathodePullsVsPhi->Fill(phi, resic / errc);
		hFDCAllCathodePullsVsTheta->Fill(theta, resic / errc);
		hFDCAllWireResidualsVsP->Fill(pmag, resi);
		hFDCAllWireResidualsVsPhi->Fill(phi, resi);
		hFDCAllWireResidualsVsTheta->Fill(theta, resi);
		hFDCAllCathodeResidualsVsP->Fill(pmag, resic);
		hFDCAllCathodeResidualsVsPhi->Fill(phi, resic);
		hFDCAllCathodeResidualsVsTheta->Fill(theta, resic);
		hFDCAllWirePullsVsNDF->Fill(locTrackTimeBased->Ndof, resi / err);
		hFDCAllWirePullsVsTrackingFOM->Fill(trackingFOM, resi / err);
		hFDCAllCathodePullsVsNDF->Fill(locTrackTimeBased->Ndof, resic / err);
		hFDCAllCathodePullsVsTrackingFOM->Fill(trackingFOM, resic / err);

		hFDCAllWirePulls_ByLayer[fdc_hit->wire->layer - 1]->Fill(resi / err);
		hFDCAllCathodeulls_ByLayer[fdc_hit->wire->layer - 1]->Fill(resic / errc);
		hFDCAllWireResiduals_ByLayer[fdc_hit->wire->layer - 1]->Fill(resi);
        if (trackingFOM > 0.02) {
                hFDCWireResidualsGoodTracks_ByLayer[fdc_hit->wire->layer - 1]->Fill(resi);
			if (pulls[iPull].left_right == 1) {
				hFDCWireResidualsGoodTracksRight_ByLayer[fdc_hit->wire->layer - 1]->Fill(resi);
			} else if (pulls[iPull].left_right == -1) {
				hFDCWireResidualsGoodTracksLeft_ByLayer[fdc_hit->wire->layer - 1]->Fill(resi);
			}
        }

		hFDCAllCathodeResiduals_ByLayer[fdc_hit->wire->layer - 1]->Fill(resic);
		hFDCAllWireResidualsVsDriftTime_ByLayer[fdc_hit->wire->layer - 1]->Fill(tdrift, resi);
		hFDCAllWirePullsVsDriftTime_ByLayer[fdc_hit->wire->layer - 1]->Fill(tdrift, resi / err);
		hFDCAllWirePullsVsP_ByLayer[fdc_hit->wire->layer - 1]->Fill(pmag, resi / err);
		hFDCAllWirePullsVsTheta_ByLayer[fdc_hit->wire->layer - 1]->Fill(theta, resi / err);
		hFDCAllWirePullsVsPhi_ByLayer[fdc_hit->wire->layer - 1]->Fill(phi, resi / err);
		hFDCAllCathodePullsVsP_ByLayer[fdc_hit->wire->layer - 1]->Fill(pmag, resic / errc);
		hFDCAllCathodePullsVsPhi_ByLayer[fdc_hit->wire->layer - 1]->Fill(phi, resic / errc);
		hFDCAllCathodePullsVsTheta_ByLayer[fdc_hit->wire->layer - 1]->Fill(theta, resic / errc);
		hFDCAllWireResidualsVsP_ByLayer[fdc_hit->wire->layer - 1]->Fill(pmag, resi);
		hFDCAllWireResidualsVsPhi_ByLayer[fdc_hit->wire->layer - 1]->Fill(phi, resi);
		hFDCAllWireResidualsVsTheta_ByLayer[fdc_hit->wire->layer - 1]->Fill(theta, resi);
		hFDCAllCathodeResidualsVsP_ByLayer[fdc_hit->wire->layer - 1]->Fill(pmag, resic);
		hFDCAllCathodeResidualsVsPhi_ByLayer[fdc_hit->wire->layer - 1]->Fill(phi, resic);
		hFDCAllCathodeResidualsVsTheta_ByLayer[fdc_hit->wire->layer - 1]->Fill(theta, resic);
		hFDCWirePulls_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->wire->wire, resi / err);
		hFDCWireResiduals_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->wire->wire, resi);
		
        if (fabs(resi / err) < 5.0) {
			hFDC2DWirePulls_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->xy.X(), fdc_hit->xy.Y(), resi / err);
        }
        if (fabs(resi) < 0.1) {
			hFDC2DWireResiduals_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->xy.X(), fdc_hit->xy.Y(), resi);
			hFDC2DWireResidualsLocal_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->w, fdc_hit->s, resi);
        }
        if (fabs(resic / errc) < 5.0) {
			hFDC2DCathodePulls_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->xy.X(), fdc_hit->xy.Y(), resic / err);
        }
        if (fabs(resic) < 0.1) {
			hFDC2DCathodeResiduals_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->xy.X(), fdc_hit->xy.Y(), resic);
			hFDC2DCathodeResidualsLocal_ByLayer[fdc_hit->wire->layer - 1]->Fill(fdc_hit->w, fdc_hit->s, resic);
        }


	if (MAKE_TREE){
	  dTreeFillData.Fill_Array<Double_t>("fdc_resi", resi, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_resic", resic, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_err", err, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_errc", errc, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_x", fdc_hit->xy.X(), fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_y", fdc_hit->xy.Y(), fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_z", pulls[iPull].z, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_w", fdc_hit->w, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_s", fdc_hit->s, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_d", pulls[iPull].d, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Double_t>("fdc_tdrift", tdrift, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Int_t>("fdc_wire", fdc_hit->wire->wire, fdc_hit->wire->layer - 1);
	  dTreeFillData.Fill_Array<Int_t>("fdc_left_right", pulls[iPull].left_right, fdc_hit->wire->layer - 1);
	}
      }

      // Once we are done with the FDC, move on to the CDC.
      if (cdc_hit != nullptr && cdc_hit->wire->ring <= nextRing &&
          (nextPlane == 25 || evntCount > 1000)) {
        if (cdc_hit->wire->ring == nextRing) nextRing++;

		hCDCAllPulls->Fill(resi / err);
		hCDCAllResiduals->Fill(resi);
		hCDCAllResidualsVsRing->Fill(cdc_hit->wire->ring, resi);
		hCDCAllPullsVsRing->Fill(cdc_hit->wire->ring, resi / err);
		hCDCAllResidualsVsDriftTime->Fill(tdrift, resi);
		hCDCAllPullsVsDriftTime->Fill(tdrift, resi / err);
		hCDCAllPullsVsP->Fill(pmag, resi / err);
		hCDCAllPullsVsTheta->Fill(theta, resi / err);
		hCDCAllPullsVsPhi->Fill(phi, resi / err);
		hCDCAllResidualsVsP->Fill(pmag, resi);
		hCDCAllResidualsVsPhi->Fill(phi, resi);
		hCDCAllResidualsVsTheta->Fill(theta, resi);
		hCDCAllPullsVsNDF->Fill(locTrackTimeBased->Ndof, resi / err);
		hCDCAllPullsVsTrackingFOM->Fill(trackingFOM, resi / err);

		hCDCAllPulls_ByRing[cdc_hit->wire->ring - 1]->Fill(resi / err);
		hCDCAllResiduals_ByRing[cdc_hit->wire->ring - 1]->Fill(resi);
		hCDCAllResidualsVsDriftTime_ByRing[cdc_hit->wire->ring - 1]->Fill(tdrift, resi);
		hCDCAllPullsVsDriftTime_ByRing[cdc_hit->wire->ring - 1]->Fill(tdrift, resi / err);
		hCDCAllResidualsVsZ_ByRing[cdc_hit->wire->ring - 1]->Fill(z, resi);
		hCDCAllPullsVsZ_ByRing[cdc_hit->wire->ring - 1]->Fill(z, resi / err);
		hCDCAllPullsVsP_ByRing[cdc_hit->wire->ring - 1]->Fill(pmag, resi / err);
		hCDCAllPullsVsTheta_ByRing[cdc_hit->wire->ring - 1]->Fill(theta, resi / err);
		hCDCAllPullsVsPhi_ByRing[cdc_hit->wire->ring - 1]->Fill(phi, resi / err);
		hCDCAllResidualsVsP_ByRing[cdc_hit->wire->ring - 1]->Fill(pmag, resi);
		hCDCAllResidualsVsPhi_ByRing[cdc_hit->wire->ring - 1]->Fill(phi, resi);
		hCDCAllResidualsVsTheta_ByRing[cdc_hit->wire->ring - 1]->Fill(theta, resi);

        double phiIntersect =
            (cdc_hit->wire->origin + (z - 92.0) * cdc_hit->wire->udir).Phi() *
            TMath::RadToDeg();

		hCDCStrawPulls_ByRing[cdc_hit->wire->ring - 1]->Fill(cdc_hit->wire->straw, resi / err);
		hCDCStrawResiduals_ByRing[cdc_hit->wire->ring -1]->Fill(cdc_hit->wire->straw, resi);

        if (fabs(resi) < 0.1) {
			hCDCStrawResidualsVsPhiTheta_ByRing[cdc_hit->wire->ring - 1]->Fill(theta, phi, resi);
			hCDCStrawResidualsVsPhiZ_ByRing[cdc_hit->wire->ring - 1]->Fill(z, phi, resi);
			hCDCStrawResidualsVsPhiIntersectZ_ByRing[cdc_hit->wire->ring - 1]->Fill(z, phiIntersect, resi);
			hCDCStrawResidualsVsPTheta_ByRing[cdc_hit->wire->ring - 1]->Fill(theta, pmag, resi);
        }

	if (MAKE_TREE){
	  dTreeFillData.Fill_Array<Double_t>("cdc_resi", resi, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Double_t>("cdc_err", err, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Double_t>("cdc_z", pulls[iPull].z, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Double_t>("cdc_tdrift", tdrift, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Int_t>("cdc_straw", cdc_hit->wire->straw, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Int_t>("cdc_left_right", pulls[iPull].left_right, cdc_hit->wire->ring - 1);
	  dTreeFillData.Fill_Array<Double_t>("cdc_phi_intersect", (cdc_hit->wire->origin + (z - 92.0) * cdc_hit->wire->udir).Phi() * TMath::RadToDeg(), cdc_hit->wire->ring - 1);
	}
      }
    }
    if (MAKE_TREE)
      dTreeInterface->Fill(dTreeFillData);
  }
  DEvent::GetLockService(event)->RootUnLock(); //RELEASE ROOT LOCK!!
}

void JEventProcessor_TrackingPulls::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

void JEventProcessor_TrackingPulls::Finish() {
  // Called before program exit after event processing is finished.
  if (MAKE_TREE)
    delete dTreeInterface; //saves trees to file, closes file
}

// $Id$
//
//    File: JEventProcessor_TrackingPulls_straight.h
// Created: Wed Jan 22 09:28:45 EST 2020
// Creator: keigo (on Linux ifarm1801 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_TrackingPulls_straight.h"
#include "HistogramTools.h"
#include "TRACKING/DTrackTimeBased.h"
#include "TRIGGER/DTrigger.h"

using namespace jana;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C" {
void InitPlugin(JApplication *app) {
  InitJANAPlugin(app);
  app->AddProcessor(new JEventProcessor_TrackingPulls_straight());
}
}  // "C"

JEventProcessor_TrackingPulls_straight::
    JEventProcessor_TrackingPulls_straight() {}

JEventProcessor_TrackingPulls_straight::
    ~JEventProcessor_TrackingPulls_straight() {}

jerror_t JEventProcessor_TrackingPulls_straight::init(void) {
  // This is called once at program startup.

  tree_ = new TTree("tracking_pulls", "tracking_pulls");
  tree_->SetAutoSave(1000);
  tree_->Branch("chi2", &chi2_, "chi2/D");
  tree_->Branch("ndf", &ndf_, "ndf/I");
  tree_->Branch("phi", &phi_, "phi/D");
  tree_->Branch("theta", &theta_, "theta/D");
  tree_->Branch("pos_x", &pos_x_, "pos_x/D");
  tree_->Branch("pos_y", &pos_y_, "pos_y/D");
  tree_->Branch("pos_z", &pos_z_, "pos_z/D");
  tree_->Branch("resi", resi_, "resi[24]/D");
  tree_->Branch("resic", resic_, "resic[24]/D");
  tree_->Branch("err", err_, "err[24]/D");
  tree_->Branch("errc", errc_, "errc[24]/D");
  tree_->Branch("x", x_, "x[24]/D");
  tree_->Branch("y", y_, "y[24]/D");
  tree_->Branch("z", z_, "z[24]/D");
  tree_->Branch("s", s_, "s[24]/D");
  tree_->Branch("w", w_, "w[24]/D");
  tree_->Branch("tdrift", tdrift_, "tdrift[24]/D");
  tree_->Branch("wire", wire_, "wire[24]/I");
  tree_->Branch("left_right", left_right_, "left_right[24]/I");

  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::brun(JEventLoop *eventLoop,
                                                      int32_t runnumber) {
  // This is called whenever the run number changes
  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::evnt(JEventLoop *loop,
                                                      uint64_t eventnumber) {
  unsigned int numstraws[28] = {
      42,  42,  54,  54,  66,  66,  80,  80,  93,  93,  106, 106, 123, 123,
      135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};
  static uint32_t evntCount = 0;
  evntCount++;
  // Loop over the tracks, get the tracking pulls, and fill some histograms.
  // Easy peasy

  const DTrigger *locTrigger = NULL;
  loop->GetSingle(locTrigger);
  if (locTrigger->Get_L1FrontPanelTriggerBits() != 0) return NOERROR;

  vector<const DTrackTimeBased *> trackVector;
  loop->Get(trackVector, "StraightLine");

  for (size_t i = 0; i < trackVector.size(); i++) {
    const DTrackTimeBased *track = trackVector[i];

    // Initializes TTree variables.
    for (int j = 0; j < 24; ++j) {
      resi_[j] = -999.9;
      resic_[j] = -999.9;
      err_[j] = -999.9;
      errc_[j] = -999.9;
      x_[j] = -999.9;
      y_[j] = -999.9;
      z_[j] = -999.9;
      w_[j] = -999.9;
      s_[j] = -999.9;
      tdrift_[j] = -999.9;
      wire_[j] = -999;
      left_right_[j] = -999;
    }

    chi2_ = track->chisq;
    ndf_ = track->Ndof;
    phi_ = track->momentum().Phi() * TMath::RadToDeg();
    theta_ = track->momentum().Theta() * TMath::RadToDeg();
    pos_x_ = track->position().X();
    pos_y_ = track->position().Y();
    pos_z_ = track->position().Z();

    double trackingFOM = TMath::Prob(chi2_, ndf_);

    // Some quality cuts for the tracks we will use
    // Keep this minimal for now and investigate later
    float trackingFOMCut = 1.0e-20;
    int trackingNDFCut = 20;

    if (trackingFOM < trackingFOMCut) continue;
    if (track->Ndof < trackingNDFCut) continue;

    double phi = phi_;
    double theta = theta_;

    // Fill some track information
    Fill1DHistogram("TrackingPulls", "TrackInfo", "Tracking FOM", trackingFOM,
                    "Tracking FOM", 200, 0.0, 1.0);
    Fill2DHistogram("TrackingPulls", "TrackInfo", "Phi Vs. Theta", theta, phi,
                    "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0,
                    140.0, 180, -180.0, 180.0);

    if (!track->IsSmoothed) {
      Fill1DHistogram("TrackingPulls", "TrackInfo_SmoothFailure",
                      "Tracking FOM", trackingFOM, "Tracking FOM", 200, 0.0,
                      1.0);
      Fill2DHistogram("TrackingPulls", "TrackInfo_SmoothFailure",
                      "Phi Vs. Theta", theta, phi,
                      "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0,
                      140.0, 180, -180.0, 180.0);
      continue;
    } else {
      Fill1DHistogram("TrackingPulls", "TrackInfo_SmoothSuccess",
                      "Tracking FOM", trackingFOM, "Tracking FOM", 200, 0.0,
                      1.0);
      Fill2DHistogram("TrackingPulls", "TrackInfo_SmoothSuccess",
                      "Phi Vs. Theta", theta, phi,
                      "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0,
                      140.0, 180, -180.0, 180.0);
    }

    vector<DTrackFitter::pull_t> pulls = track->pulls;

    // Check for NaNs
    bool anyNaN = false;
    for (size_t iPull = 0; iPull < pulls.size(); iPull++) {
      double err = pulls[iPull].err;
      double errc = pulls[iPull].errc;
      if (err != err || errc != errc) {
        anyNaN = true;
        break;
      }
    }
    if (anyNaN) {
      Fill1DHistogram("TrackingPulls", "TrackInfo_SmoothSuccess_NaN",
                      "Tracking FOM", trackingFOM, "Tracking FOM", 200, 0.0,
                      1.0);
      Fill2DHistogram("TrackingPulls", "TrackInfo_SmoothSuccess_NaN",
                      "Phi Vs. Theta", theta, phi,
                      "#phi Vs. #theta; #theta [deg.];  #phi [deg.]", 70, 0.0,
                      140.0, 180, -180.0, 180.0);
      continue;
    }

    for (size_t iPull = 0; iPull < pulls.size(); iPull++) {
      // Here is all of the information currently stored in the pulls from the
      // fit From TRACKING/DTrackFitter.h
      double resi = pulls[iPull].resi;      // residual of measurement
      double err = pulls[iPull].err;        // estimated error of measurement
      double tdrift = pulls[iPull].tdrift;  // drift time of this measurement
      const DCDCTrackHit *cdc_hit = pulls[iPull].cdc_hit;
      const DFDCPseudo *fdc_hit = pulls[iPull].fdc_hit;
      double z = pulls[iPull].z;  // z position at doca
      double resic =
          pulls[iPull].resic;  // residual for FDC cathode measuremtns
      double errc = pulls[iPull].errc;

      Fill1DHistogram("TrackingPulls", "TrackPulls", "All Pulls", resi / err,
                      "Residual/Error", 100, -5.0, 5.0);
      Fill2DHistogram("TrackingPulls", "TrackPulls", "All Pulls Vs. Phi", phi,
                      resi / err, ";#phi ;Residual/Error", 180, -180.0, 180.0,
                      100, -5.0, 5.0);
      Fill2DHistogram("TrackingPulls", "TrackPulls", "All Pulls Vs. Theta",
                      theta, resi / err, ";#theta ;Residual/Error", 140, 0.0,
                      140.0, 100, -5.0, 5.0);
      Fill2DHistogram("TrackingPulls", "TrackPulls", "All Pulls Vs. NDF",
                      track->Ndof, resi / err, ";Track NDF ;Residual/Error",
                      140, 0.0, 140.0, 100, -5.0, 5.0);
      Fill2DHistogram("TrackingPulls", "TrackPulls",
                      "All Pulls Vs. Tracking FOM", trackingFOM, resi / err,
                      ";Track FOM ;Residual/Error", 140, 0.0, 140.0, 100, -5.0,
                      5.0);
      if (fdc_hit != nullptr) {
        Fill1DHistogram("TrackingPulls", "TrackPulls", "FDC Wire Error", err,
                        "Wire Residual Error", 100, 0.0, 0.1);
        Fill1DHistogram("TrackingPulls", "TrackPulls", "FDC Cathode Error",
                        errc, "Cathode Residual Error", 100, 0.0, 0.1);
      } else {
        Fill1DHistogram("TrackingPulls", "TrackPulls", "CDCError", err,
                        "Residual Error", 100, 0.0, 0.1);
      }

      // Fill some detector specific info
      // Fill them in order = super-hacked
      static int nextPlane = 1;
      static int nextRing = 1;

      if (fdc_hit != nullptr && fdc_hit->wire->layer <= nextPlane) {
        if (fdc_hit->wire->layer == nextPlane) nextPlane++;
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Wire Residuals Vs. Plane", fdc_hit->wire->layer,
                        resi, ";plane ;Residual", 24, 0.5, 24.5, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Residuals Vs. Plane", fdc_hit->wire->layer,
                        resic, ";plane ;Residual", 24, 0.5, 24.5, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", "FDCPulls", "All Wire Pulls Vs. Plane",
                        fdc_hit->wire->layer, resi / err,
                        ";plane ;Residual/Error", 24, 0.5, 24.5, 100, -5.0,
                        5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Pulls Vs. Plane", fdc_hit->wire->layer,
                        resic / errc, ";plane ;Residual/Error", 24, 0.5, 24.5,
                        100, -5.0, 5.0);

        resi_[fdc_hit->wire->layer - 1] = resi;
        resic_[fdc_hit->wire->layer - 1] = resic;
        left_right_[fdc_hit->wire->layer - 1] = pulls[iPull].left_right;
        err_[fdc_hit->wire->layer - 1] = err;
        errc_[fdc_hit->wire->layer - 1] = errc;
        x_[fdc_hit->wire->layer - 1] = fdc_hit->xy.X();
        y_[fdc_hit->wire->layer - 1] = fdc_hit->xy.Y();
        w_[fdc_hit->wire->layer - 1] = fdc_hit->w;
        s_[fdc_hit->wire->layer - 1] = fdc_hit->s;
        tdrift_[fdc_hit->wire->layer - 1] = tdrift;
        wire_[fdc_hit->wire->layer - 1] = fdc_hit->wire->wire;
      }

      // Once we are done with the FDC, move on to the CDC.
      if (cdc_hit != nullptr && cdc_hit->wire->ring <= nextRing &&
          (nextPlane == 25 || evntCount > 1000)) {
        if (cdc_hit->wire->ring == nextRing) nextRing++;

        Fill1DHistogram("TrackingPulls", "CDCPulls", "All Residuals", resi,
                        "Residual", 100, -0.1, 0.1);
        Fill1DHistogram("TrackingPulls", "CDCPulls", "All Pulls", resi / err,
                        "Residual/Error", 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Pulls Vs. Ring",
                        cdc_hit->wire->ring, resi / err,
                        ";ring ;Residual/Error", 28, 0.5, 28.5, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Residuals Vs. Ring",
                        cdc_hit->wire->ring, resi, ";ring ;Residual", 28, 0.5,
                        28.5, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Pulls Vs. tdrift",
                        tdrift, resi / err, ";tdrift [ns] ;Residual/Error", 200,
                        0.0, 1000.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Residuals Vs. tdrift",
                        tdrift, resi, ";tdrift [ns] ;Residual", 200, 0.0,
                        1000.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Pulls Vs. Phi", phi,
                        resi / err, ";#phi ;Residual/Error", 180, -180.0, 180.0,
                        100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Pulls Vs. Theta",
                        theta, resi / err, ";#theta ;Residual/Error", 140, 0.0,
                        140.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Residuals Vs. Phi",
                        phi, resi, ";#phi ;Residual", 180, -180.0, 180.0, 100,
                        -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Residuals Vs. Theta",
                        theta, resi, ";#theta ;Residual", 140, 0.0, 140.0, 100,
                        -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", "CDCPulls", "All Pulls Vs. NDF",
                        track->Ndof, resi / err, ";Track NDF ;Residual/Error",
                        50, 0.5, 50.5, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "CDCPulls",
                        "All Pulls Vs. Tracking FOM", trackingFOM, resi / err,
                        ";Track FOM ;Residual/Error", 100, 0.0, 1.0, 100, -5.0,
                        5.0);

        // Make the Per-Ring Histograms
        char ringName[256];
        sprintf(ringName, "CDCPulls_Ring%.2i", cdc_hit->wire->ring);

        Fill1DHistogram("TrackingPulls", ringName, "All Residuals", resi,
                        "Residual", 100, -0.1, 0.1);
        Fill1DHistogram("TrackingPulls", ringName, "All Pulls", resi / err,
                        "Residual/Error", 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "All Pulls Vs. tdrift",
                        tdrift, resi / err, ";tdrift [ns] ;Residual/Error", 200,
                        0.0, 1000.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "All Pulls Vs. z", z,
                        resi / err, ";z [cm] ;Residual/Error", 200, -30., 200.,
                        100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "All Residuals Vs. tdrift",
                        tdrift, resi, ";tdrift [ns] ;Residual", 200, 0.0,
                        1000.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", ringName, "All Residuals Vs. z", z,
                        resi, ";z [cm] ;Residual", 200, -30., 200.0, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", ringName, "All Pulls Vs. Phi", phi,
                        resi / err, ";#phi ;Residual/Error", 180, -180.0, 180.0,
                        100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "All Pulls Vs. Theta", theta,
                        resi / err, ";#theta ;Residual/Error", 140, 0.0, 140.0,
                        100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "All Residuals Vs. Phi", phi,
                        resi, ";#phi ;Residual", 180, -180.0, 180.0, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", ringName, "All Residuals Vs. Theta",
                        theta, resi, ";#theta ;Residual", 140, 0.0, 140.0, 100,
                        -0.1, 0.1);

        double nStraw = numstraws[cdc_hit->wire->ring - 1];
        double phiIntersect =
            (cdc_hit->wire->origin + (z - 92.0) * cdc_hit->wire->udir).Phi() *
            TMath::RadToDeg();

        Fill2DHistogram("TrackingPulls", ringName, "Per Straw Pulls",
                        cdc_hit->wire->straw, resi / err,
                        ";Straw Number ;Residual/Error", nStraw, 0.5,
                        nStraw + 0.5, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", ringName, "Per Straw Residuals",
                        cdc_hit->wire->straw, resi, ";Straw Number ;Residual",
                        nStraw, 0.5, nStraw + 0.5, 100, -0.1, 0.1);

        if (fabs(resi) < 0.1) {
          Fill2DProfile("TrackingPulls", ringName, "Residual Vs Phi-Theta",
                        theta, phi, resi, ";#theta;#phi", 70, 0.0, 140.0, 180,
                        -180.0, 180.0);
          Fill2DProfile("TrackingPulls", ringName, "Residual Vs Phi-z", z, phi,
                        resi, ";z;#phi", 200, 0.0, 200.0, 180, -180.0, 180.0);
          Fill2DProfile("TrackingPulls", ringName, "Residual Vs PhiIntersect-z",
                        z, phiIntersect, resi, ";z;#phi Intersect", 200, 0.0,
                        200.0, nStraw, -180.0, 180.0);
        }
      }
    }
    tree_->Fill();
  }

  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::fini(void) {
  // Called before program exit after event processing is finished.
  tree_->Write();
  return NOERROR;
}

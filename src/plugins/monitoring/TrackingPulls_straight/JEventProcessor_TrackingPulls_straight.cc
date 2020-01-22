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

    double trackingFOM = TMath::Prob(track->chisq, track->Ndof);

    // Some quality cuts for the tracks we will use
    // Keep this minimal for now and investigate later
    float trackingFOMCut = 0.001;
    int trackingNDFCut = 5;

    if (trackingFOM < trackingFOMCut) continue;
    if (track->Ndof < trackingNDFCut) continue;

    double phi = track->momentum().Phi() * TMath::RadToDeg();
    double theta = track->momentum().Theta() * TMath::RadToDeg();

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
        Fill1DHistogram("TrackingPulls", "FDCPulls", "All Wire Pulls",
                        resi / err, "Residual/Error", 100, -5.0, 5.0);
        Fill1DHistogram("TrackingPulls", "FDCPulls", "All Cathode Pulls",
                        resic / errc, "Residual/Error", 100, -5.0, 5.0);
        Fill1DHistogram("TrackingPulls", "FDCPulls", "All Wire Residuals", resi,
                        "Residual", 100, -0.1, 0.1);
        Fill1DHistogram("TrackingPulls", "FDCPulls", "All Cathode Residuals",
                        resic, "Residual", 100, -0.1, 0.1);
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
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Wire Residuals Vs Drift Time", tdrift, resi,
                        ";Drift Time;Residual", 170, -20.0, 150.0, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Wire Pulls Vs Drift Time", tdrift, resi / err,
                        ";Drift Time;Residual/Error", 170, -20.0, 150.0, 100,
                        -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls", "All Wire Pulls Vs. Phi",
                        phi, resi / err, ";#phi ;Residual/Error", 180, -180.0,
                        180.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls", "All Wire Pulls Vs. Theta",
                        theta, resi / err, ";#theta ;Residual/Error", 50, 0.0,
                        25.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Pulls Vs. Phi", phi, resic / errc,
                        ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -5.0,
                        5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Pulls Vs. Theta", theta, resic / errc,
                        ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0,
                        5.0);
        Fill2DHistogram(
            "TrackingPulls", "FDCPulls", "All Wire Residuals Vs. Phi", phi,
            resi, ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1);
        Fill2DHistogram(
            "TrackingPulls", "FDCPulls", "All Wire Residuals Vs. Theta", theta,
            resi, ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1, 0.1);
        Fill2DHistogram(
            "TrackingPulls", "FDCPulls", "All Cathode Residuals Vs. Phi", phi,
            resic, ";#phi ;Residual/Error", 180, -180.0, 180.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Residuals Vs. Theta", theta, resic,
                        ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", "FDCPulls", "All Wire Pulls Vs. NDF",
                        track->Ndof, resi / err, ";Track NDF ;Residual/Error",
                        50, 0.5, 50.5, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Wire Pulls Vs. Tracking FOM", trackingFOM,
                        resi / err, ";Track FOM ;Residual/Error", 100, 0.0, 1.0,
                        100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Pulls Vs. NDF", track->Ndof, resic / errc,
                        ";Track NDF ;Residual/Error", 50, 0.5, 50.5, 100, -5.0,
                        5.0);
        Fill2DHistogram("TrackingPulls", "FDCPulls",
                        "All Cathode Pulls Vs. Tracking FOM", trackingFOM,
                        resic / errc, ";Track FOM ;Residual/Error", 100, 0.0,
                        1.0, 100, -5.0, 5.0);

        // Make the Per-Plane Histograms
        char planeName[256];
        sprintf(planeName, "FDCPulls_Plane%.2i", fdc_hit->wire->layer);

        Fill1DHistogram("TrackingPulls", planeName, "All Wire Pulls",
                        resi / err, "Residual/Error", 100, -5.0, 5.0);
        Fill1DHistogram("TrackingPulls", planeName, "All Cathode Pulls",
                        resic / errc, "Residual/Error", 100, -5.0, 5.0);
        Fill1DHistogram("TrackingPulls", planeName, "All Wire Residuals", resi,
                        "Residual", 100, -0.1, 0.1);
        Fill1DHistogram("TrackingPulls", planeName, "All Cathode Residuals",
                        resic, "Residual", 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Wire Residuals Vs Drift Time", tdrift, resi,
                        ";Drift Time;Residual", 170, -20.0, 150.0, 100, -0.1,
                        0.1);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Wire Pulls Vs Drift Time", tdrift, resi / err,
                        ";Drift Time;Residual/Error", 170, -20.0, 150.0, 100,
                        -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", planeName, "All Wire Pulls Vs. Phi",
                        phi, resi / err, ";#phi ;Residual/Error", 180, -180.0,
                        180.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", planeName, "All Wire Pulls Vs. Theta",
                        theta, resi / err, ";#theta ;Residual/Error", 50, 0.0,
                        25.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Wire Residuals Vs. Phi", phi, resi,
                        ";#phi ;Residual", 180, -180.0, 180.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Wire Residuals Vs. Theta", theta, resi,
                        ";#theta ;Residual", 50, 0.0, 25.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", planeName, "All Cathode Pulls Vs. Phi",
                        phi, resic / errc, ";#phi ;Residual/Error", 180, -180.0,
                        180.0, 100, -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Cathode Pulls Vs. Theta", theta, resic / errc,
                        ";#theta ;Residual/Error", 50, 0.0, 25.0, 100, -5.0,
                        5.0);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Cathode Residuals Vs. Phi", phi, resic,
                        ";#phi ;Residual", 180, -180.0, 180.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", planeName,
                        "All Cathode Residuals Vs. Theta", theta, resic,
                        ";#theta ;Residual", 50, 0.0, 25.0, 100, -0.1, 0.1);
        Fill2DHistogram("TrackingPulls", planeName, "Wire Pulls",
                        fdc_hit->wire->wire, resi / err,
                        ";Wire Number ;Residual/Error", 96, 0.5, 96.5, 100,
                        -5.0, 5.0);
        Fill2DHistogram("TrackingPulls", planeName, "Wire Residuals",
                        fdc_hit->wire->wire, resi, ";Wire Number ;Residual", 96,
                        0.5, 96.5, 100, -0.1, 0.1);
        if (fabs(resi / err) < 5.0) {
          Fill2DProfile("TrackingPulls", planeName, "2D Wire Hit Pulls",
                        fdc_hit->xy.X(), fdc_hit->xy.Y(), resi / err,
                        "Mean of Wire Pulls vs. PseudoHit XY", 100, -50., 50.,
                        100, -50., 50.);
        }
        if (fabs(resi) < 0.1) {
          Fill2DProfile("TrackingPulls", planeName, "2D Wire Hit Residuals",
                        fdc_hit->xy.X(), fdc_hit->xy.Y(), resi,
                        "Mean of Wire Residuals vs. PseudoHit XY", 100, -50.,
                        50., 100, -50., 50.);
          Fill2DProfile("TrackingPulls", planeName,
                        "2D Wire Hit Residuals Local", fdc_hit->w, fdc_hit->s,
                        resi,
                        "Mean of Wire Residuals vs. PseudoHit WS;Perpendicular "
                        "Distance to Wire; Distance Along the Wire",
                        100, -50., 50., 100, -50., 50.);
        }
        if (fabs(resic / errc) < 5.0) {
          Fill2DProfile("TrackingPulls", planeName, "2D Cathode Hit Pulls",
                        fdc_hit->xy.X(), fdc_hit->xy.Y(), resic / errc,
                        "Mean of Cathode Pulls vs. PseudoHit XY", 100, -50.,
                        50., 100, -50., 50.);
        }
        if (fabs(resic) < 0.1) {
          Fill2DProfile("TrackingPulls", planeName, "2D Cathode Hit Residuals",
                        fdc_hit->xy.X(), fdc_hit->xy.Y(), resic,
                        "Mean of Cathode Residuals vs. PseudoHit XY", 100, -50.,
                        50., 100, -50., 50.);
          Fill2DProfile(
              "TrackingPulls", planeName, "2D Cathode Hit Residuals Local",
              fdc_hit->w, fdc_hit->s, resic,
              "Mean of Cathode Residuals vs. PseudoHit WS;Perpendicular "
              "Distance to Wire; Distance Along the Wire",
              100, -50., 50., 100, -50., 50.);
        }
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
  return NOERROR;
}

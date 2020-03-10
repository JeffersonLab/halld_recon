// $Id$
//
//    File: JEventProcessor_MilleFieldOff.cc
// Created: Tue Jan 17 19:32:32 Local time zone must be set--see zic manual page 2017
// Creator: mstaib (on Linux egbert 2.6.32-642.6.2.el6.x86_64 x86_64)
//

#include "JEventProcessor_MilleFieldOff.h"
#include "CDC/DCDCTrackHit.h"
#include "CDC/DCDCWire.h"
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"
#include "TDirectory.h"
#include "TRACKING/DTrackTimeBased.h"
using namespace jana;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C" {
void InitPlugin(JApplication *app) {
  InitJANAPlugin(app);
  app->AddProcessor(new JEventProcessor_MilleFieldOff());
}
}  // "C"

JEventProcessor_MilleFieldOff::JEventProcessor_MilleFieldOff() {}

JEventProcessor_MilleFieldOff::~JEventProcessor_MilleFieldOff() {}

jerror_t JEventProcessor_MilleFieldOff::init(void) {
  // This is called once at program startup.
  int version = -1;
  gPARMS->SetDefaultParameter("MILLE:VERSION", version);
  if (version < 0) {
    milleWriter = new Mille("nofield_mille_out.mil");
  } else {
    milleWriter = new Mille(Form("mil/nofield_mille_out_v%02d.mil", version));
  }

  gDirectory->mkdir("AlignmentConstants");
  gDirectory->cd("AlignmentConstants");
  // We need the constants used for this iteration
  // Use a TProfile to avoid problems adding together multiple root files...
  HistCurrentConstantsCDC =
      new TProfile("CDCAlignmentConstants",
                   "Constants Used for CDC Alignment (In MILLEPEDE Order)",
                   16000, 0.5, 16000.5);

  gDirectory->cd("..");

  return NOERROR;
}

jerror_t JEventProcessor_MilleFieldOff::brun(JEventLoop *eventLoop,
                                             int32_t runnumber) {
  // Get the current set of constants and sve them in the histogram
  // This is called whenever the run number changes
  // Check for magnetic field
  DApplication *dapp =
      dynamic_cast<DApplication *>(eventLoop->GetJApplication());
  bool dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField *>(
                             dapp->GetBfield(runnumber)) != NULL);

  // This plugin is designed for field off data. If this is used for field on
  // data, Abort...
  if (!dIsNoFieldFlag) {
    jerr << " Plugin FDC_MilleFieldOff Must be run with zero magnetic field!!! "
            "Aborting "
         << endl;
    jerr << " Use -PBFIELD_TYPE=NoField " << endl;
    japp->Quit();
  }

  // Store the current values of the alignment constants
  JCalibration *jcalib = eventLoop->GetJCalibration();
  vector<map<string, double> > vals;
  if (jcalib->Get("CDC/global_alignment", vals) == false) {
    map<string, double> &row = vals[0];
    // Get the offsets from the calibration database
    HistCurrentConstantsCDC->Fill(1, row["dX"]);
    HistCurrentConstantsCDC->Fill(2, row["dY"]);
    HistCurrentConstantsCDC->Fill(3, row["dZ"]);
    HistCurrentConstantsCDC->Fill(4, row["dPhiX"]);
    HistCurrentConstantsCDC->Fill(5, row["dPhiY"]);
    HistCurrentConstantsCDC->Fill(6, row["dPhiZ"]);
  }

  if (jcalib->Get("CDC/wire_alignment", vals) == false) {
    for (unsigned int i = 0; i < vals.size(); i++) {
      map<string, double> &row = vals[i];
      // Get the offsets from the calibration database
      HistCurrentConstantsCDC->Fill(1000 + (i * 4 + 1), row["dxu"]);
      HistCurrentConstantsCDC->Fill(1000 + (i * 4 + 2), row["dyu"]);
      HistCurrentConstantsCDC->Fill(1000 + (i * 4 + 3), row["dxd"]);
      HistCurrentConstantsCDC->Fill(1000 + (i * 4 + 4), row["dyd"]);
    }
  }

  return NOERROR;
}

jerror_t JEventProcessor_MilleFieldOff::evnt(JEventLoop *loop,
                                             uint64_t eventnumber) {
  int straw_offset[29] = {0,    0,    42,   84,   138,  192,  258,  324,
                          404,  484,  577,  670,  776,  882,  1005, 1128,
                          1263, 1398, 1544, 1690, 1848, 2006, 2176, 2346,
                          2528, 2710, 2907, 3104, 3313};
  // Loop over the tracks, get the tracking pulls, and fill some histograms.
  // Easy peasy
  vector<const DTrackTimeBased *> trackVector;
  loop->Get(trackVector, "StraightLine");

  for (size_t i = 0; i < trackVector.size(); ++i) {
    const DTrackTimeBased *track = trackVector[i];

    // Some quality cuts for the tracks we will use
    // Keep this minimal for now and investigate later
    double trackingFOMCut = 1.0e-20;
    if (TMath::Prob(track->chisq, track->Ndof) < trackingFOMCut) continue;
    if (track->Ndof < 20) continue;
    if (!track->IsSmoothed) continue;

    vector<DTrackFitter::pull_t> pulls = track->pulls;

    // Checks the pull quality.
    bool contains_bad_pulls = false;
    for (size_t iPull = 0; iPull < pulls.size(); ++iPull) {
      float resi = pulls[iPull].resi;  // residual of measurement
      float err = pulls[iPull].err;
      float resic = pulls[iPull].resic;  // residual for FDC cathode
      float errc = pulls[iPull].errc;
      if (resi != resi || err != err || resic != resic || errc != errc ||
          !isfinite(resi) || !isfinite(resic)) {
        contains_bad_pulls = true;
        break;
      }
      // length unit: (cm)
      if (fabs(resi) > 10.0 || err > 10.0 || fabs(resic) > 10.0 ||
          errc > 10.0) {
        contains_bad_pulls = true;
        break;
      }
    }
    if (contains_bad_pulls) continue;

    japp->RootWriteLock();  // Just use the root lock as a temporary
    for (size_t iPull = 0; iPull < pulls.size(); ++iPull) {
      float resi = pulls[iPull].resi;  // residual of measurement
      float err = pulls[iPull].err;    // estimated error of measurement
      const DCDCTrackHit *cdc_hit = pulls[iPull].cdc_hit;
      const DFDCPseudo *fdc_hit = pulls[iPull].fdc_hit;
      float resic = pulls[iPull].resic;  // residual for FDC cathode measuremtns
      float errc = pulls[iPull].errc;

      vector<double> trackDerivatives = pulls[iPull].trackDerivatives;

      if (fdc_hit != nullptr && fdc_hit->status == 6) {
        // Add fdc hit
        DFDCPseudo *thisHit = const_cast<DFDCPseudo *>(fdc_hit);

        vector<double> pseudoAlignmentDerivatives =
            thisHit->GetFDCPseudoAlignmentDerivatives();
        vector<double> fdcStripPitchDerivatives =
            thisHit->GetFDCStripPitchDerivatives();

        // hit->wire->layer: 1-based [1-24]
        int label_layer_offset = 100000 + thisHit->wire->layer * 1000;

        // For wire measurement.
        const int NLC = 4;
        const int NGL_W = 6;
        float derLc_W[NLC];
        float derGl_W[NGL_W];
        int label_W[NGL_W];

        derLc_W[0] = trackDerivatives[FDCTrackD::dDOCAW_dx];
        derLc_W[1] = trackDerivatives[FDCTrackD::dDOCAW_dy];
        derLc_W[2] = trackDerivatives[FDCTrackD::dDOCAW_dtx];
        derLc_W[3] = trackDerivatives[FDCTrackD::dDOCAW_dty];

        derGl_W[0] = trackDerivatives[FDCTrackD::dDOCAW_dDeltaX];
        derGl_W[1] = trackDerivatives[FDCTrackD::dDOCAW_dDeltaPhiX];
        derGl_W[2] = trackDerivatives[FDCTrackD::dDOCAW_dDeltaPhiY];
        derGl_W[3] = trackDerivatives[FDCTrackD::dDOCAW_dDeltaPhiZ];
        derGl_W[4] = trackDerivatives[FDCTrackD::dDOCAW_dDeltaZ];
        derGl_W[5] = -trackDerivatives[FDCTrackD::dW_dt0];

        label_W[0] = label_layer_offset + 1;
        label_W[1] = label_layer_offset + 2;
        label_W[2] = label_layer_offset + 3;
        label_W[3] = label_layer_offset + 4;
        label_W[4] = label_layer_offset + 5;
        // label_W[5] = label_layer_offset + 900 +
        // pulls[iPull].fdc_hit->wire->wire;
        label_W[5] = label_layer_offset + 999;

        milleWriter->mille(NLC, derLc_W, NGL_W, derGl_W, label_W, resi, err);

        // For cathode measurement.
        const int NGL_C = 20;
        float derLc_C[NLC];
        float derGl_C[NGL_C];
        int label_C[NGL_C];

        derLc_C[0] = trackDerivatives[FDCTrackD::dDOCAC_dx];
        derLc_C[1] = trackDerivatives[FDCTrackD::dDOCAC_dy];
        derLc_C[2] = trackDerivatives[FDCTrackD::dDOCAC_dtx];
        derLc_C[3] = trackDerivatives[FDCTrackD::dDOCAC_dty];

        derGl_C[0] = -1.0;
        derGl_C[1] = trackDerivatives[FDCTrackD::dDOCAC_dDeltaX];
        derGl_C[2] = trackDerivatives[FDCTrackD::dDOCAC_dDeltaPhiX];
        derGl_C[3] = trackDerivatives[FDCTrackD::dDOCAC_dDeltaPhiY];
        derGl_C[4] = trackDerivatives[FDCTrackD::dDOCAC_dDeltaPhiZ];
        derGl_C[5] = trackDerivatives[FDCTrackD::dDOCAC_dDeltaZ];

        label_C[0] = label_layer_offset + 100;
        label_C[1] = label_layer_offset + 1;
        label_C[2] = label_layer_offset + 2;
        label_C[3] = label_layer_offset + 3;
        label_C[4] = label_layer_offset + 4;
        label_C[5] = label_layer_offset + 5;

        // Cathode U and V offsets
        derGl_C[6] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU];
        derGl_C[7] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV];
        derGl_C[8] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaPhiU];
        derGl_C[9] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaPhiV];

        label_C[6] = label_layer_offset + 101;
        label_C[7] = label_layer_offset + 102;
        label_C[8] = label_layer_offset + 103;
        label_C[9] = label_layer_offset + 104;

        // Strip Pitch Calibration
        derGl_C[10] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU] *
                      fdcStripPitchDerivatives[0];
        derGl_C[11] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU] *
                      fdcStripPitchDerivatives[1];
        derGl_C[12] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU] *
                      fdcStripPitchDerivatives[2];
        derGl_C[13] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU] *
                      fdcStripPitchDerivatives[3];
        derGl_C[14] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaU] *
                      fdcStripPitchDerivatives[4];
        derGl_C[15] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV] *
                      fdcStripPitchDerivatives[5];
        derGl_C[16] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV] *
                      fdcStripPitchDerivatives[6];
        derGl_C[17] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV] *
                      fdcStripPitchDerivatives[7];
        derGl_C[18] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV] *
                      fdcStripPitchDerivatives[8];
        derGl_C[19] = -pseudoAlignmentDerivatives[FDCPseudoD::dSddeltaV] *
                      fdcStripPitchDerivatives[9];

        label_C[10] = label_layer_offset + 200;
        label_C[11] = label_layer_offset + 201;
        label_C[12] = label_layer_offset + 202;
        label_C[13] = label_layer_offset + 203;
        label_C[14] = label_layer_offset + 204;
        label_C[15] = label_layer_offset + 205;
        label_C[16] = label_layer_offset + 206;
        label_C[17] = label_layer_offset + 207;
        label_C[18] = label_layer_offset + 208;
        label_C[19] = label_layer_offset + 209;

        milleWriter->mille(NLC, derLc_C, NGL_C, derGl_C, label_C, resic, errc);
      }

      if (cdc_hit != nullptr) {
        const DCDCWire *constWire = cdc_hit->wire;
        DCDCWire *thisWire = const_cast<DCDCWire *>(constWire);

        vector<double> wireDerivatives = thisWire->GetCDCWireDerivatives();

        const int NLC = 4;
        const int NGL = 10;
        float derLc[NLC];
        float derGl[NGL];
        int label[NGL];

        derLc[0] = trackDerivatives[CDCTrackD::dDOCAdS0];
        derLc[1] = trackDerivatives[CDCTrackD::dDOCAdS1];
        derLc[2] = trackDerivatives[CDCTrackD::dDOCAdS2];
        derLc[3] = trackDerivatives[CDCTrackD::dDOCAdS3];

        derGl[0] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaX] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaX] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaX];

        derGl[1] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaY] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaY] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaY];

        derGl[2] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaZ] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaZ] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaZ];

        derGl[3] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaPhiX] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaPhiX] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaPhiX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaPhiX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaPhiX] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaPhiX];

        derGl[4] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaPhiY] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaPhiY] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaPhiY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaPhiY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaPhiY] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaPhiY];

        derGl[5] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaPhiZ] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaPhiZ] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaPhiZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaPhiZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaPhiZ] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaPhiZ];

        derGl[6] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaXu] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaXu] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaXu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaXu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaXu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaXu];

        derGl[7] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaYu] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaYu] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaYu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaYu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaYu] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaYu];

        derGl[8] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaXd] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaXd] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaXd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaXd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaXd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaXd];

        derGl[9] = trackDerivatives[CDCTrackD::dDOCAdOriginX] *
                       wireDerivatives[CDCWireD::dOriginXddeltaYd] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginY] *
                       wireDerivatives[CDCWireD::dOriginYddeltaYd] +
                   trackDerivatives[CDCTrackD::dDOCAdOriginZ] *
                       wireDerivatives[CDCWireD::dOriginZddeltaYd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirX] *
                       wireDerivatives[CDCWireD::dDirXddeltaYd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirY] *
                       wireDerivatives[CDCWireD::dDirYddeltaYd] +
                   trackDerivatives[CDCTrackD::dDOCAdDirZ] *
                       wireDerivatives[CDCWireD::dDirZddeltaYd];

        label[0] = 1;
        label[1] = 2;
        label[2] = 3;
        label[3] = 4;
        label[4] = 5;
        label[5] = 6;
        label[6] = 1000 +
                   (straw_offset[thisWire->ring] + (thisWire->straw - 1)) * 4 +
                   1;
        label[7] = 1000 +
                   (straw_offset[thisWire->ring] + (thisWire->straw - 1)) * 4 +
                   2;
        label[8] = 1000 +
                   (straw_offset[thisWire->ring] + (thisWire->straw - 1)) * 4 +
                   3;
        label[9] = 1000 +
                   (straw_offset[thisWire->ring] + (thisWire->straw - 1)) * 4 +
                   4;

        milleWriter->mille(NLC, derLc, NGL, derGl, label, resi, err);
      }
    }
    milleWriter->end();
    japp->RootUnLock();
  }

  return NOERROR;
}

jerror_t JEventProcessor_MilleFieldOff::erun(void) {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

jerror_t JEventProcessor_MilleFieldOff::fini(void) {
  // Called before program exit after event processing is finished.
  delete milleWriter;
  return NOERROR;
}

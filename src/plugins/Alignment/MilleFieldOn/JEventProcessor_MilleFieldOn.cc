// $Id$
//
//    File: JEventProcessor_MilleFieldOn.cc
// Created: Tue Jan 17 19:32:32
// Creator: mstaib (on Linux egbert 2.6.32-642.6.2.el6.x86_64 x86_64)
//

#include "JEventProcessor_MilleFieldOn.h"
#include "CDC/DCDCTrackHit.h"
#include "CDC/DCDCWire.h"
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"
#include "PID/DChargedTrack.h"
#include "TDirectory.h"
#include "TRACKING/DTrackTimeBased.h"

#include <JANA/JFactoryT.h>
#include <DANA/DEvent.h>

extern "C" {
void InitPlugin(JApplication *app) {
  InitJANAPlugin(app);
  app->Add(new JEventProcessor_MilleFieldOn());
}
}  // "C"

JEventProcessor_MilleFieldOn::JEventProcessor_MilleFieldOn() {
    SetTypeName("JEventProcessor_MilleFieldOn");
}

JEventProcessor_MilleFieldOn::~JEventProcessor_MilleFieldOn() {}

void JEventProcessor_MilleFieldOn::Init() {
  // This is called once at program startup.
  auto app = GetApplication();
  string output_filename;
  app->GetParameter("OUTPUT_FILENAME", output_filename);
  int ext_pos = output_filename.rfind(".root");
  if (ext_pos != (int)output_filename.size() - 5) {
    jerr << "[MilleFieldOn] Invalid output filename." << endl;
    japp->Quit();
  }
  output_filename.replace(ext_pos, 5, ".mil");
  milleWriter = new Mille(output_filename.data());
}

void JEventProcessor_MilleFieldOn::BeginRun(const std::shared_ptr<const JEvent> &event, int32_t runnumber) {
  // This is called whenever the run number changes
  // Check for magnetic field
  bool dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField *>(DEvent::GetBfield(event)) != nullptr);

  // This plugin is designed for field on data. If this is used for field off
  // data, Abort...
  if (dIsNoFieldFlag) {
    jerr << " ********No Field******** Plugin MilleFieldOn is for use with "
            "magnetic field!!! Aborting "
         << endl;
    japp->Quit();
  }
}

void JEventProcessor_MilleFieldOn::Process(const std::shared_ptr<const JEvent> &event, uint64_t eventnumber) {

  int straw_offset[29] = {0,    0,    42,   84,   138,  192,  258,  324,
                          404,  484,  577,  670,  776,  882,  1005, 1128,
                          1263, 1398, 1544, 1690, 1848, 2006, 2176, 2346,
                          2528, 2710, 2907, 3104, 3313};
  vector<const DChargedTrack *> trackVector;
  event->Get(trackVector);

  for (size_t i = 0; i < trackVector.size(); ++i) {
    // TODO: Should be changed to use PID FOM when ready
    const DChargedTrackHypothesis *bestHypothesis =
        trackVector[i]->Get_BestTrackingFOM();

    if (bestHypothesis == nullptr) continue;

    auto track = bestHypothesis->Get_TrackTimeBased();

    // Some quality cuts for the tracks we will use
    // Keep this minimal for now and investigate later
    if (TMath::Prob(track->chisq, track->Ndof) < 0.001) continue;

    // Get the pulls vector from the track
    if (!track->IsSmoothed) continue;

    vector<DTrackFitter::pull_t> pulls = track->pulls;
    // Determine TrackType and check for nan
    bool isCDCOnly = true;
    bool anyNaN = false;
    int pullsNDF = -5;
    for (size_t iPull = 0; iPull < pulls.size(); ++iPull) {
      const DCDCTrackHit *cdc_hit = pulls[iPull].cdc_hit;
      float err = pulls[iPull].err;
      float errc = pulls[iPull].errc;
      if (cdc_hit == nullptr) {
        pullsNDF += 2;
        isCDCOnly = false;
      } else {
        pullsNDF++;
      }
      if (err != err || errc != errc) {
        anyNaN = true;
        break;
      }
    }
    if (anyNaN) continue;
    if (pullsNDF != track->Ndof) continue;
    if (track->Ndof < (isCDCOnly ? 10 : 22)) continue;

    GetLockService(event)->RootWriteLock();  // Just use the root lock as a temporary
    for (size_t iPull = 0; iPull < pulls.size(); ++iPull) {
      float resi = pulls[iPull].resi;  // residual of measurement
      float err = pulls[iPull].err;    // estimated error of measurement
      const DCDCTrackHit *cdc_hit = pulls[iPull].cdc_hit;
      const DFDCPseudo *fdc_hit = pulls[iPull].fdc_hit;
      float resic = pulls[iPull].resic;  // residual for FDC cathode measurement
      float errc = pulls[iPull].errc;

      vector<double> der = pulls[iPull].trackDerivatives;

      if (fdc_hit != nullptr && fdc_hit->status == 6) {
        // Add fdc hit
        DFDCPseudo *hit = const_cast<DFDCPseudo *>(fdc_hit);

        vector<double> pseudo_der = hit->GetFDCPseudoAlignmentDerivatives();
        vector<double> strip_der = hit->GetFDCStripPitchDerivatives();

        int label_layer_offset = 100000 + hit->wire->layer * 1000;

        // For wire measurment.
        const int NLC = 4;
        const int NGL_W = 6;
        float derLc_W[NLC];
        float derGl_W[NGL_W];
        int label_W[NGL_W];

        derLc_W[0] = der[FDCTrackD::dDOCAW_dx];
        derLc_W[1] = der[FDCTrackD::dDOCAW_dy];
        derLc_W[2] = der[FDCTrackD::dDOCAW_dtx];
        derLc_W[3] = der[FDCTrackD::dDOCAW_dty];

        derGl_W[0] = der[FDCTrackD::dDOCAW_dDeltaX];
        derGl_W[1] = der[FDCTrackD::dDOCAW_dDeltaPhiX];
        derGl_W[2] = der[FDCTrackD::dDOCAW_dDeltaPhiY];
        derGl_W[3] = der[FDCTrackD::dDOCAW_dDeltaPhiZ];
        derGl_W[4] = der[FDCTrackD::dDOCAW_dDeltaZ];
        derGl_W[5] = -der[FDCTrackD::dW_dt0];

        label_W[0] = label_layer_offset + 1;
        label_W[1] = label_layer_offset + 2;
        label_W[2] = label_layer_offset + 3;
        label_W[3] = label_layer_offset + 4;
        label_W[4] = label_layer_offset + 5;
        if (fdc_hit->wire->wire <= 48) {
          label_W[5] = label_layer_offset + 997;
        } else {
          label_W[5] = label_layer_offset + 998;
        }

        milleWriter->mille(NLC, derLc_W, NGL_W, derGl_W, label_W, resi, err);

        // For cathode measurement.
        const int NGL_C = 20;
        float derLc_C[NLC];
        float derGl_C[NGL_C];
        int label_C[NGL_C];

        derLc_C[0] = der[FDCTrackD::dDOCAC_dx];
        derLc_C[1] = der[FDCTrackD::dDOCAC_dy];
        derLc_C[2] = der[FDCTrackD::dDOCAC_dtx];
        derLc_C[3] = der[FDCTrackD::dDOCAC_dty];

        derGl_C[0] = -1.0;
        derGl_C[1] = der[FDCTrackD::dDOCAC_dDeltaX];
        derGl_C[2] = der[FDCTrackD::dDOCAC_dDeltaPhiX];
        derGl_C[3] = der[FDCTrackD::dDOCAC_dDeltaPhiY];
        derGl_C[4] = der[FDCTrackD::dDOCAC_dDeltaPhiZ];
        derGl_C[5] = der[FDCTrackD::dDOCAC_dDeltaZ];

        label_C[0] = label_layer_offset + 100;
        label_C[1] = label_layer_offset + 1;
        label_C[2] = label_layer_offset + 2;
        label_C[3] = label_layer_offset + 3;
        label_C[4] = label_layer_offset + 4;
        label_C[5] = label_layer_offset + 5;

        // Cathode U and V offsets
        derGl_C[6] = -pseudo_der[FDCPseudoD::dSddeltaU];
        derGl_C[7] = -pseudo_der[FDCPseudoD::dSddeltaV];
        derGl_C[8] = -pseudo_der[FDCPseudoD::dSddeltaPhiU];
        derGl_C[9] = -pseudo_der[FDCPseudoD::dSddeltaPhiV];

        label_C[6] = label_layer_offset + 101;
        label_C[7] = label_layer_offset + 102;
        label_C[8] = label_layer_offset + 103;
        label_C[9] = label_layer_offset + 104;

        // Strip Pitch Calibration
        derGl_C[10] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[0];
        derGl_C[11] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[1];
        derGl_C[12] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[2];
        derGl_C[13] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[3];
        derGl_C[14] = -pseudo_der[FDCPseudoD::dSddeltaU] * strip_der[4];
        derGl_C[15] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[5];
        derGl_C[16] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[6];
        derGl_C[17] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[7];
        derGl_C[18] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[8];
        derGl_C[19] = -pseudo_der[FDCPseudoD::dSddeltaV] * strip_der[9];

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
        DCDCWire *wire = const_cast<DCDCWire *>(constWire);

        vector<double> wire_der = wire->GetCDCWireDerivatives();

        const int NLC = 5;
        const int NGL = 10;  // 11 if t0 is ON.
        float derLc[NLC];
        float derGl[NGL];
        int label[NGL];

        derLc[0] = der[CDCTrackD::dDOCAdS0];
        derLc[1] = der[CDCTrackD::dDOCAdS1];
        derLc[2] = der[CDCTrackD::dDOCAdS2];
        derLc[3] = der[CDCTrackD::dDOCAdS3];
        derLc[4] = der[CDCTrackD::dDOCAdS4];

        if (isCDCOnly) {  // Global shifts will not affect residuals
          derGl[0] = 0.0;
          derGl[1] = 0.0;
          derGl[2] = 0.0;
          derGl[3] = 0.0;
          derGl[4] = 0.0;
          derGl[5] = 0.0;

          label[0] = 1;
          label[1] = 2;
          label[2] = 3;
          label[3] = 4;
          label[4] = 5;
          label[5] = 6;
        } else {
          derGl[0] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaX] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaX] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaX];

          derGl[1] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaY] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaY] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaY];

          derGl[2] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaZ] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaZ] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaZ];

          derGl[3] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaPhiX] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiX] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiX];

          derGl[4] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaPhiY] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiY] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiY];

          derGl[5] =
              der[CDCTrackD::dDOCAdOriginX] *
                  wire_der[CDCWireD::dOriginXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginY] *
                  wire_der[CDCWireD::dOriginYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdOriginZ] *
                  wire_der[CDCWireD::dOriginZddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaPhiZ] +
              der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaPhiZ];

          label[0] = 1;
          label[1] = 2;
          label[2] = 3;
          label[3] = 4;
          label[4] = 5;
          label[5] = 6;
        }
        derGl[6] =
            der[CDCTrackD::dDOCAdOriginX] *
                wire_der[CDCWireD::dOriginXddeltaXu] +
            der[CDCTrackD::dDOCAdOriginY] *
                wire_der[CDCWireD::dOriginYddeltaXu] +
            der[CDCTrackD::dDOCAdOriginZ] *
                wire_der[CDCWireD::dOriginZddeltaXu] +
            der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXu] +
            der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXu] +
            der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXu];

        derGl[7] =
            der[CDCTrackD::dDOCAdOriginX] *
                wire_der[CDCWireD::dOriginXddeltaYu] +
            der[CDCTrackD::dDOCAdOriginY] *
                wire_der[CDCWireD::dOriginYddeltaYu] +
            der[CDCTrackD::dDOCAdOriginZ] *
                wire_der[CDCWireD::dOriginZddeltaYu] +
            der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYu] +
            der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYu] +
            der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYu];

        derGl[8] =
            der[CDCTrackD::dDOCAdOriginX] *
                wire_der[CDCWireD::dOriginXddeltaXd] +
            der[CDCTrackD::dDOCAdOriginY] *
                wire_der[CDCWireD::dOriginYddeltaXd] +
            der[CDCTrackD::dDOCAdOriginZ] *
                wire_der[CDCWireD::dOriginZddeltaXd] +
            der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaXd] +
            der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaXd] +
            der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaXd];

        derGl[9] =
            der[CDCTrackD::dDOCAdOriginX] *
                wire_der[CDCWireD::dOriginXddeltaYd] +
            der[CDCTrackD::dDOCAdOriginY] *
                wire_der[CDCWireD::dOriginYddeltaYd] +
            der[CDCTrackD::dDOCAdOriginZ] *
                wire_der[CDCWireD::dOriginZddeltaYd] +
            der[CDCTrackD::dDOCAdDirX] * wire_der[CDCWireD::dDirXddeltaYd] +
            der[CDCTrackD::dDOCAdDirY] * wire_der[CDCWireD::dDirYddeltaYd] +
            der[CDCTrackD::dDOCAdDirZ] * wire_der[CDCWireD::dDirZddeltaYd];

        label[6] = 1001 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
        label[7] = 1002 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
        label[8] = 1003 + (straw_offset[wire->ring] + wire->straw - 1) * 4;
        label[9] = 1004 + (straw_offset[wire->ring] + wire->straw - 1) * 4;

        // derGl[10] = -der[CDCTrackD::dDdt0];
        // label[10] = 16000 + straw_offset[wire->ring] + wire->straw;

        milleWriter->mille(NLC, derLc, NGL, derGl, label, resi, err);
      }
    }
    milleWriter->end();
    GetLockService(event)->RootUnLock();
  }
}

void JEventProcessor_MilleFieldOn::EndRun() {
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

void JEventProcessor_MilleFieldOn::Finish() {
  // Called before program exit after event processing is finished.
  delete milleWriter;
}

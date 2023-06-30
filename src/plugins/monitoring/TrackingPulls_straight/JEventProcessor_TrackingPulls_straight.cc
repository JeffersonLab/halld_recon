// $Id$
//
//    File: JEventProcessor_TrackingPulls_straight.h
// Created: Wed Jan 22 09:28:45 EST 2020
// Creator: keigo (on Linux ifarm1801 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_TrackingPulls_straight.h"
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
  tree_->Branch("eventnumber", &eventnumber_, "eventnumber/I");
  tree_->Branch("track_index", &track_index_, "track_index/I");
  tree_->Branch("chi2", &chi2_, "chi2/D");
  tree_->Branch("ndf", &ndf_, "ndf/I");
  tree_->Branch("ncdchits", &ncdchits_, "ncdchits/I");
  tree_->Branch("nfdchits", &nfdchits_, "nfdchits/I");
  tree_->Branch("mom", &mom_, "mom/D");
  tree_->Branch("phi", &phi_, "phi/D");
  tree_->Branch("theta", &theta_, "theta/D");
  tree_->Branch("pos_x", &pos_x_, "pos_x/D");
  tree_->Branch("pos_y", &pos_y_, "pos_y/D");
  tree_->Branch("pos_z", &pos_z_, "pos_z/D");
  tree_->Branch("smoothed", &smoothed_, "smoothed/I");
  tree_->Branch("any_nan", &any_nan_, "any_nan/I");
  tree_->Branch("cdc_ring_multi_hits", &cdc_ring_multi_hits_,
                "cdc_ring_multi_hits/I");
  tree_->Branch("fdc_resi", fdc_resi_, Form("fdc_resi[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_resic", fdc_resic_,
                Form("fdc_resic[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_err", fdc_err_, Form("fdc_err[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_errc", fdc_errc_, Form("fdc_errc[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_x", fdc_x_, Form("fdc_x[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_y", fdc_y_, Form("fdc_y[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_z", fdc_z_, Form("fdc_z[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_w", fdc_w_, Form("fdc_w[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_s", fdc_s_, Form("fdc_s[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_d", fdc_d_, Form("fdc_d[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_tdrift", fdc_tdrift_,
                Form("fdc_tdrift[%d]/D", kNumFdcPlanes));
  tree_->Branch("fdc_wire", fdc_wire_, Form("fdc_wire[%d]/I", kNumFdcPlanes));
  tree_->Branch("fdc_left_right", fdc_left_right_,
                Form("fdc_left_right[%d]/I", kNumFdcPlanes));
  tree_->Branch("cdc_resi", cdc_resi_, Form("cdc_resi[%d]/D", kNumCdcRings));
  tree_->Branch("cdc_err", cdc_err_, Form("cdc_err[%d]/D", kNumCdcRings));
  tree_->Branch("cdc_z", cdc_z_, Form("cdc_z[%d]/D", kNumCdcRings));
  tree_->Branch("cdc_tdrift", cdc_tdrift_,
                Form("cdc_tdrift[%d]/D", kNumCdcRings));
  tree_->Branch("cdc_straw", cdc_straw_, Form("cdc_straw[%d]/I", kNumCdcRings));
  tree_->Branch("cdc_left_right", cdc_left_right_,
                Form("cdc_left_right[%d]/I", kNumCdcRings));
  tree_->Branch("cdc_phi_intersect", cdc_phi_intersect_,
                Form("cdc_phi_intersect[%d]/D", kNumCdcRings));

  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::brun(JEventLoop *eventLoop,
                                                      int32_t runnumber) {
  // This is called whenever the run number changes
  return NOERROR;
}

jerror_t JEventProcessor_TrackingPulls_straight::evnt(JEventLoop *loop,
                                                      uint64_t eventnumber) {
  const DTrigger *locTrigger = NULL;
  loop->GetSingle(locTrigger);
  if (locTrigger->Get_L1FrontPanelTriggerBits() != 0) return NOERROR;

  vector<const DTrackTimeBased *> trackVector;
  loop->Get(trackVector, "StraightLine");

  for (size_t i = 0; i < trackVector.size(); i++) {
    const DTrackTimeBased *track = trackVector[i];

    // Initializes TTree variables.
    for (int j = 0; j < kNumFdcPlanes; ++j) {
      fdc_resi_[j] = -999.9;
      fdc_resic_[j] = -999.9;
      fdc_err_[j] = -999.9;
      fdc_errc_[j] = -999.9;
      fdc_x_[j] = -999.9;
      fdc_y_[j] = -999.9;
      fdc_z_[j] = -999.9;
      fdc_w_[j] = -999.9;
      fdc_s_[j] = -999.9;
      fdc_d_[j] = -999.9;
      fdc_tdrift_[j] = -999.9;
      fdc_wire_[j] = -999;
      fdc_left_right_[j] = -999;
    }
    for (int j = 0; j < kNumCdcRings; ++j) {
      cdc_resi_[j] = -999.9;
      cdc_err_[j] = -999.9;
      cdc_z_[j] = -999.9;
      cdc_tdrift_[j] = -999.9;
      cdc_straw_[j] = -999;
      cdc_left_right_[j] = -999;
      cdc_phi_intersect_[j] = -999.9;
    }
    eventnumber_ = (int)eventnumber;
    track_index_ = (int)i;
    chi2_ = track->chisq;
    ndf_ = track->Ndof;
    ncdchits_ = track->measured_cdc_hits_on_track;
    nfdchits_ = track->measured_fdc_hits_on_track;
    mom_ = track->momentum().Mag();
    phi_ = track->momentum().Phi() * TMath::RadToDeg();
    theta_ = track->momentum().Theta() * TMath::RadToDeg();
    pos_x_ = track->position().X();
    pos_y_ = track->position().Y();
    pos_z_ = track->position().Z();
    smoothed_ = (track->IsSmoothed ? 1 : 0);

    vector<DTrackFitter::pull_t> pulls = track->pulls;

    // Check for NaNs
    any_nan_ = false;
    for (size_t iPull = 0; iPull < pulls.size(); iPull++) {
      double err = pulls[iPull].err;
      double errc = pulls[iPull].errc;
      if (err != err || errc != errc) {
        any_nan_ = true;
        break;
      }
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

      // Fill some detector specific info
      // Fill them in order = super-hacked
      static int nextPlane = 1;
      static int nextRing = 1;

      if (fdc_hit != nullptr && fdc_hit->wire->layer <= nextPlane) {
        if (fdc_hit->wire->layer == nextPlane) nextPlane++;

        fdc_resi_[fdc_hit->wire->layer - 1] = resi;
        fdc_resic_[fdc_hit->wire->layer - 1] = resic;
        fdc_err_[fdc_hit->wire->layer - 1] = err;
        fdc_errc_[fdc_hit->wire->layer - 1] = errc;
        fdc_x_[fdc_hit->wire->layer - 1] = fdc_hit->xy.X();
        fdc_y_[fdc_hit->wire->layer - 1] = fdc_hit->xy.Y();
        fdc_z_[fdc_hit->wire->layer - 1] = pulls[iPull].z;
        fdc_w_[fdc_hit->wire->layer - 1] = fdc_hit->w;
        fdc_s_[fdc_hit->wire->layer - 1] = fdc_hit->s;
        fdc_d_[fdc_hit->wire->layer - 1] = pulls[iPull].d;
        fdc_tdrift_[fdc_hit->wire->layer - 1] = tdrift;
        fdc_wire_[fdc_hit->wire->layer - 1] = fdc_hit->wire->wire;
        fdc_left_right_[fdc_hit->wire->layer - 1] = pulls[iPull].left_right;
      }

      // Once we are done with the FDC, move on to the CDC.
      if (cdc_hit != nullptr && cdc_hit->wire->ring <= nextRing &&
          (nextPlane == 25 || eventnumber > 1000)) {
        if (cdc_hit->wire->ring == nextRing) nextRing++;

        cdc_resi_[cdc_hit->wire->ring - 1] = resi;
        cdc_err_[cdc_hit->wire->ring - 1] = err;
        cdc_z_[cdc_hit->wire->ring - 1] = pulls[iPull].z;
        cdc_tdrift_[cdc_hit->wire->ring - 1] = tdrift;
        cdc_straw_[cdc_hit->wire->ring - 1] = cdc_hit->wire->straw;
        cdc_left_right_[cdc_hit->wire->ring - 1] = pulls[iPull].left_right;
        cdc_phi_intersect_[cdc_hit->wire->ring - 1] =
            (cdc_hit->wire->origin + (z - 92.0) * cdc_hit->wire->udir).Phi() *
            TMath::RadToDeg();
      }
    }
    japp->RootWriteLock();
    tree_->Fill();
    japp->RootUnLock();
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

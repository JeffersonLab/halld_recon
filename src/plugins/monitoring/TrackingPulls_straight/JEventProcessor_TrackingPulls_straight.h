// $Id$
//
//    File: JEventProcessor_TrackingPulls_straight.h
// Created: Wed Jan 22 09:28:45 EST 2020
// Creator: keigo (on Linux ifarm1801 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TrackingPulls_straight_
#define _JEventProcessor_TrackingPulls_straight_

#include <JANA/JEventProcessor.h>
#include <TTree.h>

class JEventProcessor_TrackingPulls_straight : public jana::JEventProcessor {
 public:
  JEventProcessor_TrackingPulls_straight();
  ~JEventProcessor_TrackingPulls_straight();
  const char *className(void) {
    return "JEventProcessor_TrackingPulls_straight";
  }

  static const int kNumFdcPlanes = 24;
  static const int kNumCdcRings = 28;

 private:
  jerror_t init(void);  ///< Called once at program start.
  jerror_t brun(
      jana::JEventLoop *eventLoop,
      int32_t runnumber);  ///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop,
                uint64_t eventnumber);  ///< Called every event.
  jerror_t erun(void);  ///< Called everytime run number changes, provided brun
                        ///< has been called.
  jerror_t fini(void);  ///< Called after last event of last event source has
                        ///< been processed.

  TTree *tree_;
  int eventnumber_;
  int track_index_;
  double chi2_;
  int ndf_;
  int ncdchits_;
  int nfdchits_;
  double mom_;
  double phi_;
  double theta_;
  double pos_x_;
  double pos_y_;
  double pos_z_;
  int smoothed_;  // 1: smoothed, 0: not smoothed
  int any_nan_;   // 1: anyNaN = true, 0: anyNaN = false
  int cdc_ring_multi_hits_;
  double fdc_resi_[kNumFdcPlanes];
  double fdc_resic_[kNumFdcPlanes];
  double fdc_err_[kNumFdcPlanes];
  double fdc_errc_[kNumFdcPlanes];
  double fdc_x_[kNumFdcPlanes];
  double fdc_y_[kNumFdcPlanes];
  double fdc_z_[kNumFdcPlanes];
  double fdc_w_[kNumFdcPlanes];
  double fdc_s_[kNumFdcPlanes];
  double fdc_d_[kNumFdcPlanes];
  double fdc_tdrift_[kNumFdcPlanes];
  int fdc_wire_[kNumFdcPlanes];
  int fdc_left_right_[kNumFdcPlanes];  // -1: left, +1: right
  double cdc_resi_[kNumCdcRings];
  double cdc_err_[kNumCdcRings];
  double cdc_z_[kNumCdcRings];
  double cdc_tdrift_[kNumCdcRings];
  int cdc_straw_[kNumCdcRings];
  int cdc_left_right_[kNumCdcRings];  // -1: left, +1: right
  double cdc_phi_intersect_[kNumCdcRings];
};

#endif  // _JEventProcessor_TrackingPulls_straight_

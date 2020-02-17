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
  double chi2_;
  int ndf_;
  double phi_;
  double theta_;
  double pos_x_;
  double pos_y_;
  double pos_z_;
  double resi_[24];
  double resic_[24];
  double err_[24];
  double errc_[24];
  double x_[24];
  double y_[24];
  double z_[24];
  double w_[24];
  double s_[24];
  double tdrift_[24];
  int wire_[24];
  int left_right_[24];  // -1: left, +1: right
};

#endif  // _JEventProcessor_TrackingPulls_straight_

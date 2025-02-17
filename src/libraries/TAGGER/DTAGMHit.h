// $Id$
//
// File: DTAGMHit.h
// Created: Sat Jul 5 07:49:15 EDT 2014
// Creator: jonesrt (on gluey.phys.uconn.edu)
//

#ifndef _DTAGMhit_
#define _DTAGMhit_

#include <JANA/JObject.h>

class DTAGMHit : public JObject{
   public:
      JOBJECT_PUBLIC(DTAGMHit);

      double E;
      double t;
      int row;
      int column;
      double integral;
      double pulse_peak;
      double time_tdc;
      double time_fadc;
      double npix_fadc;
      bool has_fADC,has_TDC;
      int bg = -1; //if MC, 0 for the photon that generated the event, nonzero otherwise //ignore if not MC

      void Summarize(JObjectSummary& summary) const override {
        summary.add(row, "row", "%d");
        summary.add(column, "column", "%d");
        summary.add((float)E, "E(GeV)", "%f");
        summary.add((float)t, "t(ns)", "%f");
        summary.add((float)time_tdc, "time_tdc(ns)", "%f");
        summary.add((float)time_fadc, "time_fadc(ns)", "%f");
        summary.add((float)integral, "integral", "%f");
        summary.add((float)pulse_peak, "pulse_peak", "%f");
        summary.add((float)npix_fadc, "npix_fadc", "%f");
        summary.add((int)has_fADC, "has_fADC", "%d");
        summary.add((int)has_TDC, "has_TDC", "%d");
        summary.add(bg, "bg", "%d");
      }
};

#endif // _DTAGMHit_

// $Id$
//
// File: DTAGHHit.h
// Created: Sat Jul 5 07:49:15 EDT 2014
// Creator: jonesrt (on gluey.phys.uconn.edu)
//

#ifndef _DTAGHhit_
#define _DTAGHhit_

#include <JANA/JObject.h>

class DTAGHHit : public JObject{
   public:
      JOBJECT_PUBLIC(DTAGHHit);

      double E;
      double t;
      int counter_id;
      double integral;
      double pulse_peak;
      double time_tdc;
      double time_fadc;
      double npe_fadc;
      bool has_fADC, has_TDC, is_double;
      int bg = -1; //if MC, 0 for the photon that generated the event, nonzero otherwise //ignore if not MC

      void Summarize(JObjectSummary& summary) const override {
      	 summary.add(counter_id, NAME_OF(counter_id), "%d");
	     summary.add(E, "E(GeV)", "%f");
	     summary.add(t, "t(ns)", "%f");
	     summary.add(time_tdc, "time_tdc(ns)", "%f");
	     summary.add(time_fadc, "time_fadc(ns)", "%f");
	     summary.add(integral, "integral", "%f");
	     summary.add(pulse_peak, "pulse_peak", "%f");
	     summary.add(npe_fadc, "npe_fadc", "%f");
	     summary.add((int)has_fADC, "has_fADC", "%d");
	     summary.add((int)has_TDC, "has_TDC", "%d");
	     summary.add((int)is_double, "is_double", "%d");
	     summary.add(bg, "bg", "%d");
      }

};

#endif // _DTAGHHit_

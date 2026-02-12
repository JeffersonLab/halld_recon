// $Id$
//
//    File: DCDCTrackHit.h
// Created: Mon Oct 16 10:20:07 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.7.0 powerpc)
//

#ifndef _DCDCTrackHit_
#define _DCDCTrackHit_

#include <JANA/JObject.h>

#include "DCDCWire.h"

enum CDCTrackD {
   dDOCAdOriginX=0,
   dDOCAdOriginY,
   dDOCAdOriginZ,
   dDOCAdDirX,
   dDOCAdDirY,
   dDOCAdDirZ,
   dDOCAdS0,
   dDOCAdS1,
   dDOCAdS2,
   dDOCAdS3,
   dDOCAdS4,
   dDdt0

};

class DCDCTrackHit:public JObject{
   public:
      JOBJECT_PUBLIC(DCDCTrackHit);

      const DCDCWire *wire;	// DCDCWire structure for this hit
      bool is_stereo; // true if this is stereo wire
      float tdrift;				// Drift time of hit in ns
      float dist;					// Measured DOCA in cm
      float dE; // Energy deposition in GeV, using integral
      float dE_amp; // same, but using amplitude

      void Summarize(JObjectSummary& summary) const override {
         summary.add(wire->ring, "ring", "%d");
         summary.add(wire->straw, "straw", "%d");
         summary.add(wire->origin.x(), "x(cm)", "%3.1f");
         summary.add(wire->origin.y(), "y(cm)", "%3.1f");
         summary.add(wire->stereo, "stereo(rad)", "%1.4f");
         summary.add(tdrift, "tdrift(ns)", "%3.1f");
         summary.add(dist, "dist(cm)", "%1.3f");
         summary.add(dE, "dE(GeV)", "%3.1g");
         summary.add(dE_amp, "dE_amp(GeV)", "%3.1g");
      }

};

#endif // _DCDCTrackHit_


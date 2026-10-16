// $Id$
//
// File: DEventHitStatistics.h
// Created: Wed Jul 8 17:49:15 EDT 2020
// Creator: jonesrt (on gluey.phys.uconn.edu)
//

#ifndef _DEventHitStatistics_
#define _DEventHitStatistics_

#include <JANA/JObject.h>

class DEventHitStatistics:public JObject{
   public:
      JOBJECT_PUBLIC(DEventHitStatistics);

      int start_counters;
      int cdc_straws;
      int fdc_pseudos;
      int bcal_cells;
      int fcal_blocks;
      int ecal_blocks;
      int ccal_blocks;
      int tof_paddles;
      int dirc_PMTs;

      void Summarize(JObjectSummary& summary) const override {
        summary.add(start_counters, "start counter hits", "%d");
        summary.add(cdc_straws, "cdc straw hits", "%d");
        summary.add(fdc_pseudos, "fdc pseudo-hits", "%d");
        summary.add(bcal_cells, "bcal cells hit", "%d");
        summary.add(fcal_blocks, "fcal blocks hit", "%d");
        summary.add(ecal_blocks, "ecal blocks hit", "%d");
        summary.add(ccal_blocks, "ccal blocks hit", "%d");
        summary.add(tof_paddles, "tof paddle hits", "%d");
        summary.add(dirc_PMTs, "dirc PMT hits", "%d");
      }
};

#endif // _DEventHitStatistics_

// $Id$
//
// File: DEventHitStatistics.h
// Created: Wed Jul 8 17:49:15 EDT 2020
// Creator: jonesrt (on gluey.phys.uconn.edu)
//

#ifndef _DEventHitStatistics_
#define _DEventHitStatistics_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DEventHitStatistics:public jana::JObject{
   public:
      JOBJECT_PUBLIC(DEventHitStatistics);

      int start_counters;
      int cdc_straws;
      int fdc_pseudos;
      int bcal_cells;
      int fcal_blocks;
      int ccal_blocks;
      int tof_paddles;
      int dirc_PMTs;

      void toStrings(vector<pair<string,string> > &items) const {
        AddString(items, "start counter hits", "%d", start_counters);
        AddString(items, "cdc straw hits", "%d", cdc_straws);
        AddString(items, "fdc pseudo-hits", "%d", fdc_pseudos);
        AddString(items, "bcal cells hit", "%d", bcal_cells);
        AddString(items, "fcal blocks hit", "%d", fcal_blocks);
        AddString(items, "ccal blocks hit", "%d", ccal_blocks);
        AddString(items, "tof paddle hits","%d", tof_paddles);
        AddString(items, "dirc PMT hits","%d", dirc_PMTs);
      }
};

#endif // _DEventHitStatistics_

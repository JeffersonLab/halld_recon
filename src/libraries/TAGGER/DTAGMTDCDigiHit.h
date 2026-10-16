// $Id$
//
//    File: DTAGMTDCDigiHit.h
// Created: Tue Aug  6 13:02:22 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DTAGMTDCDigiHit_
#define _DTAGMTDCDigiHit_

#include <JANA/JObject.h>

class DTAGMTDCDigiHit: public JObject {
   public:
      JOBJECT_PUBLIC(DTAGMTDCDigiHit);
      
      int row;         ///< row number 1-5
      int column;      ///< column number 1-102
      uint32_t time;

      void Summarize(JObjectSummary& summary) const override {
         summary.add(row, NAME_OF(row), "%d");
         summary.add(column, NAME_OF(column), "%d");
         summary.add(time, NAME_OF(time), "%d");
      }
};

#endif // _DTAGMTDCDigiHit_

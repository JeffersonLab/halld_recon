// $Id$
//
//    File: DTAGMHit_factory.h
// Created: Sat Aug  2 12:23:43 EDT 2014
// Creator: jonesrt (on Linux gluey.phys.uconn.edu)
//

#ifndef _DTAGMHit_factory_
#define _DTAGMHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTTabUtilities.h"

#include "DTAGMHit.h"
#include "DTAGMGeometry.h"

class DTAGMHit_factory: public JFactoryT<DTAGMHit> {
   public:
      DTAGMHit_factory() {};
      ~DTAGMHit_factory() {};

      // config. parameter
      double DELTA_T_CLUSTER_MAX;
      bool MERGE_HITS;

   private:
      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override;
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override;
      void Finish() override;

      void Reset_Data(void);
};

#endif // _DTAGMHit_factory_

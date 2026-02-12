// $Id$
//
//    File: DEventHitStatistics_factory.h
// Created: Wed Jul 8 17:49:15 EDT 2020
// Creator: jonesrt (on Linux gluey.phys.uconn.edu)
//

#ifndef _DEventHitStatistics_factory_
#define _DEventHitStatistics_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>

#include "DEventHitStatistics.h"

class DEventHitStatistics_factory: public JFactoryT<DEventHitStatistics> {
   public:
      DEventHitStatistics_factory() = default;
      ~DEventHitStatistics_factory() = default;

   private:
      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override;
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override;
      void Finish() override;

      void Reset_Data();
};

#endif // _DEventHitStatistics_factory_

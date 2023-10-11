// $Id$
//
//    File: DTAGHHit_factory.h
// Created: Sat Aug  2 12:23:43 EDT 2014
// Creator: jonesrt (on Linux gluey.phys.uconn.edu)
//

#ifndef _DTAGHHit_factory_
#define _DTAGHHit_factory_

#include <JANA/JFactoryT.h>
#include "DTAGHHit.h"

class DTAGHHit_factory: public JFactoryT<DTAGHHit> {
    public:
        DTAGHHit_factory() {};
        ~DTAGHHit_factory() {};

    private:
        // config. parameters
        bool MERGE_DOUBLES;
        double DELTA_T_DOUBLES_MAX;
        size_t DELTA_ID_DOUBLES_MAX;
        int ID_DOUBLES_MAX;
        bool USE_SIDEBAND_DOUBLES;

        double dBeamBunchPeriod;
        bool IsDoubleHit(double tdiff);

        void Init() override;
        void BeginRun(const std::shared_ptr<const JEvent>& event) override;
        void Process(const std::shared_ptr<const JEvent>& event) override;
        void EndRun() override;
        void Finish() override;
};

#endif // _DTAGHHit_factory_

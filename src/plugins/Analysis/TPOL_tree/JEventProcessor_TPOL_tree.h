// $Id$
//
//    File: JEventProcessor_TPOL_tree.h
// Created: Thu Feb  4 16:11:54 EST 2016
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.4.4.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TPOL_tree_
#define _JEventProcessor_TPOL_tree_

#include <JANA/JEventProcessor.h>

#include <DAQ/Df250WindowRawData.h>
#include "ANALYSIS/DTreeInterface.h"
#include <TPOL/DTPOLSectorDigiHit.h>
#include <TPOL/DTPOLRingDigiHit.h>

class JEventProcessor_TPOL_tree:public jana::JEventProcessor{
public:
    JEventProcessor_TPOL_tree();
    ~JEventProcessor_TPOL_tree();
    const char* className(void){return "JEventProcessor_TPOL_tree";}

    unsigned int count;
    double GetPhi(unsigned int sector);
    //bool DTPOLSectorHit_fadc_cmp(const DTPOLSectorDigiHit *a,const DTPOLSectorDigiHit *b);
    //bool DTPOLRingHit_fadc_cmp(const DTPOLRingDigiHit *a,const DTPOLRingDigiHit *b);
    

    static const UInt_t ntag_max = 1000000;

private:
    jerror_t init(void); ///< Called once at program start.
    jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber); ///< Called everytime a new run number is detected.
    jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber); ///< Called every event.
    jerror_t erun(void); ///< Called everytime run number changes, provided brun has been called.
    jerror_t fini(void); ///< Called after last event of last event source has been processed.

    DTreeInterface* dTreeInterface;
    //static thread_local 
    DTreeFillData dTreeFillData;
};

#endif // _JEventProcessor_TPOL_tree_

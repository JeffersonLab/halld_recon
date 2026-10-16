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
#include <TPOL/DTPOLHit.h>

class JEventProcessor_TPOL_tree:public JEventProcessor{
public:
    JEventProcessor_TPOL_tree();
    ~JEventProcessor_TPOL_tree();

    unsigned int count;
    double GetPhi(unsigned int sector);
    //bool DTPOLSectorHit_fadc_cmp(const DTPOLSectorDigiHit *a,const DTPOLSectorDigiHit *b);
    //bool DTPOLRingHit_fadc_cmp(const DTPOLRingDigiHit *a,const DTPOLRingDigiHit *b);
    

    static const UInt_t ntag_max = 1000000;

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    DTreeInterface* dTreeInterface;
    //static thread_local 
    DTreeFillData dTreeFillData;
};

#endif // _JEventProcessor_TPOL_tree_

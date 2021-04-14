// $Id$
//
//    File: DEventHitStatistics_factory.cc
// Created: Wed Jul 8 17:49:15 EDT 2020
// Creator: jonesrt (on Linux gluex.phys.uconn.edu)
//

#include <iostream>
#include <iomanip>

#include <HDDM/DEventHitStatistics.h>
#include <START_COUNTER/DSCHit.h>
#include <CDC/DCDCHit.h>
#include <FDC/DFDCPseudo.h>
#include <BCAL/DBCALHit.h>
#include <FCAL/DFCALHit.h>
#include <CCAL/DCCALHit.h>
#include <TOF/DTOFPaddleHit.h>
#include <DIRC/DDIRCPmtHit.h>

using namespace std;

#include "DEventHitStatistics_factory.h"

using namespace jana;

//------------------
// init
//------------------
jerror_t DEventHitStatistics_factory::init(void)
{
    // Setting this flag makes it so that JANA does not delete the objects in _data.
    // This factory will manage this memory.
    SetFactoryFlag(NOT_OBJECT_OWNER);

    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventHitStatistics_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventHitStatistics_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// Clear _data vector
	Reset_Data();
	DEventHitStatistics* stats = new DEventHitStatistics;
    {
       vector<const DSCHit*> hits;
	   loop->Get(hits);
       stats->start_counters = hits.size();
    }
    {
       vector<const DCDCHit*> hits;
	   loop->Get(hits);
       stats->cdc_straws = hits.size();
    }
    {
       vector<const DFDCPseudo*> hits;
	   loop->Get(hits);
       stats->fdc_pseudos = hits.size();
    }
    {
       vector<const DBCALHit*> hits;
	   loop->Get(hits);
       stats->bcal_cells = hits.size();
    }
    {
       vector<const DFCALHit*> hits;
	   loop->Get(hits);
       stats->fcal_blocks = hits.size();
    }
    {
       vector<const DCCALHit*> hits;
	   loop->Get(hits);
       stats->ccal_blocks = hits.size();
    }
    {
       vector<const DTOFPaddleHit*> hits;
	   loop->Get(hits);
       stats->tof_paddles = hits.size();
    }
    {
       vector<const DDIRCPmtHit*> hits;
	   loop->Get(hits);
       stats->dirc_PMTs = hits.size();
    }
	_data.push_back(const_cast<DEventHitStatistics*>(stats));

    return NOERROR;
}

//------------------
// Reset_Data()
//------------------
void DEventHitStatistics_factory::Reset_Data(void)
{
	// Clear _data vector
	_data.clear();
}

//------------------
// erun
//------------------
jerror_t DEventHitStatistics_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventHitStatistics_factory::fini(void)
{
    return NOERROR;
}

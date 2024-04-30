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
#include <ECAL/DECALHit.h>
#include <TOF/DTOFPaddleHit.h>
#include <DIRC/DDIRCPmtHit.h>

using namespace std;

#include "DEventHitStatistics_factory.h"



//------------------
// Init
//------------------
void DEventHitStatistics_factory::Init()
{
    // Setting this flag makes it so that JANA does not delete the objects in _data.
    // This factory will manage this memory.
    SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DEventHitStatistics_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventHitStatistics_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	// Clear _data vector
	Reset_Data();
	DEventHitStatistics* stats = new DEventHitStatistics;
    {
       vector<const DSCHit*> hits;
	   event->Get(hits);
       stats->start_counters = hits.size();
    }
    {
       vector<const DCDCHit*> hits;
	   event->Get(hits);
       stats->cdc_straws = hits.size();
    }
    {
       vector<const DFDCPseudo*> hits;
	   event->Get(hits);
       stats->fdc_pseudos = hits.size();
    }
    {
       vector<const DBCALHit*> hits;
	   event->Get(hits);
       stats->bcal_cells = hits.size();
    }
    {
       vector<const DFCALHit*> hits;
	   event->Get(hits);
       stats->fcal_blocks = hits.size();
    }
    {
       vector<const DECALHit*> hits;
	   event->Get(hits);
       stats->ecal_blocks = hits.size();
    }
    {
       vector<const DCCALHit*> hits;
	   event->Get(hits);
       stats->ccal_blocks = hits.size();
    }
    {
       vector<const DTOFPaddleHit*> hits;
	   event->Get(hits);
       stats->tof_paddles = hits.size();
    }
    {
       vector<const DDIRCPmtHit*> hits;
	   event->Get(hits);
       stats->dirc_PMTs = hits.size();
    }
	Insert(const_cast<DEventHitStatistics*>(stats));
}

//------------------
// Reset_Data()
//------------------
void DEventHitStatistics_factory::Reset_Data()
{
	// Clear _data vector
	mData.clear();
}

//------------------
// EndRun
//------------------
void DEventHitStatistics_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DEventHitStatistics_factory::Finish()
{
}

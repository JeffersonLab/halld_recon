// $Id$
//
//    File: DTAGMHit_factory.cc
// Created: Sat Aug  2 12:23:43 EDT 2014
// Creator: jonesrt (on Linux gluex.phys.uconn.edu)
//

// aebarnes moved original factory to DTAGMHit_factory_Calib.cc on August 11, 2016.

#include <iostream>
#include <iomanip>
#include <TAGGER/DTAGMHit.h>

#include <JANA/JEvent.h>
using namespace std;

// Sort by column
bool SortByCol(const DTAGMHit* a, const DTAGMHit* b)
{
        if (a->column == b->column) return a->t < b->t;
	return a->column < b->column;
}

#include "DTAGMHit_factory.h"



//------------------
// Init
//------------------
void DTAGMHit_factory::Init()
{
	auto app = GetApplication();

    // Set default configuration parameters
    DELTA_T_CLUSTER_MAX = 5;
    app->SetDefaultParameter("TAGMHit:DELTA_T_CLUSTER_MAX",DELTA_T_CLUSTER_MAX,
                                "Maximum time difference in ns between hits in adjacent"
                                " columns to be merged");

    MERGE_HITS = true;
    app->SetDefaultParameter("TAGMHit:MERGE_HITS",MERGE_HITS,
                                "Merge neighboring hits when true");

    // Setting this flag makes it so that JANA does not delete the objects in _data.
    // This factory will manage this memory.
    SetFactoryFlag(NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DTAGMHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DTAGMHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	// Clear _data vector
	Reset_Data();

	// Get (calibrated) TAGM hits
	vector<const DTAGMHit*> hits;
	event->Get(hits, "Calib");

	sort(hits.begin(), hits.end(), SortByCol);

	set<uint32_t> locHitIndexUsedSoFar;

	for(uint32_t i = 0; i < hits.size(); ++i)
	{
		DTAGMHit *hit_i = const_cast<DTAGMHit*>(hits[i]);

		if (hit_i->row > 0 || !MERGE_HITS)
		{
			Insert(const_cast<DTAGMHit*>(hit_i));
			continue;
		}
		if (!hit_i->has_fADC) continue;

		// check if column has been paired
		if (locHitIndexUsedSoFar.find(i) != locHitIndexUsedSoFar.end()) continue;

		for (uint32_t j = i+1; j < hits.size(); ++j)
		{
			const DTAGMHit *hit_j = hits[j];

			if (!hit_j->has_fADC) continue;
			if (hit_j->row > 0) continue;

			int colDiff = hit_i->column - hit_j->column;
			double deltaT = hit_i->t - hit_j->t;

			if (abs(colDiff) == 1 && fabs(deltaT) <= DELTA_T_CLUSTER_MAX)
			{
				hit_i->AddAssociatedObject(hit_j);
				locHitIndexUsedSoFar.insert(j);
				break;
			}
		}
		Insert(const_cast<DTAGMHit*>(hit_i));
	}
}

//------------------
// Reset_Data()
//------------------
void DTAGMHit_factory::Reset_Data()
{
	// Clear _data vector
	mData.clear();
}

//------------------
// EndRun
//------------------
void DTAGMHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DTAGMHit_factory::Finish()
{
}

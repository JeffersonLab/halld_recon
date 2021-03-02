// $Id$
//
//    File: DCGEMHit_factory.cc
// Created: Tue Aug  6 12:23:43 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "CGEM/DCGEMHit_factory.h"
#include "TTAB/DTTabUtilities.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DCGEMHit_factory::init(void)
{
    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DCGEMHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
    // Only print messages for one thread whenever run number change
    static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
    static set<int> runs_announced;
    pthread_mutex_lock(&print_mutex);
    bool print_messages = false;
    if(runs_announced.find(runnumber) == runs_announced.end()){
        print_messages = true;
        runs_announced.insert(runnumber);
    }
    pthread_mutex_unlock(&print_mutex);

    if(print_messages) jout << "In DCGEMHit_factory, loading zero constants..." << endl;

    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DCGEMHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    /// Generate DCGEMHit object for each DFCALDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.
    ///
    /// Note that this code does NOT get called for simulated
    /// data in HDDM format. The HDDM event source will copy
    /// the precalibrated values directly into the _data vector.
    //char str[256];

    const DTTabUtilities* locTTabUtilities = nullptr;
    loop->GetSingle(locTTabUtilities);

    vector<const DCGEMDigiHit*> digihits;
    loop->Get(digihits);
    for (unsigned int i=0; i < digihits.size(); i++) {

      const DCGEMDigiHit *digihit = digihits[i];
      // Build hit object
      DCGEMHit *hit = new DCGEMHit;
      hit->layer = digihit->layer;
      hit->dE    = digihit->dE; 
      hit->t     = digihit->t;
      hit->x     = digihit->x;
      hit->y     = digihit->y;
      hit->z     = digihit->z;
	        
      // do some basic quality checks before creating the objects
      //if( ( hit->dE > 0 ) ) {
      hit->AddAssociatedObject(digihit);
      _data.push_back(hit);
      //} else {
      //delete hit;
      //}
    }

    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DCGEMHit_factory::erun(void)
{
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DCGEMHit_factory::fini(void)
{
    return NOERROR;
}

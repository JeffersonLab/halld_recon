// $Id$
//
//    File: DTrackTimeBased_factory_StraightLine.cc
// Created: Wed Mar 13 10:00:17 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DTrackTimeBased_factory_StraightLine.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	// Code to generate factory data goes here. Add it like:
	//
	// DTrackTimeBased *myDTrackTimeBased = new DTrackTimeBased;
	// myDTrackTimeBased->x = x;
	// myDTrackTimeBased->y = y;
	// ...
	// _data.push_back(myDTrackTimeBased);
	//
	// Note that the objects you create here will be deleted later
	// by the system and the _data vector will be cleared automatically.

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::fini(void)
{
	return NOERROR;
}


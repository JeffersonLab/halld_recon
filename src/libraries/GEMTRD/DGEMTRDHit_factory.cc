// $Id$
//
//    File: DGEMTRDHit_factory.cc
// Created: Mon Feb  7 16:39:32 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DGEMTRDHit_factory.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DGEMTRDHit_factory::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DGEMTRDHit_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DGEMTRDHit_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	// Code to generate factory data goes here. Add it like:
	//
	// DGEMTRDHit *myDGEMTRDHit = new DGEMTRDHit;
	// myDGEMTRDHit->x = x;
	// myDGEMTRDHit->y = y;
	// ...
	// _data.push_back(myDGEMTRDHit);
	//
	// Note that the objects you create here will be deleted later
	// by the system and the _data vector will be cleared automatically.

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DGEMTRDHit_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DGEMTRDHit_factory::fini(void)
{
	return NOERROR;
}


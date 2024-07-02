// $Id$
//
//    File: DECALTruthShower_factory.cc
// Created: Tue Nov 30 15:02:26 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DECALTruthShower_factory.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DECALTruthShower_factory::init(void)
{
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DECALTruthShower_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DECALTruthShower_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	// Code to generate factory data goes here. Add it like:
	//
	// DECALTruthShower *myDECALTruthShower = new DECALTruthShower;
	// myDECALTruthShower->x = x;
	// myDECALTruthShower->y = y;
	// ...
	// _data.push_back(myDECALTruthShower);
	//
	// Note that the objects you create here will be deleted later
	// by the system and the _data vector will be cleared automatically.

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DECALTruthShower_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DECALTruthShower_factory::fini(void)
{
	return NOERROR;
}


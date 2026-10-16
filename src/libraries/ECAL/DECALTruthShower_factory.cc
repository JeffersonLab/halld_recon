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

//------------------
// Init
//------------------
void DECALTruthShower_factory::Init(void)
{
	return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void DECALTruthShower_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	return; //NOERROR;
}

//------------------
// Process
//------------------
void DECALTruthShower_factory::Process(const std::shared_ptr<const JEvent>& event)
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

	return; //NOERROR;
}

//------------------
// EndRun
//------------------
void DECALTruthShower_factory::EndRun(void)
{
	return; //NOERROR;
}

//------------------
// Finish
//------------------
void DECALTruthShower_factory::Finish(void)
{
	return; //NOERROR;
}


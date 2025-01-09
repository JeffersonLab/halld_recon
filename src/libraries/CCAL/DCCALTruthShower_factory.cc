// $Id$
//
//    File: DCCALTruthShower_factory.cc
// Created: Tue Nov 30 15:02:26 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DCCALTruthShower_factory.h"


//------------------
// Init
//------------------
void DCCALTruthShower_factory::Init()
{
}

//------------------
// BeginRun
//------------------
void DCCALTruthShower_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DCCALTruthShower_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	
	// Code to generate factory data goes here. Add it like:
	//
	// DCCALTruthShower *myDCCALTruthShower = new DCCALTruthShower;
	// myDCCALTruthShower->x = x;
	// myDCCALTruthShower->y = y;
	// ...
	// Insert(myDCCALTruthShower);
	//
	// Note that the objects you create here will be deleted later
	// by the system and the _data vector will be cleared automatically.

}

//------------------
// EndRun
//------------------
void DCCALTruthShower_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCCALTruthShower_factory::Finish()
{
}


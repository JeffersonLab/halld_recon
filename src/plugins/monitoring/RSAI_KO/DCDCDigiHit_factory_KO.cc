// $Id$
//
//    File: DCDCDigiHit_factory_KO.cc
// Created: Thu Sep 26 22:51:16 EDT 2019
// Creator: davidl (on Linux gluon46.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <random>
#include <mutex>
using namespace std;

#include "DCDCDigiHit_factory_KO.h"
using namespace jana;


static thread_local std::default_random_engine generator;
static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);

static std::once_flag cdc_report_flag;

#include "CDC_pins.h"

//------------------
// SetBoardEfficiency
//
// Set all pins for the speficied board to the specified efficency.
//------------------
int  DCDCDigiHit_factory_KO::SetBoardEfficiency(string boardname, double eff)
{
	if( CDC_pins.count(boardname) == 0 ){
		jerr << " Unknown CDC board name \"" << boardname << "\"!" << std::endl;
		jerr << "   known CDC boards: ";
		for( auto p : CDC_pins ) jerr << p.first << " ";
		jerr << endl;
		_exit(-1);
	}

	int Npins = 0;
	for( auto k : CDC_pins[boardname] ){
		CDC_pin_eff[k] = eff;
		Npins++;
	}

	return Npins;
}

//------------------
// init
//------------------
jerror_t DCDCDigiHit_factory_KO::init(void)
{
	// Tell JANA we don't own any of the objects in _data
	SetFactoryFlag( NOT_OBJECT_OWNER );

	// Tell JANA not to look in the source when trying to find
	// DCDCDigiHit objects for *this* factory.
	use_factory = 1;

	// Get all parameters starting with "KO:CDC_". The keys will have that
	// portion of the string removed leaving only the board name.
	map<string,string> parms;
	gPARMS->GetParameters(parms, "KO:CDC_");
	map<string, int> Npins;
	for( auto p : parms ) {
		int N = SetBoardEfficiency( p.first, stod( p.second ) );
		Npins[p.first] = N;
		string one("1.00000");
		gPARMS->SetDefaultParameter( string("KO:CDC_")+p.first, one ); // Set default parameter to avoid flashing red warning
	}

	// Print list of boards that have an efficiency defined
	std::call_once( cdc_report_flag, [=](){
		cout << "CDC Digihit KO:" << endl;
		cout << "---------------" << endl;
		if( Npins.empty() ) cout << "  <NONE> " << endl;
		for( auto p : Npins ){
			cout << "  " << p.first << " eff=" << stod(parms.at(p.first)) << " Npins=" << p.second << endl;
		}
	});

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DCDCDigiHit_factory_KO::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DCDCDigiHit_factory_KO::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	vector<const DCDCDigiHit*> cdcdigihits;
	loop->Get(cdcdigihits, "", false); // get original hits

	for( auto hit : cdcdigihits ){
		auto key = std::make_pair( hit->ring, hit->straw );
		if( CDC_pin_eff.count(key) ){
			if( distribution(generator) <= CDC_pin_eff[key]) _data.push_back( (DCDCDigiHit*)hit );
		}else{
			_data.push_back( (DCDCDigiHit*)hit );
		}
	}

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DCDCDigiHit_factory_KO::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DCDCDigiHit_factory_KO::fini(void)
{
	return NOERROR;
}


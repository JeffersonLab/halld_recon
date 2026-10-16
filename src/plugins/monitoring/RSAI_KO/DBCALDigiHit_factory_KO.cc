// $Id$
//
//    File: DBCALDigiHit_factory_KO.cc
// Created: Mon Sep 30 22:45:50 EDT 2019
// Creator: davidl (on Linux gluon113.jlab.org 3.10.0-957.21.3.el7.x86_64 x86_64)
//

#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <string>
#include <thread>
#include <random>
#include <mutex>
using namespace std;

#include "DBCALDigiHit_factory_KO.h"

//------------------
// Init
//------------------
void DBCALDigiHit_factory_KO::Init()
{
	// Tell JANA we don't own any of the objects in _data
	SetFactoryFlag( NOT_OBJECT_OWNER );

	FillEfficiencyMap("KO:BCAL_ADC_", BCAL_adc_cell_eff);

	// Print list of boards that have an efficiency defined
	static std::once_flag flag;
	std::call_once( flag, [=](){
		cout << "BCAL Digihit KO:" << endl;
		cout << "---------------" << endl;
		if( BCAL_adc_cell_eff.empty() ) cout << "  <NONE> " << endl;
		for( auto p : BCAL_adc_cell_eff ){
			auto &key = p.first;
			auto &eff = p.second;
			cout << "  module,sector,layer,end=("
				<< std::get<0>(key) << ","
				<< std::get<1>(key) << ","
				<< std::get<2>(key) << ","
				<< std::get<3>(key) << ")"
				<< " eff=" << eff << endl;
		}
		cout << endl;
	});
}

//------------------
// FillEfficiencyMap
//
// Fill the given efficiency map container with efficiencies extracted
// from config. parameters that start whose names start with the given
// prefix.
//------------------
void DBCALDigiHit_factory_KO::FillEfficiencyMap(string config_prefix, std::map< std::tuple<int,int,int,int>, double > &eff_map) {

	auto app = GetApplication();
	// Get all parameters starting with "KO:BCAL_ADC_". The keys will have that
	// portion of the string removed leaving only readout channel specific part
	map<string,string> parms;
	app->GetJParameterManager()->FilterParameters(parms, config_prefix);
	for( auto p : parms ) {

		// Extract specified module, layer, sector, end
		int module = 0;
		int layer  = 0;
		int sector = 0;
		int end    = 0;
		sscanf(p.first.c_str(), "M%dL%dS%dE%d", &module, &layer, &sector, &end);
		CheckRange(module, layer, sector, end);

		// To make things easier below we will set limits for each index.
		// The efficiency will be set for each index in the limits where
		// we'll use module,layer,sector,end as the starting value and
		// the "_last" as the last value.
		int module_last = module==0 ? 48:module;
		int layer_last  =  layer==0 ?  4:layer;
		int sector_last = sector==0 ?  4:sector;
		int end_last    =    end==2 ?  1:end;
		if( module==0 ) module = 1;
		if(  layer==0 ) layer  = 1;
		if( sector==0 ) sector = 1;
		if(    end==2 ) end    = 0;

		// Efficiency
		double eff = stod( p.second );

		// Loop over all indicies
		for( int im=module; im<=module_last; im++){
			for( int il=layer; il<=layer_last; il++){
				for( int is=sector; is<=sector_last; is++){
					for( int ie=end; ie<=end_last; ie++){
						auto key = std::make_tuple(im, il, is, ie);
						eff_map[key] = eff;
					}
				}
			}
		}

		string one("1.00000");
		app->SetDefaultParameter( config_prefix + p.first, one ); // Set default parameter to avoid flashing red warning
	}

}

//------------------
// CheckRange
//
// Check that each value is within the valid range (including 0 indicating "all")
// If any are not, then an error message is printed and _exit() is called.
//------------------
void DBCALDigiHit_factory_KO::CheckRange(int module, int layer, int sector, int end) {

	bool bad_config = false;
	if (module < 0 || module > 48) {
		cerr << " Bad module number: " << module << " (should be 0-48)" << endl;
		bad_config = true;
	}
	if (layer < 0 || layer > 4) {
		cerr << " Bad layer number: " << layer << " (should be 0-4)" << endl;
		bad_config = true;
	}
	if (sector < 0 || sector > 4) {
		cerr << " Bad sector number: " << sector << " (should be 0-4)" << endl;
		bad_config = true;
	}

	if (! bad_config) return;

	// Print help message on format of config parameters
	static std::once_flag flag;
	std::call_once(flag, [=]() {
		cout << endl;
		cout << "  Valid format for BCAL KO config. parameter is: KO:BCAL_ADC_MmLlSsEe" << endl;
		cout << "where the lower-case m,l,s,e are numbers representing the module," << endl;
		cout << "layer, sector, and end respectively. The values for any of module," << endl;
		cout << "layer, and sector may specify a single channel or 0 to specify all" << endl;
		cout << "channels for that index. For the e value, 0=upstream, 1=downstream" << endl;
		cout << "and 2=both. The value of the config. parameter itself should be a " << endl;
		cout << "value between 0 and 1 indicating the efficiency for the specified" << endl;
		cout << "channel(s)." << endl;
		cout << endl;
		cout << "example 1:  -PKO:BCAL_ADC_M10L2S2E0=0.30" << endl;
		cout << "    throw away all but 30% of the hits from module 10, layer 2," << endl;
		cout <<"     sector 2, upstream end." << endl;
		cout << endl;
		cout << "example 2:  -PKO:BCAL_ADC_M10L0S4E2=0.0" << endl;
		cout << "    throw away all hits from both ends of all layers in module 10, sector2" << endl;
		cout << endl;
		_exit(-1);
	});
}

//------------------
// BeginRun
//------------------
void DBCALDigiHit_factory_KO::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DBCALDigiHit_factory_KO::Process(const std::shared_ptr<const JEvent>& event)
{
	vector<const DBCALDigiHit*> bcaldigihits;
	event->Get(bcaldigihits, ""); // get original hits

	for( auto hit : bcaldigihits ){
		auto key = std::make_tuple( hit->module, hit->layer, hit->sector, hit->end );
		if( BCAL_adc_cell_eff.count(key) ){
			if( distribution(generator) <= BCAL_adc_cell_eff[key]) {
				Insert(const_cast<DBCALDigiHit*>(hit) );
			}
		} else {
			Insert(const_cast<DBCALDigiHit*>(hit));
		}
	}
}

//------------------
// EndRun
//------------------
void DBCALDigiHit_factory_KO::EndRun()
{
}

//------------------
// Finish
//------------------
void DBCALDigiHit_factory_KO::Finish()
{
}


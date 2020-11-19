// $Id$
//
//    File: DNeutralShower_factory_HadronPreSelect.h
//

#ifndef _DNeutralShower_factory_HadronPreSelect_
#define _DNeutralShower_factory_HadronPreSelect_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <PID/DNeutralShower.h>
#include <BCAL/DBCALShower.h>
#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALGeometry.h>

using namespace std;
using namespace jana;

class DNeutralShower_factory_HadronPreSelect : public jana::JFactory<DNeutralShower>
{
	public:
		DNeutralShower_factory_HadronPreSelect(){};
		~DNeutralShower_factory_HadronPreSelect(){};
		const char* Tag(void){return "HadronPreSelect";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		const DFCALGeometry* dFCALGeometry = nullptr;


		//Command-line values will override these
        double dMinFCALE; //HadronPreSelect:MIN_FCAL_E 
        double dMinBCALE; //HadronPreSelect:MIN_BCAL_E 
		double dMinCCALE; //HadronPreSelect:MIN_CCAL_E 
		double dMinBCALNcell; //HadronPreSelect:MIN_BCAL_NCELL
        double dMaxFCALR; //HadronPreSelect:MIN_FCAL_R
        double dMaxBCALZ; //HadronPreSelect:MIN_BCAL_Z
        double dFCALInnerRingCut; //HadronPreSelect:FCAL_INNER_CUT
        
        vector< int > dFCALInnerChannels;
};

#endif // _DNeutralShower_factory_HadronPreSelect_


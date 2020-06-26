// $Id$
//
//    File: DNeutralShower_factory_PreSelect.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralShower_factory_PreSelect_
#define _DNeutralShower_factory_PreSelect_

#include <iostream>
#include <iomanip>

#include <JANA/JFactory.h>
#include <PID/DNeutralShower.h>
#include <BCAL/DBCALShower.h>
#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALGeometry.h>

using namespace std;
using namespace jana;

class DNeutralShower_factory_PreSelect : public jana::JFactory<DNeutralShower>
{
	public:
		DNeutralShower_factory_PreSelect(){};
		~DNeutralShower_factory_PreSelect(){};
		const char* Tag(void){return "PreSelect";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		const DFCALGeometry* dFCALGeometry = nullptr;


		//Command-line values will override these
        double dMinFCALE; //PRESELECT:MIN_FCAL_E 
        double dMinBCALE; //PRESELECT:MIN_BCAL_E 
        double dMinCCALE; //PRESELECT:MIN_CCAL_E 
        double dMinBCALNcell; //PRESELECT:MIN_BCAL_NCELL
        double dMaxFCALR; //PRESELECT:MIN_FCAL_R
        double dMaxBCALZ; //PRESELECT:MIN_BCAL_Z
        double dFCALInnerRingCut; //PRESELECT:FCAL_INNER_CUT
        double dMinBCALShowerQuality; //PRESELECT:MIN_BCAL_SHOWER_QUALITY 
        double dMinFCALShowerQuality; //PRESELECT:MIN_FCAL_SHOWER_QUALITY 
       
        vector< int > dFCALInnerChannels;
};

#endif // _DNeutralShower_factory_PreSelect_


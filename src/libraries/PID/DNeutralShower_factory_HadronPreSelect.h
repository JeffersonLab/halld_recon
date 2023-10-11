// $Id$
//
//    File: DNeutralShower_factory_HadronPreSelect.h
//

#ifndef _DNeutralShower_factory_HadronPreSelect_
#define _DNeutralShower_factory_HadronPreSelect_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <PID/DNeutralShower.h>
#include <BCAL/DBCALShower.h>
#include <CCAL/DCCALShower.h>
#include <FCAL/DFCALGeometry.h>

using namespace std;

class DNeutralShower_factory_HadronPreSelect : public JFactoryT<DNeutralShower>
{
	public:
		DNeutralShower_factory_HadronPreSelect(){
			SetTag("HadronPreSelect");
		};
		~DNeutralShower_factory_HadronPreSelect(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

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


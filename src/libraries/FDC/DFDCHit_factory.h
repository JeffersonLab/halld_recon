// $Id$
//
//    File: DFDCHit_factory.h
// Created: Wed Aug  7 11:55:02 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DFDCHit_factory_
#define _DFDCHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTTabUtilities.h"

#include "DFDCHit.h"
#include "DFDCWireDigiHit.h"
#include "DFDCCathodeDigiHit.h"

// store constants indexed by gPlane/element number
typedef  vector< vector<double> >  fdc_digi_constants_t;

class DFDCHit_factory:public JFactoryT<DFDCHit>{
	public:
		DFDCHit_factory(){};
		~DFDCHit_factory(){};

		// overall scale factors
		double a_scale;
		double t_scale;
		double t_base;
		double fadc_t_base;

		// calibration constant tables
		fdc_digi_constants_t a_gains;
		fdc_digi_constants_t a_pedestals;
		fdc_digi_constants_t timing_offsets;

		const double GetConstant(const fdc_digi_constants_t &the_table,
					 const int in_gPlane, const int in_element) const;
		const double GetConstant(const fdc_digi_constants_t &the_table,
					 const DFDCCathodeDigiHit *the_digihit) const;
		const double GetConstant(const fdc_digi_constants_t &the_table,
					 const DFDCWireDigiHit *the_digihit) const;
		const double GetConstant(const fdc_digi_constants_t &the_table,
					 const DFDCHit *the_hit) const;
		//const double GetConstant(const fdc_digi_constants_t &the_table,
		//			 const DTranslationTable *ttab,
		//			 const int in_rocid, const int in_slot, const int in_channel) const;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
	
		bool USE_FDC;
		void LoadPackageCalibTables(const std::shared_ptr<const JEvent>& event, string ccdb_prefix);
};

#endif // _DFDCHit_factory_


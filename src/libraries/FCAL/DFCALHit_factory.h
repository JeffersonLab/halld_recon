// $Id$
//
//    File: DFCALHit_factory.h
// Created: Tue Aug  6 12:23:43 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DFCALHit_factory_
#define _DFCALHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"
#include "DFCALDigiHit.h"
#include "DFCALHit.h"
#include "DFCALGeometry.h"

// store constants so that they can be accessed by row/column number
typedef  vector< vector<double> >  fcal_digi_constants_t;

class DFCALHit_factory:public JFactoryT<DFCALHit>{
	public:
		DFCALHit_factory() = default;
		~DFCALHit_factory() override = default;

		static const int FCAL_MAX_CHANNELS = 2800;  // number of active channels expected

		enum fcal_quality_state {
		    GOOD,
		    BAD,
		    NOISY
		};

		// overall scale factors
		double a_scale;
		double t_scale;
                double t_base;

		// calibration constants stored in row, column format
		fcal_digi_constants_t gains;
		fcal_digi_constants_t pedestals;
		fcal_digi_constants_t time_offsets;
		fcal_digi_constants_t block_qualities;
		fcal_digi_constants_t ADC_Offsets;

		const double GetConstant(const fcal_digi_constants_t &the_table,
					 const int in_row, const int in_column) const;
		const double GetConstant(const fcal_digi_constants_t &the_table,
					 const DFCALDigiHit *the_digihit) const;
		const double GetConstant(const fcal_digi_constants_t &the_table,
					 const DFCALHit *the_hit) const;
		//const double GetConstant(const fcal_digi_constants_t &the_table,
		//			 const DTranslationTable *ttab,
		//			 const int in_rocid, const int in_slot, const int in_channel) const;
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		void FillCalibTable( fcal_digi_constants_t &table, 
				     const vector<double> &raw_table, 
				     const DFCALGeometry &fcalGeom);

        bool CHECK_FADC_ERRORS;
};

#endif // _DFCALHit_factory_


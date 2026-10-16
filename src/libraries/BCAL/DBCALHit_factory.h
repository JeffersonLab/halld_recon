// $Id$
//
//    File: DBCALHit_factory.h
// Created: Tue Aug  6 09:26:13 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DBCALHit_factory_
#define _DBCALHit_factory_

#include <vector>
#include <map>
#include <utility>
using namespace std;

#include <JANA/JFactoryT.h>
#include "TTAB/DTranslationTable.h"
#include "DBCALHit.h"

// store up/down-stream values for each detector cell
typedef pair<double,double> cell_calib_t;  
typedef vector<cell_calib_t>  bcal_digi_constants_t;

class DBCALHit_factory:public JFactoryT<DBCALHit>{
	public:
		DBCALHit_factory(){};
		~DBCALHit_factory(){};

		bool PRINTCALIBRATION;
		int VERBOSE;

		// shortcut geometry factors
		// these should really be taken from
		// DBCALGeometry/DGeometry objects
		static const int BCAL_NUM_MODULES  = 48;
		static const int BCAL_NUM_LAYERS   =  4;
		static const int BCAL_NUM_ENDS     =  2;
		static const int BCAL_NUM_SECTORS  =  4;
		static const int BCAL_MAX_CHANNELS =  1536;

		// overall scale factors
		double a_scale;
		double t_scale;
		double t_base;
		
		// constants tables
		bcal_digi_constants_t gains;
		bcal_digi_constants_t pedestals;
		bcal_digi_constants_t ADC_timing_offsets;
        bcal_digi_constants_t channel_global_offset;
        bcal_digi_constants_t tdiff_u_d;
        bcal_digi_constants_t bad_channels;
		
		const int GetCalibIndex( int module, int layer, int sector ) const {
			return BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);
		}

		const double GetConstant( const bcal_digi_constants_t &the_table,
					  const int in_module, const int in_layer,
					  const int in_sector, const int in_end) const;
		const double GetConstant( const bcal_digi_constants_t &the_table,
					  const DBCALDigiHit *the_digihit) const;
		const double GetConstant( const bcal_digi_constants_t &the_table,
					  const DBCALHit *the_hit) const;
		//const double GetConstant( const bcal_digi_constants_t &the_table,
		//			  const DTranslationTable *ttab,
		//			  const int in_rocid, const int in_slot, const int in_channel) const;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		void FillCalibTable( bcal_digi_constants_t &table, 
				     const vector<double> &raw_table);
        void FillCalibTableShort( bcal_digi_constants_t &table,
                    const vector<double> &raw_table);

        bool CHECK_FADC_ERRORS, CORRECT_FADC_SATURATION,CORRECT_SIPM_SATURATION;
	double fADC_MinIntegral_Saturation[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];
        double fADC_Saturation_Linear[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];
	double fADC_Saturation_Quadratic[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];
        double integral_to_peak[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];     // ration of pulse integral to peak value (integral counts)
	double sipm_npixels[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];         // number of pixels per sensor(s) 
        double pixel_per_count[BCAL_NUM_ENDS][BCAL_NUM_LAYERS];   // conversion between counts and pixels
};

#endif // _DBCALHit_factory_


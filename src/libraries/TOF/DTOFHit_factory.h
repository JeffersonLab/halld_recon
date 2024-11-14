// $Id$
//
/*! **File**: DTOFHit_factory.h
 *+ Created: Wed Aug  7 09:30:17 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Create TOF Hits based on DTOFDigiHit and DTOFTDCDigihit objects by finding
 * matches between ADC and TDC hits. A match is defined by a timing window and energy threshold.
 * If a match is found the TDC timing data is corrected for detector walk using the ADC 
 * amplitude. Both ADC and TDC times are converted to ns and the ADC energy deposition is 
 * converted to GeV. (No correction for attenuation yet.)
 * Calibration parameters are loaded from CCDB in the method ::brun()
*/

/// \addtogroup TOFDetector

/*! \file DTOFHit_factory.h define TOF Hit factory class and 
 * member functions definitions to faciliate the creation of TOF hits based on ADC
 * and TDC raw data.

 */

#ifndef _DTOFHit_factory_
#define _DTOFHit_factory_

#include <vector>
#include <map>
#include <utility>
using namespace std;

#include <JANA/JFactory.h>
#include "DTOFDigiHit.h"
#include "DTOFTDCDigiHit.h"
#include "DTOFHit.h"
#include "DTOFGeometry.h"
#include "TTAB/DTranslationTable.h"
#include "TTAB/DTTabUtilities.h"
#include <DAQ/Df250PulseData.h>
using namespace jana;


// store constants so that they can be accessed by plane/bar number
// each entry holds a pair of value for the two different bar ends
// [whatever the conventional choice is]
typedef  vector< vector< pair<double,double> > >  tof_digi_constants_t;

/*! \class generation of
 * TOF Hits based on ADC and TDC information contained in DTOFDigiHit and DTOFTDCDigiHit.
 * Matches between ADC and TDC hits are based on timing.
 */
class DTOFHit_factory:public jana::JFactory<DTOFHit>{
 public:
  DTOFHit_factory(){};
  ~DTOFHit_factory(){};
  
  int TOF_MAX_CHANNELS; ///< TOF channels (initizlied by geometry)
  int TOF_NUM_PLANES;   ///< TOF number of planes (initialized by geometry)
  int TOF_NUM_BARS;     ///< TOF number of paddles (initialized by geometry)
  
  // overall scale factors
  double a_scale;  ///< converstion factor ADC time to ns
  double t_scale;  ///< conversion factor TDC time to ns
  double t_base,t_base_tdc;  ///< base time offsets ADC and TDC times (from CCDB)
  double tdc_adc_time_offset;  ///< ADC/TDC relative timing offset (from CCDB) 

  // Timing Cut Values
  double TimeCenterCut;  ///< timing cut values to select in time hits
  double TimeWidthCut;   ///< timing cut values to select in time hits
  double hi_time_cut=100.;    ///< timing cut values to select in time hits
  double lo_time_cut=-100.;    ///< timing cut values to select in time hits

  // ADC to Energy conversion for individual PMT channels
  vector<double> adc2E;   ///< ADC to energy converstion values (loaded from CCDB) 

  // PARAMETERS:
  double DELTA_T_ADC_TDC_MAX;  ///< timing window to look for a match between ADC and TDC hits
  int USE_NEW_4WALKCORR;       ///< flag to use this walk correction approach
  int USE_AMP_4WALKCORR;       ///< flag to use this walk correction approach
  int USE_NEWAMP_4WALKCORR;    ///< flag to use this walk correction approach
  int USE_NEW_WALK_NEW;        ///< flag to use this walk correction approach

  tof_digi_constants_t adc_pedestals;
  tof_digi_constants_t adc_gains;
  tof_digi_constants_t adc_time_offsets;
  tof_digi_constants_t tdc_time_offsets;
  tof_digi_constants_t adc_bad_channels;
  tof_digi_constants_t tdc_bad_channels;
  
  vector<vector<double> >timewalk_parameters;       ///< walk parameters for walk correction v0
  vector<vector<double> >timewalk_parameters_AMP;   ///< walk parameters for walk correction v1
  vector<vector<double> >timewalk_parameters_NEW;   ///< walk parameters for walk correction v2
  vector<vector<double> >timewalk_parameters_NEWAMP;///< walk parameters for walk correction v3
  vector<vector<double> >timewalk_parameters_5PAR;  ///< walk parameters for walk correction v4

  /*! \fn find match between hit in TDC and ADC based on time
   */
  DTOFHit* FindMatch(int plane, int bar, int end, double T);
  
  const double GetConstant( const tof_digi_constants_t &the_table,
			    const int in_plane, const int in_bar, 
			    const int in_end ) const;
  const double GetConstant( const tof_digi_constants_t &the_table,
			    const DTOFDigiHit *the_digihit) const;
  const double GetConstant( const tof_digi_constants_t &the_table,
			    const DTOFHit *the_hit) const;
  const double GetConstant( const tof_digi_constants_t &the_table,
			    const DTOFTDCDigiHit *the_digihit) const;
  //const double GetConstant( const tof_digi_constants_t &the_table,
  //			  const DTranslationTable *ttab,
  //			  const int in_rocid, const int in_slot, const int in_channel) const;
  
  
 private:
  jerror_t init(void); ///< called at start up
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber); ///< called when run number changes
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber); ///< called for each event
  jerror_t erun(void); ///< called at the end of a run
  jerror_t fini(void); ///< called at the end of processing events
  
  void FillCalibTable(tof_digi_constants_t &table, vector<double> &raw_table,
		      const DTOFGeometry &tofGeom);///< load calibration table with values from CCDB

  double CalcWalkCorrIntegral(DTOFHit* hit); ///< apply walk correction v0
  double CalcWalkCorrAmplitude(DTOFHit* hit); ///< apply walk correction v1
  double CalcWalkCorrNEW(DTOFHit* hit); ///< apply walk correction v2
  double CalcWalkCorrNEWAMP(DTOFHit* hit); ///< apply walk correction v3
  double CalcWalkCorrNEW5PAR(DTOFHit* hit); ///< apply walk correction v4

  bool CHECK_FADC_ERRORS;
};

#endif // _DTOFHit_factory_


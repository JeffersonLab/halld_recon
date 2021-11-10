//
/*! **File**: DTPOLHit_factory.h
 *+ Created: unknown
 *+ Creator: unknown
 *+ Purpose: Class definition of the DTPOLHit_factory. This include files defines the class
 * and its members including the JANA prototype functions ::init(), ::brun(), ::evnt(), ::fini(), ::end()
 * ::brun() is called if the run number changes and the CCDB calibration parameters will be updated
 * for every event ::evnt() is called the the TPOL Hits will be created based on the raw data.
*/

/// \addtogroup TPOLDetector


/*! \file DTPOLHit_factory.h
 * TPOLHit factory class definition
 */


#ifndef _DTPOLHit_factory_
#define _DTPOLHit_factory_

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"
#include "DTPOLHit.h"


class DTPOLHit_factory:public jana::JFactory<DTPOLHit>{
 public:
  DTPOLHit_factory(){};
  ~DTPOLHit_factory(){};
  
  // Geometric information
  static const int NSECTORS   = 32; ///< hardcoded! this should be read from CCDB
  static const int NRINGS     = 24; ///< hardcoded! this whould be read from rCCDB
  
  static const double SECTOR_DIVISION;
  static const double INNER_RADIUS;      // From "ACTIVE INNER DIAMETER" in catalog
  static const double OUTER_RADIUS ;     // From "ACTIVE OUTER DIAMETER" in catalog
  static const double RING_DIVISION ;
  // (OUTER_RADIUS - INNER_RADIUS) / DTPOLRingDigiHit::NRINGS;
  // 1mm, agrees with "JUNCTION ELEMENT SEPARATION" in catalog
  
  
  // overall scale factors
  double a_scale; ///< time converstion from f250ADC counts to ns 
  double t_scale; ///< time converstion from f250ADC time counts to ns
  double t_base;  ///< global base time offset
  
  // calibration constants stored by channel
  vector<double>  a_gains; ///< ADC gain calibartion from CCDB
  vector<double>  a_pedestals; ///< ADC pedestals form CCDB
  vector<double>  adc_time_offsets; ///< ADC time offsets per channgel from CCDB
  
  double HIT_TIME_WINDOW; ///< ?
  double ADC_THRESHOLD;   ///< ?
  
  double GetPhi(int sector);
  double GetPulseTime(const vector<uint16_t> waveform,double w_min,double w_max,double minpeakheight);
  DTPOLHit* FindMatch(int sector, double T);
  const double GetConstant(const vector<double>  &the_table,const int in_sector) const;
  const double GetConstant(const vector<double>  &the_table,const DTPOLHit *the_hit) const;

 private:
  jerror_t init(void); // Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber); // Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber); // Called every event.
  jerror_t erun(void); // Called everytime run number changes, provided brun has been called.
  jerror_t fini(void); // Called after last event of last event source has been processed.
  
};

#endif // _DTPOLHit_factory_
